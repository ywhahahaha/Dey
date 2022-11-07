#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <strTools.h>
#include "main.h"
#include "stm32f4xx.h"
#include "module_4g.h"
#include "module_eth.h"
#include "bsp_gpio.h"

#define RESPONSE_MAX_LENGTH  100

void AT_SendBytes(uint8_t *sendbytes,uint16_t length);

errStatus_t module_4g_wait_response(const char *send_bytes,
                            uint16_t send_length,
                            char *expect_ack,
                            char *response,
                            uint16_t response_length,
                            uint16_t time_out);


static const AT_COMMAND_t AtCmd[];

#define RESPONSE_MAX_LENGTH  100


errStatus_t ModuleETH_CLOSE_COM1(uint8_t para)
{
	char *send_bytes = "AT+SOCKET=1,255,1023,1883,47.108.193.148\r\n";
    uint16_t send_length = strlen(send_bytes);

    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_CLOSE_COM1].time_out);
}

errStatus_t ModuleETH_SERBAUD(uint8_t para)
{

	char *send_bytes = "AT+UARTCFG=2,115200,1,0,0\r\n";
    uint16_t send_length = strlen(send_bytes);
    errStatus_t ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_SERBAUD].time_out);    
    return ret;
}
void module_4g_port_cfg(BaudRate_t baudrate);
errStatus_t ModuleETH_EnterConfig(uint8_t para) {
    
    errStatus_t ret;
    
	const char *send_bytes = "+++";
    uint16_t send_length = strlen(send_bytes);

    ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_ENTER_CONFIG].time_out);
    
    if (ret == errOK) {
        return ret;
    }
    BaudRate_t baudrate = {0}; 
    baudrate.bits.baudrate = BAUDRATE_9600;
    module_4g_port_cfg(baudrate);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_ENTER_CONFIG].time_out);
    
}


errStatus_t ModuleETH_SET_SOCKE_MODE(uint8_t para) {

	char send_bytes[128] = {0};
    int a,b,c,d;
    uint16_t send_length;
    errStatus_t ret;
        
    int result = sscanf((const char *)pDEApp->device_config.i03m.mqtt_config.mqtt_addr,"%d.%d.%d.%d",&a,&b,&c,&d);
        
    if(result == 4) {
        sprintf(send_bytes,"AT+SOCKET=2,8,1025,1883,%s\r\n",pDEApp->device_config.i03m.mqtt_config.mqtt_addr);
        send_length = strlen(send_bytes);
        ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_SOCKE_MODE].time_out);
    } else {
        sprintf(send_bytes,"AT+SOCKET=2,8,1025,1883,%s\r\n",pDEApp->device_config.i03m.mqtt_config.mqtt_addr);
        send_length = strlen(send_bytes);
        ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_SOCKE_MODE].time_out);
    }
          
    return ret;
    
}

errStatus_t ModuleETH_CLIENTID(uint8_t para) {

	char send_bytes[64] = {0};
    sprintf(send_bytes,"AT+MQTTCLIENTID=2,\"%d\"\r\n",rand());
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_CLIENTID].time_out);
}

errStatus_t ModuleETH_USERPWD(uint8_t para) {

	char send_bytes[128] = {0};
      
    sprintf(send_bytes,"AT+USERPWD=2,\"%s\",\"123456\"\r\n",pDEApp->device_config.i03m.mqtt_config.user_name);
    uint16_t send_length = strlen(send_bytes);
    errStatus_t ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_USERPWD].time_out);
    return ret;
    
}


errStatus_t ModuleETH_AUTOPUB(uint8_t para) {

	char send_bytes[128] = {0};
    sprintf(send_bytes,"AT+MQTTPUB=2,1,1,\"%s\",0\r\n",MQTT_PUB_TOPIC_1);
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_AUTOPUB].time_out);
    
}

errStatus_t ModuleETH_AUTOSUB(uint8_t para) {

	char send_bytes[128] = {0};
    sprintf(send_bytes,"AT+MQTTSUB=2,1,1,\"%s\",0\r\n",MQTT_SUB_TOPIC_1);
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_AUTOSUB].time_out);
    
}

errStatus_t ModuleETH_SET_IPCONFIG(uint8_t para) {

	char send_bytes[128] = {0};
    
    sprintf(send_bytes,"AT+IPCONFIG=0,%s,%s,%s,%s\r\n",pDEApp->device_config.i03m.net_config.ipv4.ip,\
            pDEApp->device_config.i03m.net_config.ipv4.gate,\
            pDEApp->device_config.i03m.net_config.ipv4.mask,\
            pDEApp->device_config.i03m.net_config.dns);
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_SET_IPCONFIG].time_out);
    
}

errStatus_t ModuleETH_MQTTKEEP(uint8_t para) {

    return errOK;
    /*
	const char *send_bytes = "AT+MQTTKEEP=60,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_MQTTKEEP].time_out);
    */
}



