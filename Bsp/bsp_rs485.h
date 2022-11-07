#ifndef __BSP_RS485_H_
#define __BSP_RS485_H_

#include <stdint.h>

#include "typedef.h"

void Bsp_Rs485_1_Init(BaudRate_t config);
void Bsp_Rs485_2_Init(BaudRate_t config);
void Bsp_Rs485_3_Init(BaudRate_t config);
void Bsp_Rs485_4_Init(BaudRate_t config);


void Bsp_Rs485_1_SendBytes(uint8_t *pdata,uint16_t length);
void Bsp_Rs485_2_SendBytes(uint8_t *pdata,uint16_t length);
void Bsp_Rs485_3_SendBytes(uint8_t *pdata,uint16_t length);
void Bsp_Rs485_4_SendBytes(uint8_t *pdata,uint16_t length);

#endif
