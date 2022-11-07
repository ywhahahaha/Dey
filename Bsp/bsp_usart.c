/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp_gpio.h"

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart6_rx;
DMA_HandleTypeDef hdma_usart6_tx;

/* USART1 init function */
void MX_USART1_UART_DeInit(void) {
    HAL_UART_DeInit(&huart1);
}
void MX_USART1_UART_Init(uint32_t baudrate,uint8_t check_bits)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = baudrate;//
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = check_bits;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}
/* USART2 init function */
void MX_USART2_UART_DeInit(void) {
    HAL_UART_DeInit(&huart2);
}
void MX_USART2_UART_Init(uint32_t baudrate,uint8_t check_bits)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = baudrate;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = check_bits;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}
/* USART3 init function */
void MX_USART3_UART_DeInit(void) {
    HAL_UART_DeInit(&huart3);
}

void MX_USART3_UART_Init(uint32_t baudrate,uint8_t check_bits)
{
    HAL_UART_DeInit(&huart3);
    
    huart3.Instance = USART3;
    huart3.Init.BaudRate = baudrate;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = check_bits;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&huart3) != HAL_OK) {
        Error_Handler();
    }
}
void MX_USART4_UART_DeInit(void) {
    HAL_UART_DeInit(&huart4);
}

void MX_USART4_UART_Init(uint32_t baudrate,uint8_t check_bits)
{
    huart4.Instance = UART4;
    huart4.Init.BaudRate = baudrate;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = check_bits;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&huart4) != HAL_OK) {
        Error_Handler();
    }
}
/* USART6 init function */
void MX_USART6_UART_Init(uint32_t baudrate,uint8_t check_bits)
{
    huart6.Instance = USART6;
    huart6.Init.BaudRate = baudrate;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = check_bits;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&huart6) != HAL_OK) {
        Error_Handler();
    } 
}

void MX_USART6_UART_DeInit(void) {
    HAL_UART_DeInit(&huart6);
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(uartHandle->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspInit 0 */
        /* USER CODE END USART1_MspInit 0 */
        /* USART1 clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        /* USER CODE BEGIN USART1_MspInit 1 */
        __HAL_UART_ENABLE_IT(uartHandle, UART_IT_RXNE);
        //__HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE);
        HAL_NVIC_SetPriority(USART1_IRQn, 15, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        /* Enable the UART Data Register not empty Interrupt */
        /* USER CODE END USART1_MspInit 1 */
    } else if(uartHandle->Instance == USART2) {
       /* USER CODE BEGIN USART1_MspInit 0 */
        /* USER CODE END USART1_MspInit 0 */
        /* USART1 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        /* USER CODE BEGIN USART1_MspInit 1 */
        __HAL_UART_ENABLE_IT(uartHandle, UART_IT_RXNE);
        //__HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE);
        HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        /* Enable the UART Data Register not empty Interrupt */
        /* USER CODE END USART1_MspInit 1 */
    } else if(uartHandle->Instance == USART3) {
        /* USER CODE BEGIN USART3_MspInit 0 */

      /* USER CODE END USART3_MspInit 0 */
        /* USART3 clock enable */
        __HAL_RCC_USART3_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**USART3 GPIO Configuration
        PC10     ------> USART3_TX
        PC11     ------> USART3_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        
        __HAL_UART_ENABLE_IT(uartHandle, UART_IT_RXNE);
        /* USART3 interrupt Init */
        HAL_NVIC_SetPriority(USART3_IRQn, 15, 0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
      /* USER CODE BEGIN USART3_MspInit 1 */

      /* USER CODE END USART3_MspInit 1 */
    } else if(uartHandle->Instance == UART4) {
        /* USER CODE BEGIN USART3_MspInit 0 */

      /* USER CODE END USART3_MspInit 0 */
        /* USART3 clock enable */
        __HAL_RCC_UART4_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART3 GPIO Configuration
        PC10     ------> USART3_TX
        PC11     ------> USART3_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        __HAL_UART_ENABLE_IT(uartHandle, UART_IT_RXNE);
        /* USART3 interrupt Init */
        HAL_NVIC_SetPriority(UART4_IRQn, 15, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
      /* USER CODE BEGIN USART3_MspInit 1 */

      /* USER CODE END USART3_MspInit 1 */
    } else if(uartHandle->Instance == USART6) {
        /* USER CODE BEGIN USART1_MspInit 0 */
        /* USER CODE END USART1_MspInit 0 */
        /* USART1 clock enable */
        __HAL_RCC_USART6_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        /* USER CODE BEGIN USART1_MspInit 1 */
        __HAL_UART_ENABLE_IT(uartHandle, UART_IT_RXNE);
        //__HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE);
        HAL_NVIC_SetPriority(USART6_IRQn, 15, 0);
        HAL_NVIC_EnableIRQ(USART6_IRQn);
        /* Enable the UART Data Register not empty Interrupt */
        /* USER CODE END USART1_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
    if(uartHandle->Instance == USART1) {
        
    } else if(uartHandle->Instance == USART2) {
        
    } else if(uartHandle->Instance == USART3) {
        __HAL_RCC_USART3_FORCE_RESET();
        __HAL_RCC_USART3_RELEASE_RESET();
    } else if(uartHandle->Instance == USART6) {
        __HAL_RCC_USART6_FORCE_RESET();
        __HAL_RCC_USART6_RELEASE_RESET();
    }
}

/* USER CODE BEGIN 1 */


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2) { //rs485

    }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
