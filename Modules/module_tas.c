#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <strTools.h>
#include "main.h"
#include "stm32f4xx.h"
#include "module_4g.h"
#include "module_tas.h"
#include "bsp_gpio.h"


#define RESPONSE_MAX_LENGTH  100


void AT_SendBytes(uint8_t *sendbytes,uint16_t length);

errStatus_t module_4g_wait_response(const char *send_bytes,
                            uint16_t send_length,
                            char *expect_ack,
                            char *response,
                            uint16_t response_length,
                            uint16_t time_out);

void ModuleTAS_PowerON(void) {
	ModuleTAS_POWERKEY_L();
	osDelay(2000);
	ModuleTAS_POWERKEY_H();
}

void ModuleTAS_Init(void) {
	ModuleTAS_PowerON();
}


void ModuleTAS_Reset(void) {
    ModuleTAS_RESET_H();
    osDelay(100);
    ModuleTAS_RESET_L();
    osDelay(4000);
    ModuleTAS_RESET_H();
}





static const AT_COMMAND_t AtCmd[];

#define RESPONSE_MAX_LENGTH  100



errStatus_t ModuleTAS_GMR(uint8_t para)
{
	char *send_bytes = "AT+CGMR\r\n";
    uint16_t send_length = strlen(send_bytes);

    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_GMR].time_out);
}

errStatus_t ModuleTAS_DTUPACKET(uint8_t para)
{
	char *send_bytes = "AT+DTUPACKET=100,1400\r\n";
    uint16_t send_length = strlen(send_bytes);

    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_DTUPACKET].time_out);
}

errStatus_t ModuleTAS_EnterConfig(uint8_t para) {

	const char *send_bytes = "+++";
    uint16_t send_length = strlen(send_bytes);

    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_ENTER_CONFIG].time_out);
    
}
errStatus_t ModuleTAS_UARTCFG(uint8_t para) {

	const char *send_bytes = "AT+UARTCFG=115200,1,0,0\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_ENTER_CONFIG].time_out);
    
}


void module_4g_port_cfg(BaudRate_t baudrate);
errStatus_t ModuleTAS_ModifyBaudrate(void) {

    errStatus_t ret = errErr;
    for(uint8_t i=0;i<20;i++) {
        ret = ModuleTAS_EnterConfig(0);
        if(ret == errOK) {
            return errOK;
        }
        osDelay(500);
    }
    
    BaudRate_t baudrate = {0};
    baudrate.bits.baudrate = BAUDRATE_9600;
    module_4g_port_cfg(baudrate);
    
    for(uint8_t i=0;i<20;i++) {
        ret = ModuleTAS_EnterConfig(0);
        if(ret == errOK) {
            ret = ModuleTAS_UARTCFG(0);
            if(ret == errOK) {
                break;
            }
        }
        osDelay(500);
    }
    
    if(ret == errOK) {
        for(uint8_t i=0;i<6;i++) {
            ret = ModuleTAS_SAVE(0);
            if(ret == errOK) {
                break;
            }
        }
        for(uint8_t i=0;i<6;i++) {
            ret = ModuleTAS_CFUN(0);
            if(ret == errOK) {  
                break;
            }
        }
    }
    
    baudrate.bits.baudrate = BAUDRATE_115200;
    module_4g_port_cfg(baudrate);
    if(ret == errOK) {
        osDelay(10000);
    }
    return ret;

}



errStatus_t ModuleTAS_ExitConfig(uint8_t para) {

	const char *send_bytes = "ATO\r\n";
    uint16_t send_length = strlen(send_bytes);

    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_ENTER_CONFIG].time_out);
    
}

errStatus_t ModuleTAS_AUTOATO(uint8_t para)
{
	const char *send_bytes = "AT+AUTOATO=30\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_AUTOATO].time_out);
    
}


errStatus_t ModuleTAS_AUTOSTATUS(uint8_t para) {

	const char *send_bytes = "AT+AUTOSTATUS=1,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_AUTOSTATUS].time_out);
    
}

errStatus_t ModuleTAS_IPPORT(uint8_t para) {

	char send_bytes[128] = {0};
    sprintf(send_bytes,"AT+IPPORT=\"%s\",%d,1\r\n",pDEApp->device_config.i03m.mqtt_config.mqtt_addr,\
                                                   pDEApp->device_config.i03m.mqtt_config.mqtt_port);
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_IPPORT].time_out);
    
}

