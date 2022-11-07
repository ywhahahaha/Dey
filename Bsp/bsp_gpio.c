#include "cmsis_os2.h"
#include "bsp_timer.h"
#include "bsp_gpio.h"

void Bsp_GpioInit(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = LED_FLT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(LED_FLT_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = LED_COMMU_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(LED_COMMU_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = LED_Running_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(LED_Running_Port, &GPIO_InitStruct); 
    
    
    
    
    GPIO_InitStruct.Pin = RS485_1_DE_RE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RS485_1_DE_RE_GPIO_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = RS485_2_DE_RE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RS485_2_DE_RE_GPIO_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = RS485_3_DE_RE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RS485_3_DE_RE_GPIO_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = RS485_4_DE_RE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RS485_4_DE_RE_GPIO_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = COM1_POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(COM1_POWER_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = COM2_POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(COM2_POWER_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = COM3_POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(COM3_POWER_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = COM4_POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(COM4_POWER_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = DRY_POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DRY_POWER_Port, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = BUZZER_POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BUZZER_POWER_Port, &GPIO_InitStruct); 
    
    
    Bsp_LedErrorOff();

    Bsp_Com1Enable();
    Bsp_Com2Enable();
    Bsp_Com3Enable();
    Bsp_Com4Enable();
    
    
    Bsp_BeepOff();
    Bsp_DryPowerDisable();
}

void Bsp_Com1Enable(void) {
    HAL_GPIO_WritePin(COM1_POWER_Port,COM1_POWER_Pin,GPIO_PIN_SET);
}

void Bsp_Com1Disable(void) {
    HAL_GPIO_WritePin(COM1_POWER_Port,COM1_POWER_Pin,GPIO_PIN_RESET);
}

void Bsp_Com2Enable(void) {
    HAL_GPIO_WritePin(COM2_POWER_Port,COM2_POWER_Pin,GPIO_PIN_SET);
}

void Bsp_Com2Disable(void) {
    HAL_GPIO_WritePin(COM2_POWER_Port,COM2_POWER_Pin,GPIO_PIN_RESET);
}

void Bsp_Com3Enable(void) {
    HAL_GPIO_WritePin(COM3_POWER_Port,COM3_POWER_Pin,GPIO_PIN_SET);
}

void Bsp_Com3Disable(void) {
    HAL_GPIO_WritePin(COM3_POWER_Port,COM3_POWER_Pin,GPIO_PIN_RESET);
}

void Bsp_Com4Enable(void) {
    HAL_GPIO_WritePin(COM4_POWER_Port,COM4_POWER_Pin,GPIO_PIN_SET);
}

void Bsp_Com4Disable(void) {
    HAL_GPIO_WritePin(COM4_POWER_Port,COM4_POWER_Pin,GPIO_PIN_RESET);
}

void Bsp_DryPowerEnable(void) {
    HAL_GPIO_WritePin(DRY_POWER_Port,DRY_POWER_Pin,GPIO_PIN_SET);
}

void Bsp_DryPowerDisable(void) {
    HAL_GPIO_WritePin(DRY_POWER_Port,DRY_POWER_Pin,GPIO_PIN_RESET);
}

void Bsp_BeepOn(void) {
    HAL_GPIO_WritePin(BUZZER_POWER_Port,BUZZER_POWER_Pin,GPIO_PIN_SET);
}

void Bsp_BeepOff(void) {
    HAL_GPIO_WritePin(BUZZER_POWER_Port,BUZZER_POWER_Pin,GPIO_PIN_RESET);
}


void Bsp_BeepToggle(void) {
    HAL_GPIO_TogglePin(BUZZER_POWER_Port,BUZZER_POWER_Pin);
}

void Bsp_Beeps(uint8_t times) {
    for(uint8_t i=0;i<times;i++) {
        BSP_BEEP_ON();
        osDelay(500);
        BSP_BEEP_OFF();
        osDelay(500);
    }
    Bsp_BeepOff();
}

void Bsp_LedRunningToggle(void) {
   HAL_GPIO_TogglePin(LED_Running_Port,LED_Running_Pin);
}

void Bsp_LedRunningOff(void) {
   HAL_GPIO_WritePin(LED_Running_Port,LED_Running_Pin,GPIO_PIN_SET);
}

void Bsp_LedErrorOn(void) {
    HAL_GPIO_WritePin(LED_FLT_Port,LED_FLT_Pin,GPIO_PIN_RESET);
}

void Bsp_LedErrorOff(void) {
    HAL_GPIO_WritePin(LED_FLT_Port,LED_FLT_Pin,GPIO_PIN_SET);
}

void Bsp_LedErrorToggle(void) {
    HAL_GPIO_TogglePin(LED_FLT_Port,LED_FLT_Pin);
}


void Bsp_LedCommuOn(void) {
    HAL_GPIO_WritePin(LED_COMMU_Port,LED_COMMU_Pin,GPIO_PIN_RESET);
}

void Bsp_LedCommuOff(void) {
    HAL_GPIO_WritePin(LED_COMMU_Port,LED_COMMU_Pin,GPIO_PIN_SET);
}

void Bsp_LedCommuToggle(void) {
    HAL_GPIO_TogglePin(LED_COMMU_Port,LED_COMMU_Pin);
}


