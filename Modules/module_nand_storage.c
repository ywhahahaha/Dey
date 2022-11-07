#include <stdint.h>
#include "module_nand_storage.h"
#include "stm32f4xx.h"
//return 1:busy,0:ready
int32_t Driver_NAND0_GetDeviceBusy(int devnum) {
    //busy gpio = 0,ready gpio = 1
    if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_6) == GPIO_PIN_RESET) {
        return 1;
    }
    
    return 0;
}


