errStatus_t ModuleTAS_CLIENTID(uint8_t para) {

	char send_bytes[64] = {0};
    sprintf(send_bytes,"AT+CLIENTID=\"%d%d\",1\r\n",rand(),rand());
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_CLIENTID].time_out);
}

errStatus_t ModuleTAS_USERPWD(uint8_t para) {

	char send_bytes[128] = {0};
    sprintf(send_bytes,"AT+USERPWD=\"%s\",\"\",1\r\n",pDEApp->device_config.i03m.mqtt_config.user_name);
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_USERPWD].time_out);
    
}


errStatus_t ModuleTAS_AUTOPUB(uint8_t para) {

	char send_bytes[128] = {0};
    sprintf(send_bytes,"AT+AUTOPUB=1,\"%s\",1,1,1\r\n",MQTT_PUB_TOPIC_1);
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_AUTOPUB].time_out);
    
}


errStatus_t ModuleTAS_MQTTKEEP(uint8_t para) {

	const char *send_bytes = "AT+MQTTKEEP=60,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_MQTTKEEP].time_out);
    
}


errStatus_t ModuleTAS_CLEANSESSION(uint8_t para) {

	const char *send_bytes = "AT+CLEANSESSION=1,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_CLEANSESSION].time_out);
    
}

errStatus_t ModuleTAS_CACHE(uint8_t para) {

	const char *send_bytes = "AT+CACHE=1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_CACHE].time_out);
    
}


errStatus_t ModuleTAS_DTUMODE(uint8_t para) {

	const char *send_bytes = "AT+DTUMODE=2,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_DTUMODE].time_out);
    
}

errStatus_t ModuleTAS_SAVE(uint8_t para) {

	const char *send_bytes = "AT&W\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_W].time_out);
    
}

errStatus_t ModuleTAS_CFUN(uint8_t para) {

	const char *send_bytes = "AT+CFUN=1,1\r\n";
    uint16_t send_length = strlen(send_bytes);
    return module_4g_wait_response(send_bytes,send_length,"OK\r\n",NULL, 0, AtCmd[AT_TAS_CFUN].time_out);
    
}

//AT+ASKCONNECT?
//+ASKCONNECT: 1,0,0,0
errStatus_t ModuleTAS_ASKCONNECT(uint8_t para) {

	const char *send_bytes = "AT+ASKCONNECT?\r\n";
    uint16_t send_length = strlen(send_bytes);
    char response[RESPONSE_MAX_LENGTH] = {0};
    
    errStatus_t ret;
    
    ret = module_4g_wait_response(send_bytes,send_length,"+ASKCONNECT: 1,",response,RESPONSE_MAX_LENGTH, ModuleZLG_AT_TIMEOUT);

    if(ret == errOK) {
        pModule4GInfor->connect = 1;
    } 
    return ret;
}


