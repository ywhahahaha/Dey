/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2008-7-12      Bernard      the first version
 * 2010-06-09     Bernard      fix the end stub of heap
 *                             fix memory check in realloc function
 * 2010-07-13     Bernard      fix ALIGN issue found by kuronca
 * 2010-10-14     Bernard      fix realloc issue when realloc a NULL pointer.
 * 2017-07-14     armink       fix realloc issue when new size is 0
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *         Simon Goldschmidt
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "sys_mem.h"
#include "cmsis_os2.h"

/* #define MEM_DEBUG */
#define MEM_STATS
//#define USING_HOOK

#define ALIGN_SIZE                   4
#define ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

#ifdef USING_HOOK
#define OBJECT_HOOK_CALL(func, argv) \
    do { if ((func) != NULL) func argv; } while (0)
#else
#define OBJECT_HOOK_CALL(func, argv)
#endif

void (*malloc_hook)(void *ptr, size_t size);
void (*free_hook)(void *ptr);



/**
 * @addtogroup Hook
 */

/**@{*/

/**
 * This function will set a hook function, which will be invoked when a memory
 * block is allocated from heap memory.
 *
 * @param hook the hook function
 */
void malloc_sethook(void (*hook)(void *ptr, size_t size))
{
    malloc_hook = hook;
}

/**
 * This function will set a hook function, which will be invoked when a memory
 * block is released to heap memory.
 *
 * @param hook the hook function
 */
void free_sethook(void (*hook)(void *ptr))
{
    free_hook = hook;
}

/**@}*/


#define HEAP_MAGIC 0x1ea0
struct heap_mem {
    /* magic and used flag */
    uint16_t magic;
    uint16_t used;

    size_t next, prev;

#ifdef USING_MEMTRACE
    uint8_t thread[4];   /* thread name */
#endif
};

/** pointer to the heap: for alignment, heap_ptr is now a pointer instead of an array */
static uint8_t *heap_ptr;

/** the last entry, always unused! */
static struct heap_mem *heap_end;

#define MIN_SIZE 12
#define MIN_SIZE_ALIGNED     ALIGN(MIN_SIZE, ALIGN_SIZE)
#define SIZEOF_STRUCT_MEM    ALIGN(sizeof(struct heap_mem), ALIGN_SIZE)

static struct heap_mem *lfree;   /* pointer to the lowest free block */

osSemaphoreId_t heap_sem = NULL;  


static size_t mem_size_aligned;

static uint8_t heap[SYS_HEAP_SIZE];

#ifdef MEM_STATS
static size_t used_mem, max_mem;
#endif

void sys_free(void *rmem);

#ifdef USING_MEMTRACE
inline void mem_setname(struct heap_mem *mem, const char *name)
{
    int index;

    for(index = 0; index < sizeof(mem->thread); index ++) {
        if(name[index] == '\0') {
            break;
        }

        mem->thread[index] = name[index];
    }

    for(; index < sizeof(mem->thread); index ++) {
        mem->thread[index] = ' ';
    }
}
#endif

static void plug_holes(struct heap_mem *mem)
{
    struct heap_mem *nmem;
    struct heap_mem *pmem;
    ////ASSERT((uint8_t *)mem >= heap_ptr);
    ////ASSERT((uint8_t *)mem < (uint8_t *)heap_end);
    ////ASSERT(mem->used == 0);
    /* plug hole forward */
    nmem = (struct heap_mem *)&heap_ptr[mem->next];

    if(mem != nmem &&
       nmem->used == 0 &&
       (uint8_t *)nmem != (uint8_t *)heap_end) {
        /* if mem->next is unused and not end of heap_ptr,
         * combine mem and mem->next
         */
        if(lfree == nmem) {
            lfree = mem;
        }

        mem->next = nmem->next;
        ((struct heap_mem *)&heap_ptr[nmem->next])->prev = (uint8_t *)mem - heap_ptr;
    }

    /* plug hole backward */
    pmem = (struct heap_mem *)&heap_ptr[mem->prev];

    if(pmem != mem && pmem->used == 0) {
        /* if mem->prev is unused, combine mem and mem->prev */
        if(lfree == mem) {
            lfree = pmem;
        }

        pmem->next = mem->next;
        ((struct heap_mem *)&heap_ptr[mem->next])->prev = (uint8_t *)pmem - heap_ptr;
    }
}

