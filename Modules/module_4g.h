#ifndef __WIRELESS_MESSAGE_H__
#define __WIRELESS_MESSAGE_H__

#include <stdint.h>
#include <stdbool.h>
#include "typedef.h"

#include "module_tas.h"
#include "module_zlg.h"


typedef errStatus_t (*pAtHandle)(uint8_t);

#define MODULE4G_MAX_SOCKET   5

typedef struct {
    uint8_t   at_code;
    uint8_t   at_next_code;
    uint16_t  time_out;
    pAtHandle handle;
    uint8_t   times_sec;
    uint8_t   para;
}AT_COMMAND_t;

#define MODULE_4G_MAX_TX_SIZE 2048
typedef struct {
    uint8_t  data[MODULE_4G_MAX_TX_SIZE];
    uint16_t length;
}AT_TX_t;

#define MODULE_4G_MAX_RX_SIZE 2048
typedef struct {
    uint32_t tick;
    uint8_t  data[MODULE_4G_MAX_RX_SIZE];
    uint16_t length;
}AT_RX_t;

typedef struct {
    uint16_t length;
    uint8_t *pbuf;
}AT_RX_BUF_t;

typedef enum
{
    SIM_FAILED         = 0,
    SIM_CHINA_MOBILE ,
    SIM_CHINA_UNICOM ,  
    SIM_CHINA_TELECOM ,
    SIM_UNKNOW,
    
}SimType_t;

typedef enum 
{
    NET_UNKNOW         = 0,
    NET_2G             = 2,
    NET_3G             ,
    NET_4G             ,
}NetType_t;



typedef struct {
    uint8_t  at_code;
    uint8_t  rssi;
    uint8_t  ber;
    uint8_t  socket;
    int16_t  snr;
    uint8_t  connect; //
    bool     attach;
    bool     cgatt;
    bool     get_ip;
    bool     cpsms;
    bool     cereg4;
	bool     act_cgatt;
	bool     act_ppp;
	
	NetType_t  net_type;
	SimType_t  sim_type;
	char       module_infor[32];
	
    uint8_t  cid;
    uint8_t  imei[16];
    uint8_t  iccid[21];
    uint8_t  imsi[16];
	char     apn[10];
    
    uint16_t pid;  //package id.
    uint16_t pending;//pending for at ack.
    uint8_t  err_count;
	bool     is_sending;
    
}Module4GInfor_t;




typedef char* (*pRxCommandHandle)(char *,uint16_t);

typedef struct {
    char *command;
    pRxCommandHandle rx_handle;
}RX_CMD_Handle_t;

/* nbiot msg time out.*/
#define WIRELESS_QUEUE_TX_TIME_OUT        5000u 

#define AT_OK_STR "OK\r\n"
#define AT_FRAME_TIME     100
#define AT_BUF_MAX_SIZE   36

extern AT_RX_t * const pModule4GRx;
extern AT_TX_t * const pModule4GTx;
extern Module4GInfor_t * pModule4GInfor;

//extern power_on module_4g_power_on;
//extern power_off module_4g_power_off;


void Module4G_Init(void);
bool isExistG4thModule(void);
errStatus_t Module4G_CSQ(void);
errStatus_t module_4g_wait_response(const char *send_bytes,
                            uint16_t send_length,
                            char *expect_ack,
                            char *response,
                            uint16_t response_length,
                            uint16_t time_out);

#endif