//+CSQ:21,99
errStatus_t ModuleTAS_CSQ(uint8_t para) {

	const char *send_bytes = "AT+CSQ\r\n";
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
AT+CGSN
+CGSN: 355897043139120
OK
*/
errStatus_t Module4G_CGSN(uint8_t para)
{
    #if 0
    errStatus_t ret;
    char response[RESPONSE_MAX_LENGTH];
    char *send_bytes = "AT+CGSN\r";
	ret = module_4g_wait_response(send_bytes,strlen(send_bytes),"+CGSN: ",response,AT_BUF_MAX_SIZE, AtCmd[AT_ModuleTAS_CGSN].time_out);
    if(ret == errOK) {
        char *pIndex = strstr(response, "+CGSN: ");
        if(pIndex != NULL) {
            memset(pModule4GInfor->imei, 0, sizeof(pModule4GInfor->imei));
            memcpy(pModule4GInfor->imei, pIndex + 7, 15);
        }
    }
    return ret;
    #endif
    return errOK;
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
typedef struct {
	const char *mnc;
	const char *apn;
}MNC_APN_t;

const MNC_APN_t mnc_apn[] = {
	{"46000","CMNET",},
	{"46002","CMNET",},
	{"46004","CMNET",},
	{"46007","CMNET",},
	
	{"46003","3GNET",},
	{"46005","3GNET",},
	{"46011","3GNET",},
	{"46012","3GNET",},
	
	{"46001","CTLTE",},
	{"46006","CTLTE",},
	{"46009","CTLTE",},
	{"46010","CTLTE",},
}; 

#define MNC_APN_SIZE sizeof(mnc_apn)/sizeof(mnc_apn[0])


/*
AT+CCID
+CCID: 89860002190810001367
OK
*/
errStatus_t Module4G_CCID(uint8_t para)
{
    #if 0
    errStatus_t ret;
    char response[RESPONSE_MAX_LENGTH];
    char *send_bytes = "AT+CCID\r";
    uint16_t send_length = strlen(send_bytes);

	ret = module_4g_wait_response(send_bytes,send_length,"+CCID: ",response,RESPONSE_MAX_LENGTH, AtCmd[AT_ModuleTAS_CCID].time_out);

    if(ret == errOK) {
        char *pIndex = strstr(response, "+CCID: ");
        if(pIndex != NULL) {
            memset(pModule4GInfor->iccid, 0, sizeof(pModule4GInfor->iccid));
            memcpy(pModule4GInfor->iccid, pIndex + 7, 20);
        }
    }
    return ret;
    #endif
    return errErr;
}

errStatus_t Module4G_CheckIccid(void) {
	for(uint8_t i=0;i<3;i++) {
		if(Module4G_CCID(0) == errOK){
			return errOK;
		}
	}
	
	return errErr;
}


/*
+TIME: "21/06/01,14:38:34"
*/
errStatus_t ModuleTAS_TIME(uint8_t para)
{
    errStatus_t ret;
    char response[RESPONSE_MAX_LENGTH] = {0};
    char *send_bytes = "AT+TIME\r\n";
    uint16_t send_length = strlen(send_bytes);

    ret = module_4g_wait_response(send_bytes, send_length,"+TIME: ", response, RESPONSE_MAX_LENGTH,ModuleTAS_AT_TIMEOUT);
    if(ret == errOK) {
        int a,b,c,d,e,f;
        char *p = strstr(response,"+TIME: ");
        if(p != NULL) {
            
            if(6 == sscanf((char *)p,"+TIME: \"%d/%d/%d,%d:%d:%d\"",&a,&b,&c,&d,&e,&f)) {
                DATE_yymmddhhmmss_t now;
                now.bits.year = a;
                now.bits.month = b;
                now.bits.day = c;
                now.bits.hour = d;
                now.bits.min = e;
                now.bits.sec = f;
                
                int32_t time_diff = now.date - pDEApp->now.date;
                if(abs(time_diff) > 600) {
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

errStatus_t ModuleTAS_Register(void)
{
	errStatus_t ret = errErr;
    uint32_t last_tick = osKernelGetTickCount();
	
	ModuleTAS_Reset();
	
	osDelay(10000);
	
    //Module4GStatus_SetOffline();
	
    memset((void *)pModule4GInfor, 0, sizeof(Module4GInfor_t));

    pModule4GInfor->at_code = AT_TAS_ENTER_CONFIG;
    pModule4GInfor->act_ppp = false;
    pModule4GInfor->pending = true;
	pModule4GInfor->attach = false;
	pModule4GInfor->connect = 0;
    pModule4GInfor->socket = 0;
	pModule4GInfor->iccid[0] = 0x00;
    
    ModuleTAS_ModifyBaudrate();
    
    osDelay(10000);

    int32_t diff_time;
    while(1) {
        if(AtCmd[pModule4GInfor->at_code].handle != NULL) {
            errStatus_t err = AtCmd[pModule4GInfor->at_code].handle(AtCmd[pModule4GInfor->at_code].para);
            if(err == errOK) {
                last_tick = osKernelGetTickCount();
                pModule4GInfor->at_code = AtCmd[pModule4GInfor->at_code].at_next_code;
                if(pModule4GInfor->at_code == AT_TAS_NONE) {
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
                if(pModule4GInfor->at_code == AT_TAS_NONE) {
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
    { AT_TAS_ENTER_CONFIG,  AT_TAS_GMR,            ModuleTAS_AT_TIMEOUT,      ModuleTAS_EnterConfig,    20,     0},
    { AT_TAS_GMR,           AT_TAS_DTUPACKET,      ModuleTAS_AT_TIMEOUT,      ModuleTAS_GMR,            1,      0},
    { AT_TAS_DTUPACKET,     AT_TAS_AUTOATO,        ModuleTAS_AT_TIMEOUT,      ModuleTAS_DTUPACKET,      1,      0},
    { AT_TAS_AUTOATO,       AT_TAS_AUTOSTATUS,     ModuleTAS_AT_TIMEOUT,      ModuleTAS_AUTOATO,        3,      0},
    { AT_TAS_AUTOSTATUS,    AT_TAS_IPPORT,         ModuleTAS_AT_TIMEOUT,      ModuleTAS_AUTOSTATUS,     3,      0},
    { AT_TAS_IPPORT,        AT_TAS_CLIENTID,       ModuleTAS_AT_TIMEOUT,      ModuleTAS_IPPORT,         3,      0},
    { AT_TAS_CLIENTID,      AT_TAS_USERPWD,        ModuleTAS_AT_TIMEOUT,      ModuleTAS_CLIENTID,       3,      0},
    { AT_TAS_USERPWD,       AT_TAS_AUTOPUB,        ModuleTAS_AT_TIMEOUT,      ModuleTAS_USERPWD,        3,      0},
    { AT_TAS_AUTOPUB,       AT_TAS_MQTTKEEP,       ModuleTAS_AT_TIMEOUT,      ModuleTAS_AUTOPUB,        3,      0},
    { AT_TAS_MQTTKEEP,      AT_TAS_CLEANSESSION,   ModuleTAS_AT_TIMEOUT,      ModuleTAS_MQTTKEEP,       3,      0},
    { AT_TAS_CLEANSESSION,  AT_TAS_CACHE,          ModuleTAS_AT_TIMEOUT,      ModuleTAS_CLEANSESSION,   3,      0},
    { AT_TAS_CACHE,         AT_TAS_DTUMODE,        ModuleTAS_AT_TIMEOUT,      ModuleTAS_CACHE,          3,      0},
    { AT_TAS_DTUMODE,       AT_TAS_W,              ModuleTAS_AT_TIMEOUT,      ModuleTAS_DTUMODE,        3,      0},
    { AT_TAS_W,             AT_TAS_CFUN,           ModuleTAS_AT_TIMEOUT,      ModuleTAS_SAVE,           3,      0},
    { AT_TAS_CFUN,          AT_TAS_NONE,           ModuleTAS_AT_TIMEOUT,      ModuleTAS_CFUN,           3,      0},

};

/*

+STATUS: 1, MQTT CONNECTED 
+STATUS: 1, MQTT CLOSED 
+STATUS: 1, HTTP CONNECTED 
+STATUS: 1, HTTP CLOSED
+STATUS: 1, CONNECTED
+STATUS: 1, CLOSED
*/

static char * ModuleTAS_AutoStatus(char *pdata, uint16_t length) {
    if(strstr(pdata,"MQTT CONNECTED")) {
        pModule4GInfor->connect = 1;
    }
    
    if(strstr(pdata,"MQTT CLOSED")) {
        pModule4GInfor->connect = 0;
    }
    
    return NULL;
}

/*
+EUSIM:ERROR
*/
static RX_CMD_Handle_t ModuleTAS_rx_handle[] = {
	{"+STATUS:",                   ModuleTAS_AutoStatus},
	
};

#define MAX_ModuleTAS_RX_COMMAND sizeof(ModuleTAS_rx_handle)/sizeof(ModuleTAS_rx_handle[0])
    
void ModuleTAS_RxMsgHandle(uint8_t *pMsg,uint16_t length) {
	for(uint8_t index = 0; index < MAX_ModuleTAS_RX_COMMAND; index++) {
		char *pIndexCmdStr = strstr((char *)pMsg, ModuleTAS_rx_handle[index].command);
		/* handle the rx command. */
		if(pIndexCmdStr != NULL) {
			if(ModuleTAS_rx_handle[index].rx_handle != NULL) {
				ModuleTAS_rx_handle[index].rx_handle((char *)pMsg,length);
			}
		}
	}

}









































