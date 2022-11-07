#ifndef __STRTOOLS_H_
#define __STRTOOLS_H_

#include <stdint.h>

void STR_Hex2Str(char *src, int length, char *dest);
void STR_Str2Hex(char *src, int length, char *dest);
uint8_t BCD2HEX(uint8_t bcd);
uint8_t HEX2BCD(uint8_t hex);
uint32_t str2int(char *str);
void remove_spc_chars(uint8_t *source,uint8_t *dest);
void remove_chars(char *source, char *dest, char ch);
void getf_fi(float f,int *fi,int *ff);
#endif
