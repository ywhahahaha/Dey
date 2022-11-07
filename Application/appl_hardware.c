#include "main.h"
#include "appl_hardware.h"
#include "bsp_timer.h"

void MX_USART6_UART_DeInit(void);
void MX_FSMC_Init(void);
void Bsp_Rs485Uplink1Init(void);
void Bsp_Rs485Uplink2Init(void);
void Bsp_pvdInit(void);
void storage_init(void);

void appl_hardware_init(void) {
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();
    __GPIOH_CLK_ENABLE();
    
    MX_RTC_Init();
    
    Bsp_GpioInit();
    
    storage_init();

    MX_FSMC_Init();
    
    Bsp_Rs485_1_Init(pDEApp->device_config.i03m.rs485_cfg[0].baudrate); //4g,eth.
    Bsp_Rs485_2_Init(pDEApp->device_config.i03m.rs485_cfg[1].baudrate); //lcd.
    Bsp_Rs485_3_Init(pDEApp->device_config.i03m.rs485_cfg[2].baudrate); //ext control.
    Bsp_Rs485_4_Init(pDEApp->device_config.i03m.rs485_cfg[3].baudrate); //i03t building net work. 

    Bsp_pvdInit();
    
    DATE_yymmddhhmmss_t now;
    Bsp_RtcGetTime(&now);
    srand(now.date);
    
    MX_IWDG_Init();
    
    Bsp_TimerInit();

}
