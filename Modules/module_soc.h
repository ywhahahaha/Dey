#ifndef __MODULE_SOC_H_
#define __MODULE_SOC_H_

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>
#include <math.h>
#include <stdlib.h>

uint16_t module_soc_calculate(uint16_t soc_now,uint16_t voltage,int16_t current,int16_t average_current,int16_t temp,uint8_t charge_state);


#ifdef  __cplusplus
}
#endif

#endif

