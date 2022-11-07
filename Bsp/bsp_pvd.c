#include <string.h>
#include "stm32f4xx.h"
#include "bsp_pvd.h"

PWR_PVDTypeDef sConfigPVD;

void Bsp_pvdInit(void) {
    /*##-1- Enable Power Clock #################################################*/
    __HAL_RCC_PWR_CLK_ENABLE();

    /*##-2- Configure the NVIC for PVD #########################################*/
    //HAL_NVIC_SetPriority(PVD_IRQn, 15, 0);
    //HAL_NVIC_EnableIRQ(PVD_IRQn);

    /* Configure the PVD Level to 3 and generate an interrupt on rising and falling
     edges(PVD detection level set to 2.5V, refer to the electrical characteristics
     of you device datasheet for more details) */
    sConfigPVD.PVDLevel = PWR_PVDLEVEL_7;
    sConfigPVD.Mode = PWR_PVD_MODE_IT_RISING_FALLING;
    HAL_PWR_ConfigPVD(&sConfigPVD);

    /* Enable the PVD Output */
    HAL_PWR_EnablePVD();
}

///static uint8_t power_down_flag = 0;

void HAL_PWR_PVDCallback(void) {
    //power_down_flag = (PWR->CSR & PWR_CSR_PVDO) ? 1 : 0;
}


// return 1 power down.
uint8_t bsp_pvd_get_power_flag(void) {
    volatile uint16_t delay = 0xff;
    while(delay--) {
        if(PWR->CSR & PWR_CSR_PVDO) {
            return 1;
        }
    }
    
    
    return 0;
}
