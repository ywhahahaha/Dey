#ifndef __BSP_GPIO_H_
#define __BSP_GPIO_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "stm32f4xx.h"

#define RS485_1_DE_RE_GPIO_Port    		GPIOC
#define RS485_1_DE_RE_Pin          		GPIO_PIN_8

#define RS485_2_DE_RE_GPIO_Port    		GPIOD
#define RS485_2_DE_RE_Pin          		GPIO_PIN_10

#define RS485_3_DE_RE_GPIO_Port    		GPIOA
#define RS485_3_DE_RE_Pin          		GPIO_PIN_4

#define RS485_4_DE_RE_GPIO_Port    		GPIOA
#define RS485_4_DE_RE_Pin          		GPIO_PIN_8



#define LED_COMMU_Pin                       GPIO_PIN_12
#define LED_COMMU_Port                      GPIOE
#define LED_FLT_Pin                         GPIO_PIN_13
#define LED_FLT_Port                        GPIOE
#define LED_Running_Pin                     GPIO_PIN_14
#define LED_Running_Port                    GPIOE

#define COM1_POWER_Port                     GPIOB
#define COM1_POWER_Pin                      GPIO_PIN_10

#define COM2_POWER_Port                     GPIOB
#define COM2_POWER_Pin                      GPIO_PIN_11

#define COM3_POWER_Port                     GPIOB
#define COM3_POWER_Pin                      GPIO_PIN_12

#define COM4_POWER_Port                     GPIOB
#define COM4_POWER_Pin                      GPIO_PIN_13

#define DRY_POWER_Port                      GPIOB
#define DRY_POWER_Pin                       GPIO_PIN_0

#define BUZZER_POWER_Port                   GPIOB
#define BUZZER_POWER_Pin                    GPIO_PIN_3

#define CARD1_POWER_Port                    GPIOC
#define CARD1_POWER_Pin                     GPIO_PIN_13

#define CARD2_POWER_Port                    GPIOB
#define CARD2_POWER_Pin                     GPIO_PIN_15

#define KEY_1_Port                          GPIOB
#define KEY_1_Pin                           GPIO_PIN_4
#define KEY_2_Port                          GPIOB
#define KEY_2_Pin                           GPIO_PIN_5
#define KEY_3_Port                          GPIOB
#define KEY_3_Pin                           GPIO_PIN_6
#define KEY_4_Port                          GPIOB
#define KEY_4_Pin                           GPIO_PIN_7
#define KEY_5_Port                          GPIOB
#define KEY_5_Pin                           GPIO_PIN_8
#define KEY_6_Port                          GPIOB
#define KEY_6_Pin                           GPIO_PIN_9



void Bsp_GpioInit(void);
void Bsp_LedRunningToggle(void);
void Bsp_LedRunningOff(void);
void Bsp_LedErrorOn(void);
void Bsp_LedErrorOff(void);
void Bsp_LedErrorToggle(void);
void Bsp_LedCommuOn(void);
void Bsp_LedCommuOff(void);
void Bsp_LedCommuToggle(void);


void Bsp_Com1Enable(void);
void Bsp_Com1Disable(void);
void Bsp_Com2Enable(void);
void Bsp_Com2Disable(void);
void Bsp_Com3Enable(void);
void Bsp_Com3Disable(void);
void Bsp_Com4Enable(void);
void Bsp_Com4Disable(void);
void Bsp_DryPowerEnable(void);
void Bsp_DryPowerDisable(void);
void Bsp_BeepOn(void);
void Bsp_BeepOff(void);
void Bsp_BeepToggle(void);
void Bsp_Beeps(uint8_t times);

GPIO_PinState Read_Key1 (void);
GPIO_PinState Read_Key2 (void);
GPIO_PinState Read_Key3 (void);
GPIO_PinState Read_Key4 (void);
GPIO_PinState Read_Key5 (void);
GPIO_PinState Read_Key6 (void);
void Bsp_KeyInit(void);


#define I03T_POWER_ENABLE   Bsp_Com3Enable

#define BSP_BEEP_ON()   Bsp_TimerStart()
#define BSP_BEEP_OFF()  Bsp_TimerStop()

#ifdef __cplusplus
}
#endif


#endif /*__ usart_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
