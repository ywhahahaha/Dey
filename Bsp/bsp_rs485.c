#include <string.h>
#include "bsp_usart.h"
#include "bsp_rs485.h"
#include "bsp_gpio.h"
#include "stm32f4xx.h"
#include "cmsis_os2.h"

#define RS485_1_RE_RESET()           HAL_GPIO_WritePin(RS485_1_DE_RE_GPIO_Port, RS485_1_DE_RE_Pin, GPIO_PIN_RESET)
#define RS485_1_DE_SET()             HAL_GPIO_WritePin(RS485_1_DE_RE_GPIO_Port, RS485_1_DE_RE_Pin, GPIO_PIN_SET)
#define RS485_2_RE_RESET()           HAL_GPIO_WritePin(RS485_2_DE_RE_GPIO_Port, RS485_2_DE_RE_Pin, GPIO_PIN_RESET)
#define RS485_2_DE_SET()             HAL_GPIO_WritePin(RS485_2_DE_RE_GPIO_Port, RS485_2_DE_RE_Pin, GPIO_PIN_SET)
#define RS485_3_RE_RESET()           HAL_GPIO_WritePin(RS485_3_DE_RE_GPIO_Port, RS485_3_DE_RE_Pin, GPIO_PIN_RESET)
#define RS485_3_DE_SET()             HAL_GPIO_WritePin(RS485_3_DE_RE_GPIO_Port, RS485_3_DE_RE_Pin, GPIO_PIN_SET)
#define RS485_4_RE_RESET()           HAL_GPIO_WritePin(RS485_4_DE_RE_GPIO_Port, RS485_4_DE_RE_Pin, GPIO_PIN_RESET)
#define RS485_4_DE_SET()             HAL_GPIO_WritePin(RS485_4_DE_RE_GPIO_Port, RS485_4_DE_RE_Pin, GPIO_PIN_SET)


osSemaphoreId_t Rs485_1_lock = NULL;
osSemaphoreId_t Rs485_2_lock = NULL;
osSemaphoreId_t Rs485_3_lock = NULL;
osSemaphoreId_t Rs485_4_lock = NULL;

//-------------------------------------------
void Bsp_Rs485_1_Init(BaudRate_t config) {
    RS485_1_RE_RESET();
    Rs485_1_lock = osSemaphoreNew(1,1,NULL);
    
    uint32_t baudrate = 115200;
    uint32_t checkbits = 0;
   
    switch(config.bits.baudrate) {
        case 0:
            baudrate = 4800;
            break;
        case 1:
            baudrate = 9600;
            break;
        case 2:
            baudrate = 19200;
            break;
        case 3:
            baudrate = 38400;
            break;
        case 4:
            baudrate = 57600;
            break;
        case 5:
            baudrate = 115200;
            break;
        default:
            break;
    }
                    
    switch(config.bits.check_bits) {
        case 0:
            checkbits = UART_PARITY_NONE;
            break;
        case 1:
            checkbits = UART_PARITY_ODD;
            break;
        case 2:
            checkbits = UART_PARITY_EVEN;
            break;
        default:
            break;
    }
    MX_USART6_UART_DeInit();
    MX_USART6_UART_Init(baudrate,checkbits);
}

void Bsp_Rs485_1_SendBytes(uint8_t *pdata,uint16_t length) {
    
    osSemaphoreAcquire(Rs485_1_lock,osWaitForever);
    
    RS485_1_DE_SET();
    
    osDelay(10);
    
    while(length--) {
        while((USART6->SR & 0X40) == 0);
        USART6->DR = *pdata++;
       
    }
    
    while((USART6->SR & 0X40) == 0);
    osDelay(1);
    
    RS485_1_RE_RESET();
    
    osSemaphoreRelease(Rs485_1_lock);
}
//-------------------------------------------
void Bsp_Rs485_2_Init(BaudRate_t config) {
    RS485_2_RE_RESET();
    Rs485_2_lock = osSemaphoreNew(1,1,NULL);
    
    uint32_t baudrate = 115200;
    uint32_t checkbits = 0;
   
    switch(config.bits.baudrate) {
        case 0:
            baudrate = 4800;
            break;
        case 1:
            baudrate = 9600;
            break;
        case 2:
            baudrate = 19200;
            break;
        case 3:
            baudrate = 38400;
            break;
        case 4:
            baudrate = 57600;
            break;
        case 5:
            baudrate = 115200;
            break;
        default:
            break;
    }
                    
    switch(config.bits.check_bits) {
        case 0:
            checkbits = UART_PARITY_NONE;
            break;
        case 1:
            checkbits = UART_PARITY_ODD;
            break;
        case 2:
            checkbits = UART_PARITY_EVEN;
            break;
        default:
            break;
    }
    MX_USART3_UART_DeInit();
    MX_USART3_UART_Init(baudrate,checkbits);
}

