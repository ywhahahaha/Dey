#include <stdint.h>
#include "cmsis_os2.h"



void *sys_malloc(size_t size) {
    void *mem = pvPortMalloc(size);
    if(mem == NULL) {
        return NULL;
    }
    
    if((uint32_t)mem < 0x20000000) {
        return NULL;
    }
    
    memset(mem,0,size);
    return mem;
}

void sys_free(void *mem) {
    vPortFree(mem);
}
