#ifndef __LOG_RECODER_H__
#define __LOG_RECODER_H__

#ifdef __cplusplus
 extern "C" {
#endif
#include <stdbool.h>     
#include <stdint.h>
#include <string.h>
#include "typedef.h"
#include "bsp_flash.h"


/* logger type */
typedef enum {
    LOG_TYPE_None = 0,
	LOG_TYPE_RESET,
	LOG_TYPE_OPERATE,
	LOG_TYPE_EXCEPTION,
	LOG_TYPE_MAX,
} LOG_TYPE_t;


#define LOGGER_WITH_FS   0

     
#include <stdint.h>

typedef enum {
    LOGGER_RESET = 0,
    LOGGER_CONFIG,
    LOGGER_OPERATE,
    LOGGER_WARNING,
    LOGGER_EXCEPT,
    LOGGER_INFOR,
    LOGGER_OTHERS,
    LOGGER_TYPE_MAX,
}LOGGER_TYPE_t;

typedef enum {
    LOGGER_MORE_NONE = 0,
    LOGGER_MORE_FS_FORMAT,
    LOGGER_MORE_FILE_CREATE,
    LOGGER_MORE_FILE_WRITE_OPEN_FAILED,
    LOGGER_MORE_FILE_OPERATE_FAILED,
    LOGGER_MORE_SYNCH_PARA_I03T,
    LOGGER_MORE_SYNCH_SN_I03T,
    LOGGER_MORE_ADD_I03T,
    LOGGER_MORE_DEL_I03T,
    LOGGER_MORE_ADD_SN,
    LOGGER_MORE_DEL_ALL_SN,
    LOGGER_MORE_DEL_HIST,
    LOGGER_MORE_DEL_ALARM,
    LOGGER_MORE_DEL_DISCHARGE,
    LOGGER_MORE_UPGRADE,
    LOGGER_MORE_SOC_INIT,
    LOGGER_MORE_MQTT,
    LOGGER_MORE_CONFIGTIME,
}LOGGER_MORE_INFOR_t;
#if 0
typedef struct {
    uint32_t      type;         
    uint32_t      more;         
    uint8_t       file[32];    
    uint32_t      line;         
    union {                     
        uint8_t       d1[40];   
        uint16_t      d2[20];
        uint32_t      d4[10];   
    }v;
}logger_msg_t;
#endif

typedef struct {
    uint32_t      type;         
    uint32_t      more;         
    uint8_t       file[32];    
    uint32_t      line;         
    union {                     
        uint8_t       d1[72];   
        uint16_t      d2[36];
        uint32_t      d4[18];   
    }v;
}logger_msg_t;

typedef struct {
    uint32_t                 sum;    
    DATE_yymmddhhmmss_t      time;   
    uint32_t                 index; 
    logger_msg_t             msg;    
}logger_t;

extern const char* logger_type[];

#define LOGGER_MAX_SIZE        (32 * 1024u)
#define LOGGER_START_ADDR       ADDR_LOGGER_START
#define LOGGER_END_ADDR        (LOGGER_START_ADDR +  LOGGER_MAX_SIZE)


#define LOGGER_SECTOR_SIZE        (16 * 1024u)
#define INDEX_LOGGER_OF_SECTOR    (LOGGER_SECTOR_SIZE/sizeof(logger_t))
#define INDEX_LOGGER_MAX_NUMBER   (LOGGER_MAX_SIZE / sizeof(logger_t))
    
void logger_infor_load(COMM_TYPE_t commType,bool load);
void logger_infor_clear(void);
void logger_infor_save(logger_msg_t msg);
void logger_infor_save_file(LOGGER_TYPE_t type,LOGGER_MORE_INFOR_t more,uint32_t infor,const char *str,const char *file,uint16_t line);
void logger_infor_save_more(LOGGER_TYPE_t type,LOGGER_MORE_INFOR_t more,uint32_t infor,const char *str,const char *file,uint16_t line);
void logger_set_reset_reg(void);
void logger_get_reboot(uint32_t reg,char *buf);
void rtc_get_time(DATE_yymmddhhmmss_t *time);
#ifdef __cplusplus
}
#endif

#endif
