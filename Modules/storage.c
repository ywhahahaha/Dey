#include <stdint.h>
#include "stm32f4xx.h"
#include "bsp_flash.h"
#include "storage.h"

#define TRY_TIMES 3

void storage_readbytes(uint32_t addr,uint8_t *pdata,uint16_t length) {
    Bsp_FlashRead(addr,(uint32_t *)pdata,length);
}

void storage_readwords(uint32_t addr,uint32_t *pdata,uint16_t length) {
    Bsp_FlashRead(addr,(uint32_t *)pdata,length);
}

bool storage_writebytes(uint32_t addr,uint8_t *pdata,uint16_t length) {
    bool ret = false;
    __disable_irq();
    for(uint8_t try_times = 0;try_times < TRY_TIMES; try_times++) {
        ret = Bsp_FlashProgram(addr,(uint32_t *)pdata,length); 
        if(ret) {
            break;
        }
    }
    __enable_irq();
    
    return ret;
}

bool storage_erasesector(uint32_t addr) {
    bool ret = false;
    __disable_irq();
    for(uint8_t try_times = 0;try_times < TRY_TIMES; try_times++) {
        ret = Bsp_FlashEraseSector(addr);
        if(ret) {
            break;
        }
    }
    
    __enable_irq();
    
    return ret;
}

void storage_init(void) {
    
    HAL_FLASH_Unlock(); 
    
    __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

}
