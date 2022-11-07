#ifndef __STORAGE_H_
#define __STORAGE_H_
#include <stdint.h>

#include "bsp_flash.h"

#define ADDR_BOOT_START       ADDR_FLASH_SECTOR_0  // 16K

#define ADDR_DEVICE_CONFIG    ADDR_FLASH_SECTOR_1  // 16K

#define ADDR_LOGGER_START     ADDR_FLASH_SECTOR_2  // 16K 16K 2&3

#define ADDR_APP_START        ADDR_FLASH_SECTOR_4  // 4(64),5(128),6(128)  320K
#define ADDR_APP_BACK_START   ADDR_FLASH_SECTOR_7  // 7(128),8(128),9(128) 384K

#define ADDR_FILE_INDEX_SAVE  ADDR_FLASH_SECTOR_10 // 128K
#define ADDR_SN_CONFIG        ADDR_FLASH_SECTOR_11 // 128K

void storage_readbytes(uint32_t addr,uint8_t *pdata,uint16_t length);
bool storage_writebytes(uint32_t addr,uint8_t *pdata,uint16_t length);
bool storage_erasesector(uint32_t addr);
void storage_init(void);

#endif

