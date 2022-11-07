#include "cmsis_os2.h"
#include "bsp_timer.h"
#include "bsp_gpio.h"

void Bsp_KeyInit(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = KEY_1_Pin ; 
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(KEY_1_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY_2_Pin ; 
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(KEY_2_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY_3_Pin ; 
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(KEY_3_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY_4_Pin ; 
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(KEY_4_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY_5_Pin ; 
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(KEY_5_Port, &GPIO_InitStruct);
  
    GPIO_InitStruct.Pin = KEY_6_Pin ; 
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(KEY_6_Port, &GPIO_InitStruct);
    
 
}

GPIO_PinState KeyBack (void) {
    return HAL_GPIO_ReadPin(KEY_1_Port,KEY_1_Pin);
}

GPIO_PinState KeyEnsur (void) {
    return HAL_GPIO_ReadPin(KEY_2_Port,KEY_2_Pin);
}

GPIO_PinState KeyRight (void) {
    return HAL_GPIO_ReadPin(KEY_3_Port,KEY_3_Pin);
}

GPIO_PinState KeyLeft (void) {
    return HAL_GPIO_ReadPin(KEY_4_Port,KEY_4_Pin);
}


GPIO_PinState KeyDown (void) {
    return HAL_GPIO_ReadPin(KEY_5_Port,KEY_5_Pin);
}

GPIO_PinState KeyUp (void) {
    return HAL_GPIO_ReadPin(KEY_6_Port,KEY_6_Pin);
}



