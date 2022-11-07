#ifndef __MAIN_H_
#define __MAIN_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "cmsis_os2.h"
#include "stm32f4xx.h"
#include "typedef.h"
#include "sys_mem.h"
     
#include "bsp_wdt.h"
#include "bsp_flash.h"
#include "bsp_fmc.h"
#include "bsp_gpio.h"
#include "bsp_rs485.h"
#include "bsp_usart.h"
#include "bsp_pvd.h"
#include "bsp_rtc.h"

#include "i03t_list.h"
#include "module_file_storage.h"
#include "project_config.h"
#include "logger.h"
#include "appl_noinit.h"

void debug_printf(const char *fmt, ...);

void Error_Handler(void);



//logger_infor_save_more(LOGGER_WARNING,LOGGER_MORE_NONE,0,__FUNCTION__,__FILE__,__LINE__);
#define configASSERT( x ) if ((x) == 0) {\
    for( ;; );} 

#define usbh_printf(...)    \
{   \
    debug_printf("[%s:%d]",    \
    __FUNCTION__, __LINE__);  \
    debug_printf(__VA_ARGS__);  \
}

#ifdef __cplusplus
}
#endif


#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
