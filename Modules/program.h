#ifndef HARDWARE_PROGRAM_DRIVER_H_
#define HARDWARE_PROGRAM_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "string.h"

#ifdef __cplusplus    
extern "C" {          
#endif

#define WORD unsigned short 
#define BYTE unsigned char 
WORD CRC16_lookup(BYTE *pucFrame,int usLen);
WORD CheckCRCModBus(const BYTE* pDataIn, int iLenIn);
char *my_iota(int var, int radix);
int str_to_hex(char *string, unsigned char *cbuf, int len);
void hex_to_str(char *ptr,unsigned char *buf,int len);
void bcd2str_float(uint8_t *bcd,char *ret, uint32_t length,int id);
void bcd2str_int(uint8_t *bcd,char *ret, uint32_t length);
//WORD CheckCRCModBus2(const BYTE* pDataIn, int iLenIn);
WORD CheckCRCModBus2(BYTE pDataIn[], int iLenIn);
uint32_t sum_check(const uint8_t *ptr,uint32_t len);
char* itoa(int num,char* str,int radix);


#ifdef __cplusplus    
}         
#endif

#endif