/**
 * @ingroup SystemInit
 *
 * This function will initialize system heap memory.
 *
 * @param begin_addr the beginning address of system heap memory.
 * @param end_addr the end address of system heap memory.
 */

void system_heap_init_arr(void)
{
    struct heap_mem *mem;
    uint32_t begin_align = ALIGN((uint32_t)heap, ALIGN_SIZE);
    uint32_t end_align = ALIGN_DOWN((uint32_t)(heap + SYS_HEAP_SIZE), ALIGN_SIZE);

    //DEBUG_NOT_IN_INTERRUPT;
    /* alignment addr */
    if((end_align > (2 * SIZEOF_STRUCT_MEM)) &&
       ((end_align - 2 * SIZEOF_STRUCT_MEM) >= begin_align)) {
        /* calculate the aligned memory size */
        mem_size_aligned = end_align - begin_align - 2 * SIZEOF_STRUCT_MEM;
    } else {
        return;
    }

    /* point to begin address of heap */
    heap_ptr = (uint8_t *)begin_align;
    /* initialize the start of the heap */
    mem        = (struct heap_mem *)heap_ptr;
    mem->magic = HEAP_MAGIC;
    mem->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    mem->prev  = 0;
    mem->used  = 0;
#ifdef USING_MEMTRACE
    mem_setname(mem, "INIT");
#endif
    /* initialize the end of the heap */
    heap_end        = (struct heap_mem *)&heap_ptr[mem->next];
    heap_end->magic = HEAP_MAGIC;
    heap_end->used  = 1;
    heap_end->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    heap_end->prev  = mem_size_aligned + SIZEOF_STRUCT_MEM;
#ifdef USING_MEMTRACE
    mem_setname(heap_end, "INIT");
#endif
    //sem_init(&heap_sem, "heap", 1, IPC_FLAG_FIFO);
    heap_sem = osSemaphoreNew(1, 0, NULL);
    /* initialize the lowest-free pointer to the start of the heap */
    lfree = (struct heap_mem *)heap_ptr;
}

void system_heap_init(void *begin_addr, void *end_addr)
{
    struct heap_mem *mem;
    uint32_t begin_align = ALIGN((uint32_t)begin_addr, ALIGN_SIZE);
    uint32_t end_align = ALIGN_DOWN((uint32_t)end_addr, ALIGN_SIZE);

    //DEBUG_NOT_IN_INTERRUPT;
    /* alignment addr */
    if((end_align > (2 * SIZEOF_STRUCT_MEM)) &&
       ((end_align - 2 * SIZEOF_STRUCT_MEM) >= begin_align)) {
        /* calculate the aligned memory size */
        mem_size_aligned = end_align - begin_align - 2 * SIZEOF_STRUCT_MEM;
    } else {
#if MEM_DEBUG
        printf("mem init, error begin address 0x%x, and end address 0x%x\n",
               (uint32_t)begin_addr, (uint32_t)end_addr);
#endif
        return;
    }

    /* point to begin address of heap */
    heap_ptr = (uint8_t *)begin_align;
    /* initialize the start of the heap */
    mem        = (struct heap_mem *)heap_ptr;
    mem->magic = HEAP_MAGIC;
    mem->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    mem->prev  = 0;
    mem->used  = 0;
#ifdef USING_MEMTRACE
    mem_setname(mem, "INIT");
#endif
    /* initialize the end of the heap */
    heap_end        = (struct heap_mem *)&heap_ptr[mem->next];
    heap_end->magic = HEAP_MAGIC;
    heap_end->used  = 1;
    heap_end->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    heap_end->prev  = mem_size_aligned + SIZEOF_STRUCT_MEM;
#ifdef USING_MEMTRACE
    mem_setname(heap_end, "INIT");
#endif
    //sem_init(&heap_sem, "heap", 1, IPC_FLAG_FIFO);
    heap_sem = osSemaphoreNew(1, 1, NULL);
    /* initialize the lowest-free pointer to the start of the heap */
    lfree = (struct heap_mem *)heap_ptr;
}

/**
 * @addtogroup MM
 */

/**@{*/

/**
 * Allocate a block of memory with a minimum of 'size' bytes.
 *
 * @param size is the minimum size of the requested block in bytes.
 *
 * @return pointer to allocated memory or NULL if no free memory was found.
 */
#include "stm32f4xx.h"

