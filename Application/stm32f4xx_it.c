/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os2.h"
#include "typedef.h"
#include "thread_debug.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart6;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */
    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */

enum { r0, r1, r2, r3, r12, lr, pc, psr};
char msg[80];
void stack_dump(uint32_t stack[])
{
	sprintf(msg, "r0  = 0x%08x\n", stack[r0]);  debug_sendstring(msg);
	sprintf(msg, "r1  = 0x%08x\n", stack[r1]);  debug_sendstring(msg);
	sprintf(msg, "r2  = 0x%08x\n", stack[r2]);  debug_sendstring(msg);
	sprintf(msg, "r3  = 0x%08x\n", stack[r3]);  debug_sendstring(msg);
	sprintf(msg, "r12 = 0x%08x\n", stack[r12]); debug_sendstring(msg);
	sprintf(msg, "lr  = 0x%08x\n", stack[lr]);  debug_sendstring(msg);
	sprintf(msg, "pc  = 0x%08x\n", stack[pc]);  debug_sendstring(msg);
	sprintf(msg, "psr = 0x%08x\n", stack[psr]); debug_sendstring(msg);
}

#include "cmsis_os2.h"
#include "logger.h"

#if OS_FREERTOS == OS_TYPE
#include "FreeRTOS.h"
#include "task.h"
#endif

osThreadId_t osThreadGetId (void);
extern void* volatile pxCurrentTCB;
void Hard_Fault_Handler(uint32_t stack[]) {
	
	stack_dump(stack);
    
    logger_msg_t log = {0};
    
    log.type = LOGGER_EXCEPT;
    for(uint8_t i=0;i<8;i++) {
        log.v.d4[i] = stack[i];
    }
    
#if OS_TYPE == OS_FREERTOS
    log.v.d4[8] = (uint32_t)pxCurrentTCB;
    strcpy((char *)log.file,pcTaskGetTaskName(pxCurrentTCB));
#endif
    
#if OS_TYPE == OS_RTX5
    #include "rtx_os.h"
    log.v.d4[8] = (uint32_t)osThreadGetId();

    osRtxThread_t *thread = (osRtxThread_t *)(log.v.d4[8]);
    if(thread != NULL) {
        int length = strlen(thread->name);
        if(length < sizeof(log.file)) {
            memcpy((char *)log.file,thread->name,length);
        } else {
            memcpy((char *)log.file,thread->name,sizeof(log.file)-2);
        }
    }
#endif
    
    logger_infor_save(log);
    
    sprintf(msg, "thread:%08x,%s\r\n", log.v.d4[8], log.file);
    debug_sendstring(msg);
	
	if ((SCB->HFSR & (1 << 30)) != 0) {
       debug_sendstring("Forced Hard Fault\n");
       sprintf(msg, "SCB->CFSR = 0x%08x\n", SCB->CFSR );
       debug_sendstring(msg);
   }

	__ASM volatile("BKPT #01");

    while(1) {
		
    }
}

#if defined(__CC_ARM)
__asm void HardFault_Handler(void)
{
  TST lr, #4     // Test for MSP or PSP
  ITE EQ
  MRSEQ r0, MSP
  MRSNE r0, PSP
  B __cpp(Hard_Fault_Handler)
}
#else
void HardFault_Handler(void)
{
   __asm(   "TST lr, #4          \n"
            "ITE EQ              \n"
            "MRSEQ r0, MSP       \n"
            "MRSNE r0, PSP       \n"
            "B Hard_Fault_Handler\n"
   );
}
#endif


/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
    /* USER CODE BEGIN MemoryManagement_IRQn 0 */
    /* USER CODE END MemoryManagement_IRQn 0 */
    debug_sendstring("MemManage_Handler");
    while(1) {
        /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
        /* USER CODE END W1_MemoryManagement_IRQn 0 */
    }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
    /* USER CODE BEGIN BusFault_IRQn 0 */
    /* USER CODE END BusFault_IRQn 0 */
    debug_sendstring("BusFault_Handler");
    while(1) {
        /* USER CODE BEGIN W1_BusFault_IRQn 0 */
        /* USER CODE END W1_BusFault_IRQn 0 */
    }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
    /* USER CODE BEGIN UsageFault_IRQn 0 */
    /* USER CODE END UsageFault_IRQn 0 */
    debug_sendstring("UsageFault_Handler");
    while(1) {
        /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
        /* USER CODE END W1_UsageFault_IRQn 0 */
    }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
    /* USER CODE BEGIN DebugMonitor_IRQn 0 */
    /* USER CODE END DebugMonitor_IRQn 0 */
    /* USER CODE BEGIN DebugMonitor_IRQn 1 */
    /* USER CODE END DebugMonitor_IRQn 1 */
}

