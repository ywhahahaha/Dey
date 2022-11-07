#ifndef __MODULE_FILE_STORAGE_H_
#define __MODULE_FILE_STORAGE_H_

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>
#include <string.h>
#include "rl_fs.h"

typedef struct {
    const    char *file;
    uint16_t max_count;
    uint16_t storge_size;  
    uint16_t i03t_addr;    
}SYS_FILE_t;

void module_file_load(void);
void module_file_check(void);
uint32_t module_file_dir(void);
void module_file_load_all(void);
bool module_file_system_format(void);
bool module_file_new(char *path,uint8_t *pdata,uint32_t length);
void module_i03t_file_flush(uint8_t i03t_addr);
void module_i03t_del_hist(uint8_t i03t_addr);
void module_i03t_del_alarm(uint8_t i03t_addr);
void module_i03t_del_discharge(uint8_t i03t_addr);
    
#ifdef  __cplusplus
}
#endif



#endif
