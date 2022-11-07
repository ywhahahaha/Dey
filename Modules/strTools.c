#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strTools.h>

void remove_spc_chars(uint8_t *source,uint8_t *dest) {
    char ch;
    while((ch = *source++) != '\0') {
        if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            continue;
        }
        
        *dest++ = ch;
    }
    
    *dest = 0;
}

void remove_chars(char *source, char *dest, char ch)
{
    uint8_t i = 0;
    while(*source != '\0') {
        if(*source != ch) {
            *dest = *source;
            dest++;
            i++;
        }
        source++;
    }
}

//0x34 -->34
void STR_Hex2Str(char *src, int length, char *dest)
{
    uint8_t str_H, str_L;
    for (int i = 0; i < length; i++) {
        str_H = '0' + (*src >> 4);
        str_L = '0' + (*src & 0x0f);
        if (str_H > '9') {
            str_H += 7;
        }
        if (str_L > '9') {
            str_L += 7;
        }
        *dest++ = str_H;
        *dest++ = str_L;
        src++;
    }
    *dest++ = '\0';
}

//length : str length
void STR_Str2Hex(char *src, int length, char *dest)
{
    bool hbit = false;
    uint8_t h, l;
    for (int i = 0; i < length; i++) {
        if (src[i] == ' ') {
            continue;
        }
        if (hbit == false) {
            h = src[i] - 0x30;
            if (h > 9) {
                h -= 7;
            }
            hbit = true;
            continue;
        } else {
            l = src[i] - 0x30;
            if (l > 9) {
                l -= 7;
            }
        }
        *dest++ = h << 4 | l;
        hbit = false;
    }
}

uint8_t BCD2HEX(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0f);
}

uint8_t HEX2BCD(uint8_t hex)
{
    return ((hex / 10) << 4) + (hex % 10);
}

uint32_t str2int(char *str)
{
    uint32_t result = 0;
    char ch;
    while ((ch = *str++) != '\0') {
        if (ch >= '0' && ch <= '9') {
            result = 10 * result + ch - 0x30;
        } else {
            break;
        }
    }
    return result;
}

void getf_fi(float f,int *fi,int *ff) {
	int f_int,f_float;

	f_int = f;

	f_float = (f-f_int) * 100;

	*fi = f_int;
	*ff = (int)f_float;
}