void *sys_malloc(size_t size)
{
    size_t ptr, ptr2;
    struct heap_mem *mem, *mem2;
    //Logger_printf("malloc:%d\r\n",size);
    if(size == 0)
    { return NULL; }

    //DEBUG_NOT_IN_INTERRUPT;
    /* alignment size */
    size = ALIGN(size, ALIGN_SIZE);

    if(size > mem_size_aligned) {
        return NULL;
    }

    /* every data block must be at least MIN_SIZE_ALIGNED long */
    if(size < MIN_SIZE_ALIGNED)
    { size = MIN_SIZE_ALIGNED; }

    /* take memory semaphore */
    //sem_take(&heap_sem, WAITING_FOREVER);
    osSemaphoreAcquire(heap_sem, osWaitForever);

    for(ptr = (uint8_t *)lfree - heap_ptr;
        ptr < mem_size_aligned - size;
        ptr = ((struct heap_mem *)&heap_ptr[ptr])->next) {
        mem = (struct heap_mem *)&heap_ptr[ptr];

        if((!mem->used) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size) {
            /* mem is not used and at least perfect fit is possible:
             * mem->next - (ptr + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */
            if(mem->next - (ptr + SIZEOF_STRUCT_MEM) >=
               (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED)) {
                /* (in addition to the above, we test if another struct heap_mem (SIZEOF_STRUCT_MEM) containing
                 * at least MIN_SIZE_ALIGNED of data also fits in the 'user data space' of 'mem')
                 * -> split large block, create empty remainder,
                 * remainder must be large enough to contain MIN_SIZE_ALIGNED data: if
                 * mem->next - (ptr + (2*SIZEOF_STRUCT_MEM)) == size,
                 * struct heap_mem would fit in but no data between mem2 and mem2->next
                 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
                 *       region that couldn't hold data, but when mem->next gets freed,
                 *       the 2 regions would be combined, resulting in more free memory
                 */
                ptr2 = ptr + SIZEOF_STRUCT_MEM + size;
                /* create mem2 struct */
                mem2       = (struct heap_mem *)&heap_ptr[ptr2];
                mem2->magic = HEAP_MAGIC;
                mem2->used = 0;
                mem2->next = mem->next;
                mem2->prev = ptr;
#ifdef USING_MEMTRACE
                mem_setname(mem2, "    ");
#endif
                /* and insert it between mem and mem->next */
                mem->next = ptr2;
                mem->used = 1;

                if(mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM) {
                    ((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
                }

#ifdef MEM_STATS
                used_mem += (size + SIZEOF_STRUCT_MEM);

                if(max_mem < used_mem)
                { max_mem = used_mem; }

#endif
            } else {
                /* (a mem2 struct does no fit into the user data space of mem and mem->next will always
                 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
                 * take care of this).
                 * -> near fit or excact fit: do not split, no mem2 creation
                 * also can't move mem->next directly behind mem, since mem->next
                 * will always be used at this point!
                 */
                mem->used = 1;
#ifdef MEM_STATS
                used_mem += mem->next - ((uint8_t *)mem - heap_ptr);

                if(max_mem < used_mem)
                { max_mem = used_mem; }

#endif
            }

            /* set memory block magic */
            mem->magic = HEAP_MAGIC;
#ifdef USING_MEMTRACE

            if(thread_self())
            { mem_setname(mem, thread_self()->name); }
            else
            { mem_setname(mem, "NONE"); }

#endif

            if(mem == lfree) {
                /* Find next free block after mem and update lowest free pointer */
                while(lfree->used && lfree != heap_end)
                { lfree = (struct heap_mem *)&heap_ptr[lfree->next]; }

                //ASSERT(((lfree == heap_end) || (!lfree->used)));
            }
            
            

            //sem_release(&heap_sem);
            osSemaphoreRelease(heap_sem);
            //ASSERT((uint32_t)mem + SIZEOF_STRUCT_MEM + size <= (uint32_t)heap_end);
            //ASSERT((uint32_t)((uint8_t *)mem + SIZEOF_STRUCT_MEM) % ALIGN_SIZE == 0);
            //ASSERT((((uint32_t)mem) & (ALIGN_SIZE - 1)) == 0);
            OBJECT_HOOK_CALL(malloc_hook,
                             (((void *)((uint8_t *)mem + SIZEOF_STRUCT_MEM)), size));
            memset(((void *)((uint8_t *)mem + SIZEOF_STRUCT_MEM)),0, size);
            /* return the memory data except mem struct */
            return (uint8_t *)mem + SIZEOF_STRUCT_MEM;
        }
    }

    //sem_release(&heap_sem);
    osSemaphoreRelease(heap_sem);
    //Logger_sendString("malloc_err\r\n");
    //__disable_irq(); 
    //NVIC_SystemReset();
    
    return NULL;
}

/**
 * This function will change the previously allocated memory block.
 *
 * @param rmem pointer to memory allocated by malloc
 * @param newsize the required new size
 *
 * @return the changed memory block address
 */
void *sys_realloc(void *rmem, size_t newsize)
{
    size_t size;
    size_t ptr, ptr2;
    struct heap_mem *mem, *mem2;
    void *nmem;
    //DEBUG_NOT_IN_INTERRUPT;
    /* alignment size */
    newsize = ALIGN(newsize, ALIGN_SIZE);

    if(newsize > mem_size_aligned) {
        //DEBUG_LOG(DEBUG_MEM, ("realloc: out of memory\n"));
        return NULL;
    } else if(newsize == 0) {
        sys_free(rmem);
        return NULL;
    }

    /* allocate a new memory block */
    if(rmem == NULL)
    { return sys_malloc(newsize); }

    osSemaphoreAcquire(heap_sem, osWaitForever);

    if((uint8_t *)rmem < (uint8_t *)heap_ptr ||
       (uint8_t *)rmem >= (uint8_t *)heap_end) {
        /* illegal memory */
        osSemaphoreRelease(heap_sem);
        return rmem;
    }

    mem = (struct heap_mem *)((uint8_t *)rmem - SIZEOF_STRUCT_MEM);
    ptr = (uint8_t *)mem - heap_ptr;
    size = mem->next - ptr - SIZEOF_STRUCT_MEM;

    if(size == newsize) {
        /* the size is the same as */
        osSemaphoreRelease(heap_sem);
        return rmem;
    }

    if(newsize + SIZEOF_STRUCT_MEM + MIN_SIZE < size) {
        /* split memory block */
#ifdef MEM_STATS
        used_mem -= (size - newsize);
#endif
        ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
        mem2 = (struct heap_mem *)&heap_ptr[ptr2];
        mem2->magic = HEAP_MAGIC;
        mem2->used = 0;
        mem2->next = mem->next;
        mem2->prev = ptr;
#ifdef USING_MEMTRACE
        mem_setname(mem2, "    ");
#endif
        mem->next = ptr2;

        if(mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM) {
            ((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
        }

        plug_holes(mem2);
        osSemaphoreRelease(heap_sem);
        return rmem;
    }

    osSemaphoreRelease(heap_sem);
    /* expand memory */
    nmem = sys_malloc(newsize);

    if(nmem != NULL) {  /* check memory */
        memcpy(nmem, rmem, size < newsize ? size : newsize);
        sys_free(rmem);
    }

    return nmem;
}

/**
 * This function will contiguously allocate enough space for count objects
 * that are size bytes of memory each and returns a pointer to the allocated
 * memory.
 *
 * The allocated memory is filled with bytes of value zero.
 *
 * @param count number of objects to allocate
 * @param size size of the objects to allocate
 *
 * @return pointer to allocated memory / NULL pointer if there is an error
 */
void *sys_calloc(size_t count, size_t size)
{
    void *p;
    /* allocate 'count' objects of size 'size' */
    p = sys_malloc(count * size);

    /* zero the memory */
    if(p)
    { memset(p, 0, count * size); }

    return p;
}

/**
 * This function will release the previously allocated memory block by
 * malloc. The released memory block is taken back to system heap.
 *
 * @param rmem the address of memory which will be released
 */
void sys_free(void *rmem)
{
    struct heap_mem *mem;

    if(rmem == NULL) { 
        return; 
    }

    //DEBUG_NOT_IN_INTERRUPT;
    OBJECT_HOOK_CALL(free_hook, (rmem));

    if((uint8_t *)rmem < (uint8_t *)heap_ptr ||
       (uint8_t *)rmem >= (uint8_t *)heap_end) {
        return;
    }

    /* Get the corresponding struct heap_mem ... */
    mem = (struct heap_mem *)((uint8_t *)rmem - SIZEOF_STRUCT_MEM);
    
    /* ... which has to be in a used state ... */
    if(!mem->used || mem->magic != HEAP_MAGIC) {
        //printf("to free a bad data block\n");
        //printf("mem: 0x%08x, used flag: %d, magic code: 0x%04x\n", mem, mem->used, mem->magic);
        return;
    } 
    
    /* protect the heap from concurrent access */
    osSemaphoreAcquire(heap_sem, osWaitForever);

    //ASSERT(mem->used);
    //ASSERT(mem->magic == HEAP_MAGIC);
    /* ... and is now unused. */
    mem->used  = 0;
    mem->magic = HEAP_MAGIC;
#ifdef USING_MEMTRACE
    mem_setname(mem, "    ");
#endif

    if(mem < lfree) {
        /* the newly freed struct is now the lowest */
        lfree = mem;
    }

#ifdef MEM_STATS
    used_mem -= (mem->next - ((uint8_t *)mem - heap_ptr));
#endif
    /* finally, see if prev or next are free also */
    plug_holes(mem);
    osSemaphoreRelease(heap_sem);
}

#ifdef MEM_STATS
void memory_info(uint32_t *total,
                 uint32_t *used,
                 uint32_t *max_used)
{
    if(total != NULL)
    { *total = mem_size_aligned; }

    if(used  != NULL)
    { *used = used_mem; }

    if(max_used != NULL)
    { *max_used = max_mem; }
}

uint32_t memory_get_totalsize(void)
{
    return mem_size_aligned;
}

uint32_t memory_get_used(void)
{
    return used_mem;
}

uint32_t memory_get_max_used(void)
{
    return max_mem;
}

#ifdef USING_FINSH
#include <finsh.h>

void list_mem(void)
{
    printf("total memory: %d\n", mem_size_aligned);
    printf("used memory : %d\n", used_mem);
    printf("maximum allocated memory: %d\n", max_mem);
}
FINSH_FUNCTION_EXPORT(list_mem, list memory usage information)

#ifdef USING_MEMTRACE
int memcheck(void)
{
    int position;
    uint32_t level;
    struct heap_mem *mem;
    level = hw_interrupt_disable();

    for(mem = (struct heap_mem *)heap_ptr; mem != heap_end; mem = (struct heap_mem *)&heap_ptr[mem->next]) {
        position = (uint32_t)mem - (uint32_t)heap_ptr;

        if(position < 0) {
            goto __exit;
        }

        if(position > mem_size_aligned) {
            goto __exit;
        }

        if(mem->magic != HEAP_MAGIC) {
            goto __exit;
        }

        if(mem->used != 0 && mem->used != 1) {
            goto __exit;
        }
    }

    hw_interrupt_enable(level);
    return 0;
__exit:
    printf("Memory block wrong:\n");
    printf("address: 0x%08x\n", mem);
    printf("  magic: 0x%04x\n", mem->magic);
    printf("   used: %d\n", mem->used);
    printf("  size: %d\n", mem->next - position - SIZEOF_STRUCT_MEM);
    hw_interrupt_enable(level);
    return 0;
}
MSH_CMD_EXPORT(memcheck, check memory data);

int memtrace(int argc, char **argv)
{
    struct heap_mem *mem;
    list_mem();
    printf("\nmemory heap address:\n");
    printf("heap_ptr: 0x%08x\n", heap_ptr);
    printf("lfree   : 0x%08x\n", lfree);
    printf("heap_end: 0x%08x\n", heap_end);
    printf("\n--memory item information --\n");

    for(mem = (struct heap_mem *)heap_ptr; mem != heap_end; mem = (struct heap_mem *)&heap_ptr[mem->next]) {
        int position = (uint32_t)mem - (uint32_t)heap_ptr;
        int size;
        printf("[0x%08x - ", mem);
        size = mem->next - position - SIZEOF_STRUCT_MEM;

        if(size < 1024)
        { printf("%5d", size); }
        else if(size < 1024 * 1024)
        { printf("%4dK", size / 1024); }
        else
        { printf("%4dM", size / (1024 * 1024)); }

        printf("] %c%c%c%c", mem->thread[0], mem->thread[1], mem->thread[2], mem->thread[3]);

        if(mem->magic != HEAP_MAGIC)
        { printf(": ***\n"); }
        else
        { printf("\n"); }
    }

    return 0;
}
MSH_CMD_EXPORT(memtrace, dump memory trace information);
#endif /* end of USING_MEMTRACE */
#endif /* end of USING_FINSH    */

#endif

/**@}*/