#if 0
/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
    /* USER CODE BEGIN SysTick_IRQn 0 */
    /* USER CODE END SysTick_IRQn 0 */
    HAL_IncTick();
    //osSystickHandler();
    /* USER CODE BEGIN SysTick_IRQn 1 */
    /* USER CODE END SysTick_IRQn 1 */
}
#endif
/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI0_IRQn 0 */
    /* USER CODE END EXTI0_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    /* USER CODE BEGIN EXTI0_IRQn 1 */
    /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI2_IRQn 0 */
    /* USER CODE END EXTI2_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    /* USER CODE BEGIN EXTI2_IRQn 1 */
    /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line3 interrupt.
  */
void EXTI3_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI3_IRQn 0 */
    /* USER CODE END EXTI3_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
    /* USER CODE BEGIN EXTI3_IRQn 1 */
    /* USER CODE END EXTI3_IRQn 1 */
}

/**
  * @brief This function handles EXTI line4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI4_IRQn 0 */
    /* USER CODE END EXTI4_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    /* USER CODE BEGIN EXTI4_IRQn 1 */
    /* USER CODE END EXTI4_IRQn 1 */
}


/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI9_5_IRQn 0 */
    /* USER CODE END EXTI9_5_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    /* USER CODE BEGIN EXTI9_5_IRQn 1 */
    /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream1 global interrupt.
  */
void DMA1_Stream1_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */
    /* USER CODE END DMA1_Stream1_IRQn 0 */
    //HAL_DMA_IRQHandler(&hdma_usart3_rx);
    /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */
    /* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream3 global interrupt.
  */
void DMA1_Stream3_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Stream3_IRQn 0 */
    /* USER CODE END DMA1_Stream3_IRQn 0 */
    //HAL_DMA_IRQHandler(&hdma_usart3_tx);
    /* USER CODE BEGIN DMA1_Stream3_IRQn 1 */
    /* USER CODE END DMA1_Stream3_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream5 global interrupt.
  */
void DMA1_Stream5_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */
    /* USER CODE END DMA1_Stream5_IRQn 0 */
    //HAL_DMA_IRQHandler(&hdma_usart2_rx);
    /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */
    /* USER CODE END DMA1_Stream5_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream6 global interrupt.
  */
void DMA1_Stream6_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Stream6_IRQn 0 */
    /* USER CODE END DMA1_Stream6_IRQn 0 */
    //HAL_DMA_IRQHandler(&hdma_usart2_tx);
    /* USER CODE BEGIN DMA1_Stream6_IRQn 1 */
    /* USER CODE END DMA1_Stream6_IRQn 1 */
}


void USART1_IRQHandler(void)
{

    uint32_t isrflags = huart1.Instance->SR;
    uint32_t cr1its =  huart1.Instance->CR1;
    uint32_t cr3its = huart1.Instance->CR3;

    if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) {
        uint8_t temp = (uint8_t)(huart1.Instance->DR & (uint8_t)0x00FF);

        if(protocolRxMsg.length < COM_DEBUG_RX_SIZE) {
            protocolRxMsg.buffer[protocolRxMsg.length++] = temp;
            protocolRxMsg.tick = osKernelGetTickCount() + PROTOCOL_I03T_TIME_OUT;
        }
    }

    if((isrflags & USART_SR_ORE) != RESET) {
        huart1.Instance->DR;
        huart1.Instance->SR;
    }

    if(((isrflags & USART_SR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET)) {
        huart1.Instance->DR;
    }

}



void USART2_IRQHandler(void)
{

    uint32_t isrflags = huart2.Instance->SR;
    uint32_t cr1its =  huart2.Instance->CR1;
    uint32_t cr3its = huart2.Instance->CR3;

    if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) {
        uint8_t temp = (uint8_t)(huart2.Instance->DR & (uint8_t)0x00FF);

        if(RxMsg485_3.length < PROTOCOL_RX_SIZE) {
            RxMsg485_3.buffer[RxMsg485_3.length++] = temp;
            RxMsg485_3.tick = osKernelGetTickCount() + PROTOCOL_TIME_OUT_1;
            
        }
    }

    if((isrflags & USART_SR_ORE) != RESET) {
        huart2.Instance->DR;
        huart2.Instance->SR;
    }

    if(((isrflags & USART_SR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET)) {
        huart2.Instance->DR;
    }

}


