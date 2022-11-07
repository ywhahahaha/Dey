#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <strTools.h>
#include "main.h"
#include "stm32f4xx.h"
#include "module_4g.h"
#include "module_zlg.h"
#include "bsp_gpio.h"


#define RESPONSE_MAX_LENGTH  100


void AT_SendBytes(uint8_t *sendbytes,uint16_t length);

errStatus_t module_4g_wait_response(const char *send_bytes,
                            uint16_t send_length,
                            char *expect_ack,
                            char *response,
                            uint16_t response_length,
                            uint16_t time_out);

void ModuleZLG_PowerON(void) {
	ModuleZLG_POWERKEY_L();
	osDelay(2000);
	ModuleZLG_POWERKEY_H();
}

void ModuleZLG_Init(void) {
	ModuleZLG_PowerON();
}


void ModuleZLG_Reset(void) {
    ModuleZLG_RESET_H();
    osDelay(100);
    ModuleZLG_RESET_L();
    osDelay(4000);
    ModuleZLG_RESET_H();
}





static const AT_COMMAND_t AtCmd[];

#define RESPONSE_MAX_LENGTH  100


errStatus_t ModuleZLG_SERS(uint8_t para)
{
	char *send_bytes = "AT+SERS=100\r\n";
    uint16_t send_length = strlen(send_bytes);

    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_SERS].time_out);
}

errStatus_t ModuleZLG_SERBAUD(uint8_t para)
{

	char *send_bytes = "AT+SERBAUD=115200\r\n";
    uint16_t send_length = strlen(send_bytes);
    errStatus_t ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_SERBAUD].time_out);
    
    send_bytes = "AT+SERBAUD2=115200\r\n";
    send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_SERBAUD].time_out);
    
}

errStatus_t ModuleZLG_EnterConfig(uint8_t para) {

	const char *send_bytes = "+++ in set\r\n";
    uint16_t send_length = strlen(send_bytes);

    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_ENTER_CONFIG].time_out);
    
}

errStatus_t ModuleZLG_ExitConfig(uint8_t para) {

	const char *send_bytes = "AT+OUTSET=ON\r\n";
    uint16_t send_length = strlen(send_bytes);

    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_ENTER_CONFIG].time_out);
    
}



errStatus_t ModuleZLG_IPPORT(uint8_t para) {

	char send_bytes[128] = {0};
    int a,b,c,d;
    uint16_t send_length;
    errStatus_t ret;
    int result = sscanf((const char *)pDEApp->device_config.i03m.mqtt_config.mqtt_addr,"%d.%d.%d.%d",&a,&b,&c,&d);
    if(result == 4) {
        sprintf(send_bytes,"AT+SVRIP1=%s\r\n",pDEApp->device_config.i03m.mqtt_config.mqtt_addr);
        send_length = strlen(send_bytes);
        ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_IPPORT].time_out);
        if(ret == errOK) {
            const char *_bytes = "AT+SVRNAM1=\r\n";
            send_length = strlen(_bytes);
            ret = module_4g_wait_response(_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_IPPORT].time_out);
        }
    } else {
        sprintf(send_bytes,"AT+SVRNAM1=%s\r\n",pDEApp->device_config.i03m.mqtt_config.mqtt_addr);
        send_length = strlen(send_bytes);
        ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_IPPORT].time_out);
        if(ret == errOK) {
            const char *_bytes = "AT+SVRIP1=\r\n";
            send_length = strlen(_bytes);
            ret = module_4g_wait_response(_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_IPPORT].time_out);
        }
    }
    
    
    
    if(ret == errOK) {
        memset(send_bytes,0,sizeof(send_bytes));
        sprintf(send_bytes,"AT+SVRPORT1=%d\r\n",pDEApp->device_config.i03m.mqtt_config.mqtt_port);
        send_length = strlen(send_bytes);
        ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_IPPORT].time_out);
    }
    
    
    return ret;
    
}

errStatus_t ModuleZLG_CLIENTID(uint8_t para) {

	char send_bytes[64] = {0};
    sprintf(send_bytes,"AT+CLIENTID=%d%d\r\n",rand(),rand());
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_CLIENTID].time_out);
}

errStatus_t ModuleZLG_USERPWD(uint8_t para) {

	char send_bytes[128] = {0};
    sprintf(send_bytes,"AT+MQTTUSR=%s\r\n",pDEApp->device_config.i03m.mqtt_config.user_name);
    uint16_t send_length = strlen(send_bytes);
    errStatus_t ret = module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_USERPWD].time_out);
    return ret;
    

}


