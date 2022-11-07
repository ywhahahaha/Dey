#include "main.h"
#include "module_4g.h"
#include "module_tas.h"
#include "module_zlg.h"
#include "module_eth.h"
#include "appl_rs485_manage.h"

/*

user name.
JbjVy3HM7MjTy5YhHU7X
public.
v1/gateway/telemetry
{
    "c428e635b5ac464f9f182061c13afada": 
    [
        {
            "ts": 1644025767526,
            "values": 
            {
                "TolVolt": "5161.22",
                "Soc":1000,
                "TolCur":-1.7,
                "Status":3,
                "TolTs":1644025767526,
                "TolVer":"V1.0.7",
                "TolTs":1644025767526,
                "CellVer":"21",
                "CellTs":1644025767526,
                "DebugDetail":"1",
                "DebugTs":1644025767526,
                "DebugType":4
            }
        }
    ],
 
    "a357a93bb2e04161a74ecdcdd7587415":
    [
        {
            "ts":1636618423339,
            "values":
            {
                "Volt":2002,
                "Res":0,
                "Temp":229
            }
        }
    ]
}

subscribe.
v1/devices/me/rpc/request/+
*/

/* save nb information */
Module4GInfor_t   Module4GInfor = {0};
Module4GInfor_t * pModule4GInfor = &Module4GInfor;

void Bsp_Rs485Uplink2SendBytes(uint8_t *pdata,uint16_t length);
void hal_usart_wireless_sendbytes(uint8_t *buff, uint16_t length);

typedef void(* module_4g_tx_cb)(uint8_t *,uint16_t);
module_4g_tx_cb module_tx_cb = NULL;

power_on module_4g_power_on_cb = NULL;
power_off module_4g_power_off_cb = NULL;
port_cfg module_port_cfg_cb = NULL;
/*
at command send bytes.
*/
void AT_SendBytes(uint8_t *sendbytes,uint16_t length) {
    if(module_tx_cb != NULL) {
        module_tx_cb(sendbytes,length);
    }
}

void module_4g_power_on(void) {
    if(module_4g_power_on_cb != NULL) {
        module_4g_power_on_cb();
    }
}

void module_4g_power_off(void) {
    if(module_4g_power_off_cb != NULL) {
        module_4g_power_off_cb();
    }
}

void module_4g_port_cfg(BaudRate_t baudrate) {
    if(module_port_cfg_cb != NULL) {
        module_port_cfg_cb(baudrate);
    }
}

bool isExistWirlessModule(void) {
    return pModule4GInfor->imei[0] != 0;
}

bool isCheckSimCard(void) {
	return pModule4GInfor->iccid[0] != 0;
}

bool isWirelessBusy(void) {
	if(pModule4GInfor->is_sending || pModule4GInfor->pending) {
		return true;
	}
	
	return false;
}

bool isCSQGood(void) {
	if(pModule4GInfor->rssi > 3 && pModule4GInfor->rssi != 99) {
		return true;
	}
	return false;
}


/*
send command and wait nb module or platform ack.
*/
extern osMessageQueueId_t mid_MsgAtResp;
void debug_sendbytes(uint8_t *pdata,uint16_t bytes);

void module_4g_msg_flush(void) {
    uint32_t msg;       
    
    while(1) {
        osStatus_t status = osMessageQueueGet(mid_MsgAtResp,&msg,NULL,0);
        if(status == osOK) { 
            LinkerMsg_t *pMsg = (LinkerMsg_t *)msg;
            if(pMsg != NULL && pMsg->pdata != NULL) {
                sys_free(pMsg);
                sys_free(pMsg->pdata);
                pMsg = NULL;
                pMsg->pdata = NULL;  
            }
        }  else {
            break;
        }
    }
}
errStatus_t module_4g_wait_response(const char *send_bytes,
                            uint16_t send_length,
                            char *expect_ack,
                            char *response,
                            uint16_t response_length,
                            uint16_t time_out) {
    pModule4GInfor->pending = true;   
                    
    module_4g_msg_flush();
                   
    if(pDEApp->Flag.bits.g4_trace) {
        debug_printf("%s",send_bytes);
    }
                                
    uint32_t send_tick = osKernelGetTickCount();
              
    AT_SendBytes((uint8_t *)send_bytes,send_length);

    errStatus_t err = errErr;
                                
    bool breakFlg = false;
                                
    uint32_t delay = time_out;
                                
    while(1) {
        uint32_t msg;
        osStatus_t status = osMessageQueueGet(mid_MsgAtResp,&msg,NULL,delay);
        if(status == osOK) { 
            LinkerMsg_t *pMsg = (LinkerMsg_t *)msg;
            if(pMsg != NULL && pMsg->pdata != NULL) {
                if(strstr((const char *)pMsg->pdata, (const char *)expect_ack) != NULL) {
                    err = errOK;
                    if(response != NULL) {
                        uint16_t rx_length = response_length > pMsg->length ? pMsg->length : response_length;
                        memset(response,0,response_length);
                        memcpy(response,pMsg->pdata,rx_length);
                    }
                    breakFlg = true;
                } else if(strstr((const char *)pMsg->pdata, (const char *)"ERROR") != NULL) {
                    breakFlg = true;    
                } else {
                    uint32_t os_tick = osKernelGetTickCount();
                    if(os_tick > send_tick + time_out || os_tick < send_tick ) {
                        breakFlg = true;  
                    }
                }   
            }

            sys_free(pMsg);
            sys_free(pMsg->pdata);
            pMsg = NULL;
            pMsg->pdata = NULL;  
        } else {
            break;
        }
        
        if(breakFlg) {
            break;
        } 
    }

    pModule4GInfor->pending = false;

    return err;
}
							

