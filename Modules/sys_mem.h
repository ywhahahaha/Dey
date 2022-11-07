#ifndef __SYS_MEM_H_
#define __SYS_MEM_H_

#include <stdint.h>

#if defined ( __CC_ARM )
extern int Image$$RW_IRAM1$$ZI$$Limit;

#endif
#define HEAP_BEGIN ( void *)0x10000000//((void *)&Image$$RW_IRAM1$$ZI$$Limit)//
#define HEAP_END   ((void *)(0x10000000 + 0x10000))//((void *)(0x20000000 + 0x20000))//
#define SYS_HEAP_SIZE  (size_t)4//20480

void system_heap_init(void *begin_addr, void *end_addr);
void *sys_malloc(size_t size);
void *sys_realloc(void *rmem, size_t newsize);
void *sys_calloc(size_t count, size_t size);
void sys_free(void *rmem);

uint32_t memory_get_totalsize(void);
uint32_t memory_get_used(void);
uint32_t memory_get_max_used(void);

void memory_info(uint32_t *total,
                 uint32_t *used,
                 uint32_t *max_used);

#endif