errStatus_t ModuleZLG_AUTOPUB(uint8_t para) {

	char send_bytes[128] = {0};
    sprintf(send_bytes,"AT+PUBTOPIC=%s\r\n",MQTT_PUB_TOPIC_1);
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_AUTOPUB].time_out);
    
}

errStatus_t ModuleZLG_MQTTKEEP(uint8_t para) {

    return errOK;
    /*
	const char *send_bytes = "AT+MQTTKEEP=60,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_MQTTKEEP].time_out);
    */
}



errStatus_t ModuleZLG_CLEANSESSION(uint8_t para) {
    return errOK;
/*
	const char *send_bytes = "AT+CLEANSESSION=1,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_CLEANSESSION].time_out);
   */ 
}

errStatus_t ModuleZLG_CACHE(uint8_t para) {
    return errOK;
/*
	const char *send_bytes = "AT+CACHE=1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_CACHE].time_out);
*/    
}


errStatus_t ModuleZLG_MODE(uint8_t para) {

	const char *send_bytes = "AT+MODE=ONLIN\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_MODE].time_out);
    
}

errStatus_t ModuleZLG_TRNMODE(uint8_t para) {

	const char *send_bytes = "AT+TRNMODE=MQTT\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_ZLG_TRNMODE].time_out);
    
}




errStatus_t ModuleZLG_GETDTUSTATE(uint8_t para) {

	const char *send_bytes = "AT+GETDTUSTATE\r\n";
    uint16_t send_length = strlen(send_bytes);
    char response[RESPONSE_MAX_LENGTH] = {0};
    
    errStatus_t ret;
    
    ret = module_4g_wait_response(send_bytes,send_length,"DTU CONNECT OK",response,RESPONSE_MAX_LENGTH, ModuleZLG_AT_TIMEOUT);

    if(ret == errOK) {
        pModule4GInfor->connect = 1;
    } 
    return ret;
}


//+CSQ:21,99
errStatus_t ModuleZLG_CSQ(uint8_t para) {

	const char *send_bytes = "AT+CSQ?\r\n";
    uint16_t send_length = strlen(send_bytes);
    char response[RESPONSE_MAX_LENGTH] = {0};
    
    errStatus_t ret;
    
    ret = module_4g_wait_response(send_bytes,send_length,"+CSQ:",response,RESPONSE_MAX_LENGTH, ModuleZLG_AT_TIMEOUT);

    if(ret == errOK) {
        char *pIndex = strstr(response, "+CSQ:");
        if(pIndex != NULL) {
            pIndex += 5;
            pModule4GInfor->rssi = atoi(pIndex);
            if(pModule4GInfor->rssi == 99 || pModule4GInfor->rssi < 2) {
                return errErr;
            } 
        }
    }
    return ret;
}




/*
AT+CIMI
+CIMI: 460020188385503
OK

[46000] MCC:460 MNC:00 China China Mobile
[46002] MCC:460 MNC:02 China China Mobile
[46004] ---------------------------------
[46007] MCC:460 MNC:07 China China Mobile

[46003] MCC:460 MNC:03 China China Telecom
[46005] MCC:460 MNC:05 China China Telecom
[46011] MCC:460 MNC:11 China China Telecom
[46012] ----------------------------------


[46001] MCC:460 MNC:01 China China Unicom
[46006] MCC:460 MNC:06 China China Unicom
[46009] MCC:460 MNC:09 China China Unicom
[46010] MCC:460 MNC:09 China China Unicom

[46020] MCC:460 MNC:20 China China Tietong

*/
/*
//china mobile "CMNET"
//china uniom  "3GNET" //3gnet[wonet,uninet]
//china tele   "CTLTE"  
*/




