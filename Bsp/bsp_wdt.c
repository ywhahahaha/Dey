
#include "main.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

IWDG_HandleTypeDef hiwdg;

#define WATCH_DOG_ENABLE  1

/* IWDG init function */
void MX_IWDG_Init(void)
{
#if WATCH_DOG_ENABLE == 1
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
    hiwdg.Init.Reload = 4095;//1S

    if(HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        Error_Handler();
    }

#endif
}

/* USER CODE BEGIN 1 */
void feed_watchdog(void)
{
#if WATCH_DOG_ENABLE == 1

    if(HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {  //1s
        Error_Handler();
    }

#endif
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
