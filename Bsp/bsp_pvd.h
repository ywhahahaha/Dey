#ifndef __BSP_PVD_H_
#define __BSP_PVD_H_

#ifdef __cplusplus
 extern "C" {
#endif
     
#include <stdint.h>
void Bsp_pvdInit(void);    
uint8_t bsp_pvd_get_power_flag(void);
     
#ifdef __cplusplus
}
#endif


#endif