void Module4G_Init(void) {

    module_tx_cb = appl_rs485_search_port_type_tx_handle(PORT_4G);
    if(module_tx_cb == NULL) {
        module_tx_cb = appl_rs485_search_port_type_tx_handle(PORT_ETH);
    }

    module_4g_power_on_cb = appl_rs485_search_port_type_power_on(PORT_4G);
    if(module_4g_power_on_cb == NULL) {
        module_4g_power_on_cb = appl_rs485_search_port_type_power_on(PORT_ETH);
    }
    
    module_4g_power_off_cb = appl_rs485_search_port_type_power_off(PORT_4G);
    if(module_4g_power_off_cb == NULL) {
        module_4g_power_off_cb = appl_rs485_search_port_type_power_off(PORT_ETH);
    }
    
    module_port_cfg_cb = appl_rs485_search_port_type_cfg(PORT_4G);
    if(module_port_cfg_cb == NULL) {
        module_port_cfg_cb = appl_rs485_search_port_type_cfg(PORT_ETH);
    }
    switch(pDEApp->device_config.i03m.module_type) {
        case MODULE_4G_TAS:
            ModuleTAS_Init();
            break;
        case MODULE_4G_ZLG:
            ModuleZLG_Init();
            break;
        case MODULE_ETH_TAS:
            //eth tas init.
            break;
        default:
            ModuleZLG_Init();
            break;
    }
}


bool isExistG4thModule(void) {
    return pModule4GInfor->imei[0] != 0;
}

void Module4G_KeepAlive(uint8_t tryTimes,uint16_t msgId) {

}

errStatus_t Module4G_Register(void) {
    switch(pDEApp->device_config.i03m.module_type) {
        case MODULE_4G_TAS:
            return ModuleTAS_Register();

        case MODULE_4G_ZLG:
            return ModuleZLG_Register();
            
        case MODULE_ETH_TAS:
            return  ModuleETH_Register();
    }
    
    return ModuleZLG_Register();
}


void ModuleTAS_RxMsgHandle(uint8_t *pMsg,uint16_t length);
void Module4G_RxMsgHandle(LinkerMsg_t *msg) {

    switch(pDEApp->device_config.i03m.module_type) {
        case MODULE_4G_TAS:
            ModuleTAS_RxMsgHandle(msg->pdata,msg->length);
            break;
        case MODULE_4G_ZLG:
            ModuleZLG_RxMsgHandle(msg->pdata,msg->length);
            break;
        case MODULE_ETH_TAS:
            ModuleETH_RxMsgHandle(msg->pdata,msg->length);
            break;
    }
    
    
    //application json data handle.
    //......
}

void Module4G_Handle(uint8_t *pdata,uint16_t length) {
    LinkerMsg_t *msg = sys_malloc(sizeof(LinkerMsg_t));

    if(msg != NULL) {
        msg->length = length;
        msg->pdata = sys_malloc(msg->length + 1);

        if(msg->pdata != NULL) {
            memset(msg->pdata,0,msg->length + 1);
            memcpy(msg->pdata,pdata,msg->length);
        } else {
            sys_free(msg);
            msg = NULL;
            return;
        }
    } else {
        return;
    }

    if(pDEApp->Flag.bits.g4_trace) {
        debug_printf("%s",msg->pdata);
    }
    
    Module4G_RxMsgHandle(msg);
    
    if(pModule4GInfor->pending) {
        osStatus_t status = osMessageQueuePut(mid_MsgAtResp,&msg,osPriorityNormal,0);
        if(status != osOK) {
            sys_free(msg->pdata);
            sys_free(msg);
        }
    } else {
        sys_free(msg->pdata);
        sys_free(msg);
    }

}