errStatus_t ModuleETH_CLEANSESSION(uint8_t para) {
    return errOK;
/*
	const char *send_bytes = "AT+CLEANSESSION=1,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_CLEANSESSION].time_out);
   */ 
}


errStatus_t ModuleETH_SET_ACKTIME(uint8_t para) {

	const char *send_bytes = "AT+ACKTIME=2,0\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_SET_ACKTIME].time_out);
    
}

errStatus_t ModuleETH_SET_PORTTIME(uint8_t para) {

	const char *send_bytes = "AT+PORTTIME=2,0\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[ AT_ETH_SET_PORTTIME].time_out);
    
}

errStatus_t ModuleETH_SET_RESTTIME(uint8_t para) {

	const char *send_bytes = "AT+RESTTIME=0\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_SET_RESTTIME].time_out);
    
}

errStatus_t ModuleETH_SET_SAVE(uint8_t para) {

	const char *send_bytes = "AT&W\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_SET_SAVE].time_out);
    
}

errStatus_t ModuleETH_SET_REBOT(uint8_t para) {

	const char *send_bytes = "AT+CFUN=1,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ETH_SET_REBOT].time_out);
    
}

char *module_payload_make_i03t(uint8_t i03t_addr);
errStatus_t appl_cloud_send_by_eth(void) {
    errStatus_t ret;
    
    
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_I03T; i03t_addr++) {
        char *json = (char *)module_payload_make_i03t(i03t_addr);
        if(json != NULL) {
            if(pDEApp->Flag.bits.g4_trace) {
                debug_printf("I03T[%d]:===>%s\r\n",i03t_addr,json);
            }
            AT_SendBytes((uint8_t *)json,strlen(json));            
            osDelay(5000);
            sys_free(json);
        }
    } 
    ret = errOK;
    return ret;
}



errStatus_t ModuleETH_TIME(uint8_t para)
{
    errStatus_t ret;
    char response[RESPONSE_MAX_LENGTH] = {0};
    char *send_bytes = "AT+TIME?\r\n";
    uint16_t send_length = strlen(send_bytes);

    ret = module_4g_wait_response(send_bytes, send_length,"+TIME: ", response, RESPONSE_MAX_LENGTH,ModuleETH_AT_TIMEOUT);
    if(ret == errOK) {
        uint32_t unix_sec;
        char *p = strstr(response,"+TIME: ");
        if(p != NULL) {
            if(1 == sscanf((char *)p,"+TIME: %d\r\n",&unix_sec)) {
                #include <time.h>
                DATE_yymmddhhmmss_t now;

                unix_sec += 8 * 3600;
                struct tm *time1 = localtime(&unix_sec);
                struct tm time2;
                uint32_t sec1;

                //Bsp_RtcGetTime(&now);

                time2.tm_year = (uint32_t)pDEApp->now.bits.year + 2000 - 1900;
                time2.tm_mon = pDEApp->now.bits.month - 1;
                time2.tm_mday = pDEApp->now.bits.day;
                time2.tm_hour = pDEApp->now.bits.hour;
                time2.tm_min = pDEApp->now.bits.min;
                time2.tm_sec = pDEApp->now.bits.sec;
                time2.tm_isdst = 0;
                sec1 = mktime(&time2);

                int32_t time_diff = (int32_t)(sec1 - unix_sec);
                if(abs(time_diff) > 600) {
                    
                    now.bits.year = time1->tm_year - 100;
                    now.bits.month = time1->tm_mon + 1;
                    now.bits.day = time1->tm_mday;
                    now.bits.hour = time1->tm_hour;
                    now.bits.min = time1->tm_min;
                    now.bits.sec = time1->tm_sec;
                    
                    Bsp_RtcSetTime(&now);
                    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,now.date,NULL,__FILE__,__LINE__);
                    
                }
            }
        }
 
        return errOK;
    }
    return ret;
}


errStatus_t AppMqttSubscribeTopics4G(void);

