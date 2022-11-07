#include <stdint.h>
#include "typedef.h"
#include "crc_check.h"
#include "project_config.h"

#if OS_TYPE == OS_FREERTOS
ApplNoInit_t appl_noinit __attribute__((section("NO_INIT"),zero_init));
#else
ApplNoInit_t appl_noinit __attribute__((at(0x1000FE00)));
#endif


errStatus_t appl_noinit_load(void) {
    
    uint32_t value = CRC_Get32((uint8_t *)(&appl_noinit) + 4,sizeof(ApplNoInit_t) - 4);
    
    if(appl_noinit.check == value && appl_noinit.magic == 0x55AA5A5A) {
        return errOK;
    }
    
    memset(&appl_noinit,0,sizeof(ApplNoInit_t));
    
    appl_noinit.magic = 0x55AA5A5Aul;
    
    appl_noinit.check = CRC_Get32((uint8_t *)(&appl_noinit) + 4,sizeof(ApplNoInit_t) - 4);
    
    return errErr;
}

void appl_noinit_store(void) {
    __disable_irq();
    appl_noinit.check = CRC_Get32((uint8_t *)(&appl_noinit) + 4,sizeof(ApplNoInit_t) - 4);
    __enable_irq();
}