void USART3_IRQHandler(void)
{
    uint32_t isrflags = huart3.Instance->SR;
    uint32_t cr1its =  huart3.Instance->CR1;
    uint32_t cr3its = huart3.Instance->CR3;

    if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) {
        uint8_t temp = (uint8_t)(huart3.Instance->DR & (uint8_t)0x00FF);
        if(RxMsg485_2.length < PROTOCOL_RX_SIZE) {
            RxMsg485_2.buffer[RxMsg485_2.length++] = temp;
            RxMsg485_2.tick = osKernelGetTickCount() + PROTOCOL_TIME_OUT_1;
        }
    } else if(((isrflags & USART_SR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET)) {
        huart3.Instance->DR;
    } else if((isrflags & USART_SR_ORE) != RESET) {
        huart3.Instance->DR;
        huart3.Instance->SR;
    } else {
        HAL_UART_IRQHandler(&huart3);
    }
}


void UART4_IRQHandler(void)
{

    uint32_t isrflags = huart4.Instance->SR;
    uint32_t cr1its =  huart4.Instance->CR1;
    uint32_t cr3its = huart4.Instance->CR3;

    if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) {
        uint8_t temp = (uint8_t)(huart4.Instance->DR & (uint8_t)0x00FF);

        if(comm_debug_msg.length < COM_DEBUG_RX_SIZE) {
            comm_debug_msg.buffer[comm_debug_msg.length++] = temp;
            comm_debug_msg.tick = osKernelGetTickCount() + PROTOCOL_TIME_OUT;
        }
    }

    if((isrflags & USART_SR_ORE) != RESET) {
        huart4.Instance->DR;
        huart4.Instance->SR;
    }

    if(((isrflags & USART_SR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET)) {
        huart4.Instance->DR;
    }
}


void USART6_IRQHandler(void)
{

    uint32_t isrflags = huart6.Instance->SR;
    uint32_t cr1its =  huart6.Instance->CR1;
    uint32_t cr3its = huart6.Instance->CR3;
    uint8_t temp;
    if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) {
        temp = (uint8_t)(huart6.Instance->DR & (uint8_t)0x00FF);
        if(RxMsg485_1.length < PROTOCOL_RX_SIZE) {
            RxMsg485_1.buffer[RxMsg485_1.length++] = temp;
            RxMsg485_1.tick = osKernelGetTickCount() + PROTOCOL_TIME_OUT_1;
        }
    } else if(((isrflags & USART_SR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET)) {
        temp = huart6.Instance->DR;
    } else if((isrflags & USART_SR_ORE) != RESET) {
        temp = huart6.Instance->DR;
        temp = huart6.Instance->SR;
    } else {
        HAL_UART_IRQHandler(&huart6);
    }
}


/**
  * @brief This function handles DMA2 stream0 global interrupt.
  */
void DMA2_Stream0_IRQHandler(void)
{
    /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */
    /* USER CODE END DMA2_Stream0_IRQn 0 */
    //HAL_DMA_IRQHandler(&hdma_adc1);
    /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */
    /* USER CODE END DMA2_Stream0_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream1 global interrupt.
  */
void DMA2_Stream1_IRQHandler(void)
{
    /* USER CODE BEGIN DMA2_Stream1_IRQn 0 */
    /* USER CODE END DMA2_Stream1_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart6_rx);
    /* USER CODE BEGIN DMA2_Stream1_IRQn 1 */
    /* USER CODE END DMA2_Stream1_IRQn 1 */
}

/**
  * @brief This function handles Ethernet global interrupt.
  */
void ETH_IRQHandler(void)
{
    /* USER CODE BEGIN ETH_IRQn 0 */
    /* USER CODE END ETH_IRQn 0 */
    //HAL_ETH_IRQHandler(&heth);
    /* USER CODE BEGIN ETH_IRQn 1 */
    /* USER CODE END ETH_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream6 global interrupt.
  */
void DMA2_Stream6_IRQHandler(void)
{
    /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */
    /* USER CODE END DMA2_Stream6_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart6_tx);
    /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */
    /* USER CODE END DMA2_Stream6_IRQn 1 */
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   
}


void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  //HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */

  /* USER CODE END TIM4_IRQn 1 */
}

extern TIM_HandleTypeDef    TimHandle;
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&TimHandle);
  /* USER CODE BEGIN TIM4_IRQn 1 */

  /* USER CODE END TIM4_IRQn 1 */
}



extern RTC_HandleTypeDef hrtc;
void RTC_Alarm_IRQHandler(void)
{
  HAL_RTC_AlarmIRQHandler(&hrtc);
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
