#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "stm32f4xx.h"
#include "typedef.h"
#include "module_index.h"
#include "crc_check.h"
#include "storage.h"
#include "thread_debug.h"


static uint32_t _index = 0;




/*
get current logger index.
*/
uint32_t FileIndex_get(void) {
	return _index;
}

/*
save the information.
*/
bool FileIndex_save(FileIndexStore_t msg) {

    FileIndexStore_t log = {0};

	/* mode index in total index numbers */
	uint16_t indexofMod = _index % FILE_INDEX_LOGGERS_MAX_NUMBER;
	/* current index page address */
	uint32_t pageAddr = ADDR_FILE_INDEX_START;
	
    /* aligne the erase address */
	if((_index % FILE_INDEX_LOGGERS_MAX_NUMBER) == 0) {
		storage_erasesector(pageAddr);
	}
    
    /* current write address */
	uint32_t addr = ADDR_FILE_INDEX_START + indexofMod * sizeof(FileIndexStore_t);
    
    storage_readbytes(addr,(uint8_t*)&log,sizeof(FileIndexStore_t));
    uint8_t *p = (uint8_t*)&log;
    for(uint8_t i=0;i<sizeof(FileIndexStore_t);i++) {
        if(p[i] != 0xff) {
            storage_erasesector(pageAddr);
            break;
        }
    }
    
    memset(&log,0,sizeof(FileIndexStore_t));
    memcpy(&log,&msg,sizeof(FileIndexStore_t));
    
	log.index = _index;
    log.crc_chk = CRC_Get32((uint8_t *)&log + 4,sizeof(FileIndexStore_t) - 4); 
    
    _index++;

    for(uint8_t i=0;i<3;i++) {
        
        FileIndexStore_t temp = {0};
        
        storage_writebytes(addr,(uint8_t *)&log, sizeof(FileIndexStore_t));
        
        storage_readbytes(addr,(uint8_t *)&temp, sizeof(FileIndexStore_t));
        
        if(!memcmp(&log,&temp,sizeof(FileIndexStore_t))) {
            return true;
        }
        
        osDelay(5);
    }
    
    return false;
}

bool FileIndex_load(FileIndexStore_t *pIndex) {

	uint32_t addr = ADDR_FILE_INDEX_START;
    
	bool find = false;
    
	FileIndexStore_t logger = {0}; 

	for(uint32_t size = 0; size < MODULE_INDEX_MAX_SIZE; size += sizeof(FileIndexStore_t),addr += sizeof(FileIndexStore_t)) {
        
        if(size + sizeof(FileIndexStore_t) > MODULE_INDEX_MAX_SIZE) {
            break;
        }
        
		storage_readbytes(addr,(uint8_t *)&logger,sizeof(FileIndexStore_t));
        
#if WATCH_DOG_ENABLE == 1
    hal_feed_dog();
#endif
	    if(logger.crc_chk == CRC_Get32((uint8_t *)&logger + 4,sizeof(FileIndexStore_t) - 4)) 
        {
            if(logger.index > _index) {
                _index = logger.index;
                memcpy(pIndex,&logger,sizeof(FileIndexStore_t));
            }
            find = true;
		}
	}

    if(find) {
        _index++;
        pIndex->index++;
    }     
    
    return find;
}