errStatus_t ModuleETH_Register(void)
{
	errStatus_t ret = errErr;
    uint32_t last_tick = osKernelGetTickCount();
	
    memset((void *)pModule4GInfor, 0, sizeof(Module4GInfor_t));

    pModule4GInfor->at_code = AT_ETH_ENTER_CONFIG;
    pModule4GInfor->act_ppp = false;
    pModule4GInfor->pending = true;
	pModule4GInfor->attach = false;
	pModule4GInfor->connect = 0;
    pModule4GInfor->socket = 0;
	pModule4GInfor->iccid[0] = 0x00;

    int32_t diff_time;
    while(1) {
        if(AtCmd[pModule4GInfor->at_code].handle != NULL) {
            errStatus_t err = AtCmd[pModule4GInfor->at_code].handle(AtCmd[pModule4GInfor->at_code].para);
            if(err == errOK) {
                last_tick = osKernelGetTickCount();
                pModule4GInfor->at_code = AtCmd[pModule4GInfor->at_code].at_next_code;
                if(pModule4GInfor->at_code == AT_ETH_NONE) {
                    pModule4GInfor->pending = false;
                    if(pModule4GInfor->connect) {
                        ret = errOK;
						break;
                    }
                    ret = errErr;
					break;
                }
                continue;
            }
            
            diff_time = osKernelGetTickCount() - last_tick;
            
            if(abs(diff_time) >= AtCmd[pModule4GInfor->at_code].times_sec * 1000ul) {

                last_tick = osKernelGetTickCount();
                
                pModule4GInfor->at_code = AtCmd[pModule4GInfor->at_code].at_next_code;
                if(pModule4GInfor->at_code == AT_ETH_NONE) {
					pModule4GInfor->pending = false;
                    if(pModule4GInfor->connect) {
                        ret = errOK;
						break;
                    }
                    ret = errErr;
					break;
                }
            }
        }
        osDelay(100);
    }
    
    BaudRate_t baudrate = {0}; 
    baudrate.bits.baudrate = BAUDRATE_115200;
    module_4g_port_cfg(baudrate);

	return ret;

}


static const AT_COMMAND_t AtCmd[] = {
    { AT_ETH_ENTER_CONFIG,        AT_ETH_CLOSE_COM1,       ModuleETH_AT_TIMEOUT,      ModuleETH_EnterConfig,    20,     0},
    { AT_ETH_CLOSE_COM1,          AT_ETH_SOCKE_MODE,       ModuleETH_AT_TIMEOUT,      ModuleETH_CLOSE_COM1,      1,     0},    
    { AT_ETH_SOCKE_MODE,          AT_ETH_CLIENTID,         ModuleETH_AT_TIMEOUT,      ModuleETH_SET_SOCKE_MODE,  10,    0},
    { AT_ETH_CLIENTID,            AT_ETH_USERPWD,          ModuleETH_AT_TIMEOUT,      ModuleETH_CLIENTID,        3,     0},
    { AT_ETH_USERPWD,             AT_ETH_AUTOPUB,          ModuleETH_AT_TIMEOUT,      ModuleETH_USERPWD,         3,     0},
    { AT_ETH_AUTOPUB,             AT_ETH_MQTTKEEP,         ModuleETH_AT_TIMEOUT,      ModuleETH_AUTOPUB,         3,     0},
    { AT_ETH_MQTTKEEP,            AT_ETH_CLEANSESSION,     ModuleETH_AT_TIMEOUT,      ModuleETH_MQTTKEEP,        3,     0},
    { AT_ETH_CLEANSESSION,        AT_ETH_SET_IPCONFIG,     ModuleETH_AT_TIMEOUT,      ModuleETH_CLEANSESSION,    3,     0},  
    { AT_ETH_SET_IPCONFIG,        AT_ETH_SET_ACKTIME,      ModuleETH_AT_TIMEOUT,      ModuleETH_SET_IPCONFIG,    3,     0},
    { AT_ETH_SET_ACKTIME,         AT_ETH_SET_PORTTIME,     ModuleETH_AT_TIMEOUT,      ModuleETH_SET_ACKTIME,     3,     0},    
    { AT_ETH_SET_PORTTIME,        AT_ETH_SET_RESTTIME,     ModuleETH_AT_TIMEOUT,      ModuleETH_SET_PORTTIME,    3,     0},
    { AT_ETH_SET_RESTTIME,        AT_ETH_AUTOSUB,          ModuleETH_AT_TIMEOUT,      ModuleETH_SET_RESTTIME,    3,     0},
    { AT_ETH_AUTOSUB,             AT_ETH_SERBAUD,          ModuleETH_AT_TIMEOUT,      ModuleETH_AUTOSUB,         3,     0},
    { AT_ETH_SERBAUD,             AT_ETH_SET_SAVE,         ModuleETH_AT_TIMEOUT,      ModuleETH_SERBAUD,         3,     0},
    { AT_ETH_SET_SAVE,            AT_ETH_SET_REBOT,        ModuleETH_AT_TIMEOUT,      ModuleETH_SET_SAVE,        3,     0},
    { AT_ETH_SET_REBOT,           AT_ETH_NONE,             ModuleETH_AT_TIMEOUT,      ModuleETH_SET_REBOT,       3,     0},
};


void ModuleETH_RxMsgHandle(uint8_t *pMsg,uint16_t length) {
        
   char  *ucpStatus;  
   ucpStatus = strstr((char *)pMsg,"COM2 DISCONNECT");
   if (ucpStatus != NULL) {
       pModule4GInfor->connect = 0;       
   }
   
   ucpStatus = strstr((char *)pMsg,"COM2 CONNECT OK");
   if (ucpStatus != NULL) {
       pModule4GInfor->connect = 1;       
   }
   
}










































