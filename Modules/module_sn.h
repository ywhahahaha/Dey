#ifndef __MODULE_SN_H__
#define __MODULE_SN_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>     
#include <stdint.h>
#include <string.h>
#include "typedef.h"
#include "bsp_flash.h"


#include "main.h"
#include "module_sn.h"
#include "crc_check.h"
#include "appl_para.h"
#include "thread_debug.h"


#define SN_FILE_SIZE  CONFIG_MAX_I03T * CONFIG_MAX_CELL * sizeof(SNStore_t)
    
uint16_t module_sn_get_count(uint8_t i03t,bool print);
SNStore_t *module_sn_get(uint8_t sn[]);    
SNStore_t *module_sn_get_by_index(uint8_t i03t_addr,uint16_t cell_id);
errStatus_t module_sn_find(uint8_t sn[]);
errStatus_t module_sn_add(SNStore_t *sn);
errStatus_t module_sn_add_mult(SNStore_t sn[],uint16_t num);
errStatus_t module_sn_replace(uint8_t i03t_addr,uint8_t sn_new[],uint8_t sn_old[]);
errStatus_t module_sn_delete(SNStore_t *sn) ;
errStatus_t module_sn_clear(uint8_t i03t_addr);



#ifdef __cplusplus
}
#endif

#endif