/*
+TIME: 1650592701
OK
*/
errStatus_t ModuleZLG_TIME(uint8_t para)
{
    errStatus_t ret;
    char response[RESPONSE_MAX_LENGTH] = {0};
    char *send_bytes = "AT+TIME?\r\n";
    uint16_t send_length = strlen(send_bytes);

    ret = module_4g_wait_response(send_bytes, send_length,"+TIME: ", response, RESPONSE_MAX_LENGTH,ModuleZLG_AT_TIMEOUT);
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

errStatus_t ModuleZLG_Register(void)
{
	errStatus_t ret = errErr;
    uint32_t last_tick = osKernelGetTickCount();
	
	ModuleZLG_Reset();
	
	osDelay(15000);

    memset((void *)pModule4GInfor, 0, sizeof(Module4GInfor_t));

    pModule4GInfor->at_code = AT_ZLG_ENTER_CONFIG;
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
                if(pModule4GInfor->at_code == AT_ZLG_NONE) {
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
                if(pModule4GInfor->at_code == AT_ZLG_NONE) {
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
	
	return ret;

}



static const AT_COMMAND_t AtCmd[] = {
    { AT_ZLG_ENTER_CONFIG,  AT_ZLG_SERS,           ModuleZLG_AT_TIMEOUT,      ModuleZLG_EnterConfig,    20,     0},
    { AT_ZLG_SERS,          AT_ZLG_SERBAUD,        ModuleZLG_AT_TIMEOUT,      ModuleZLG_SERS,           1,      0},
    { AT_ZLG_SERBAUD,       AT_ZLG_IPPORT,         ModuleZLG_AT_TIMEOUT,      ModuleZLG_SERBAUD,        3,      0}, 
    { AT_ZLG_IPPORT,        AT_ZLG_CLIENTID,       ModuleZLG_AT_TIMEOUT,      ModuleZLG_IPPORT,         10,      0},
    { AT_ZLG_CLIENTID,      AT_ZLG_USERPWD,        ModuleZLG_AT_TIMEOUT,      ModuleZLG_CLIENTID,       3,      0},
    { AT_ZLG_USERPWD,       AT_ZLG_AUTOPUB,        ModuleZLG_AT_TIMEOUT,      ModuleZLG_USERPWD,        3,      0},
    { AT_ZLG_AUTOPUB,       AT_ZLG_MQTTKEEP,       ModuleZLG_AT_TIMEOUT,      ModuleZLG_AUTOPUB,        3,      0},
    { AT_ZLG_MQTTKEEP,      AT_ZLG_CLEANSESSION,   ModuleZLG_AT_TIMEOUT,      ModuleZLG_MQTTKEEP,       3,      0},
    { AT_ZLG_CLEANSESSION,  AT_ZLG_MODE,           ModuleZLG_AT_TIMEOUT,      ModuleZLG_CLEANSESSION,   3,      0},
  
    { AT_ZLG_MODE,          AT_ZLG_TRNMODE,        ModuleZLG_AT_TIMEOUT,      ModuleZLG_MODE,           3,      0},
    { AT_ZLG_TRNMODE,       AT_ZLG_EXIT_CONFIG,    ModuleZLG_AT_TIMEOUT,      ModuleZLG_TRNMODE,        3,      0},

    { AT_ZLG_EXIT_CONFIG,   AT_ZLG_NONE,           ModuleZLG_AT_TIMEOUT,      ModuleZLG_ExitConfig,     3,      0},

};

/*

POWER OFF DTU 关机 
POWER ON DTU 开机 
DTU INIT DTU 初始化 
NO SIM DTU 找不到 SIM 卡 
DTU SET DTU 进入配置状态 
DTU PPP DTU 正在 PPP 拨号 
DTU CONNECTING DTU 正在连接服务器 
DTU CONNECT OK DTU 连接服务器成功 
AT SMS DTU 处于 AT 命令的短信模式 
TRAN SMS DTU 处于透明短信模式
*/

char * ModuleZLG_AutoStatus(char *pdata, uint16_t length) {
    if(strstr(pdata,"MQTT CONNECTED")) {
        pModule4GInfor->connect = 1;
    }
    
    if(strstr(pdata,"MQTT CLOSED")) {
        pModule4GInfor->connect = 0;
    }
    
    return NULL;
}

/*
POWER OFF DTU 关机 
POWER ON DTU 开机 
DTU INIT DTU 初始化 
NO SIM DTU 找不到 SIM 卡 
DTU SET DTU 进入配置状态 
DTU PPP DTU 正在 PPP 拨号 
DTU CONNECTING DTU 正在连接服务器 
DTU CONNECT OK DTU 连接服务器成功 
AT SMS DTU 处于 AT 命令的短信模式 
TRAN SMS DTU 处于透明短信模式
*/
#if 0
static RX_CMD_Handle_t ModuleZLG_rx_handle[] = {
	{"DTU CONNECT OK DTU",                   ModuleZLG_AutoStatus},
	
};

#define MAX_ModuleZLG_RX_COMMAND sizeof(ModuleZLG_rx_handle)/sizeof(ModuleZLG_rx_handle[0])
#endif

void ModuleZLG_RxMsgHandle(uint8_t *pMsg,uint16_t length) {
#if 0
	for(uint8_t index = 0; index < MAX_ModuleZLG_RX_COMMAND; index++) {
		char *pIndexCmdStr = strstr((char *)pMsg, ModuleZLG_rx_handle[index].command);
		/* handle the rx command. */
		if(pIndexCmdStr != NULL) {
			if(ModuleZLG_rx_handle[index].rx_handle != NULL) {
				ModuleZLG_rx_handle[index].rx_handle((char *)pMsg,length);
			}
		}
	}
    
#endif
 

}










































