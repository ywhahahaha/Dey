#include "cmsis_os2.h"
#include <stdint.h>
#include "project_config.h"


#if OS_TYPE == OS_FREERTOS
#include "FreeRTOS.h"
void *sys_malloc(size_t size) {
    void *mem = (void *)pvPortMalloc(size);
    if(mem == NULL) {
        return NULL;
    }
    
    memset(mem,0,size);
    return mem;
}

void sys_free(void *mem) {
    vPortFree(mem);
}
#endif


