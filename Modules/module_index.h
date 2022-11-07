#ifndef __MODULE_INDEX_H__
#define __MODULE_INDEX_H__

#ifdef __cplusplus
 extern "C" {
#endif
#include <stdbool.h>     
#include <stdint.h>
#include <string.h>
#include "typedef.h"
#include "bsp_flash.h"
#include "storage.h"



#define MODULE_INDEX_MAX_SIZE              (128 * 1024u)
#define ADDR_FILE_INDEX_START              (ADDR_FILE_INDEX_SAVE)
#define ADDR_FILE_INDEX_END                (ADDR_FILE_INDEX_SAVE +  MODULE_INDEX_MAX_SIZE)


#define FILE_INDEX_SECTOR_SIZE             (128 * 1024u)
#define MAX_LOGGERS_OF_SECTOR              (FILE_INDEX_SECTOR_SIZE/sizeof(FileIndexStore_t))
#define FILE_INDEX_LOGGERS_MAX_NUMBER      (MODULE_INDEX_MAX_SIZE/sizeof(FileIndexStore_t))
    


bool FileIndex_save(FileIndexStore_t msg);
bool FileIndex_load(FileIndexStore_t *pIndex);

#ifdef __cplusplus
}
#endif

#endif