void Bsp_Rs485_2_SendBytes(uint8_t *pdata,uint16_t length) {
    
    osSemaphoreAcquire(Rs485_2_lock,osWaitForever);
    
    RS485_2_DE_SET();
    
    osDelay(10);
    
    while(length--) {
        while((USART3->SR & 0X40) == 0);
        USART3->DR = *pdata++;
       
    }
    
    while((USART3->SR & 0X40) == 0);
    osDelay(1);
    
    RS485_2_RE_RESET();
    
    osSemaphoreRelease(Rs485_2_lock);
}
//-------------------------------------------
void Bsp_Rs485_3_Init(BaudRate_t config) {
    RS485_3_RE_RESET();
    Rs485_3_lock = osSemaphoreNew(1,1,NULL);
    
    uint32_t baudrate = 115200;
    uint32_t checkbits = 0;
   
    switch(config.bits.baudrate) {
        case 0:
            baudrate = 4800;
            break;
        case 1:
            baudrate = 9600;
            break;
        case 2:
            baudrate = 19200;
            break;
        case 3:
            baudrate = 38400;
            break;
        case 4:
            baudrate = 57600;
            break;
        case 5:
            baudrate = 115200;
            break;
        default:
            break;
    }
                    
    switch(config.bits.check_bits) {
        case 0:
            checkbits = UART_PARITY_NONE;
            break;
        case 1:
            checkbits = UART_PARITY_ODD;
            break;
        case 2:
            checkbits = UART_PARITY_EVEN;
            break;
        default:
            break;
    }
    MX_USART2_UART_DeInit();
    MX_USART2_UART_Init(baudrate,checkbits);
}

void Bsp_Rs485_3_SendBytes(uint8_t *pdata,uint16_t length) {
    
    osSemaphoreAcquire(Rs485_3_lock,osWaitForever);
    
    RS485_3_DE_SET();
    
    osDelay(10);
    
    while(length--) {
        while((USART2->SR & 0X40) == 0);
        USART2->DR = *pdata++;
       
    }
    
    while((USART2->SR & 0X40) == 0);
    osDelay(1);
    
    RS485_3_RE_RESET();
    
    osSemaphoreRelease(Rs485_3_lock);
}

//------------------------------------------------------
void Bsp_Rs485_4_Init(BaudRate_t config) {
    RS485_4_RE_RESET();
    Rs485_4_lock = osSemaphoreNew(1,1,NULL);
    
    uint32_t baudrate = 115200;
    uint32_t checkbits = 0;
   
    switch(config.bits.baudrate) {
        case 0:
            baudrate = 4800;
            break;
        case 1:
            baudrate = 9600;
            break;
        case 2:
            baudrate = 19200;
            break;
        case 3:
            baudrate = 38400;
            break;
        case 4:
            baudrate = 57600;
            break;
        case 5:
            baudrate = 115200;
            break;
        default:
            break;
    }
                    
    switch(config.bits.check_bits) {
        case 0:
            checkbits = UART_PARITY_NONE;
            break;
        case 1:
            checkbits = UART_PARITY_ODD;
            break;
        case 2:
            checkbits = UART_PARITY_EVEN;
            break;
        default:
            break;
    }
    MX_USART1_UART_DeInit();
    MX_USART1_UART_Init(baudrate,checkbits);
}

void Bsp_Rs485_4_SendBytes(uint8_t *pdata,uint16_t length) {
    
    osSemaphoreAcquire(Rs485_4_lock,osWaitForever);
    
    RS485_4_DE_SET();
    
    osDelay(10);
    
    while(length--) {
        while((USART1->SR & 0X40) == 0);
        USART1->DR = *pdata++;
       
    }
    
    while((USART1->SR & 0X40) == 0);
    osDelay(1);
    
    RS485_4_RE_RESET();
    
    osSemaphoreRelease(Rs485_4_lock);
}


