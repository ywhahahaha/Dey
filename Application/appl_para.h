#ifndef __APP_PARA_H_
#define __APP_PARA_H_
 
#include "typedef.h" 
#include "bsp_flash.h"
#include "storage.h"
  
void appl_para_factory(void);
errStatus_t appl_para_load(void);
errStatus_t appl_para_save(void);
void appl_para_init(void);

#endif
