#ifndef __MODULE_ETH_H__
#define __MODULE_ETH_H__

#include <stdbool.h>
#include <stdint.h>

#include "bsp_gpio.h"

#include "typedef.h"

#define MQTT_SUB_TOPIC_1           "v1/devices/me/rpc/request/+"
typedef enum
{
    AT_ETH_ENTER_CONFIG = 0,
    AT_ETH_CLOSE_COM1,
    AT_ETH_SOCKE_MODE,

    AT_ETH_CLIENTID,
    AT_ETH_USERPWD,
    AT_ETH_AUTOPUB,
    AT_ETH_MQTTKEEP,
    AT_ETH_CLEANSESSION,
    AT_ETH_SET_IPCONFIG,

    AT_ETH_SET_ACKTIME,
    AT_ETH_SET_PORTTIME,
    AT_ETH_SET_RESTTIME,
    AT_ETH_AUTOSUB,
    AT_ETH_SERBAUD,
    AT_ETH_SET_SAVE,
    AT_ETH_SET_REBOT,
    AT_ETH_NONE,
     
}AT_ETH_t;

#define ModuleETH_AT_TIMEOUT     800ul

errStatus_t ModuleETH_Register(void);
errStatus_t ModuleETH_EnterConfig(uint8_t para);
errStatus_t ModuleETH_CLOSE_COM1(uint8_t para);
errStatus_t ModuleETH_SET_SOCKE_MODE(uint8_t para);
errStatus_t ModuleETH_SET_SOCKE_MODE(uint8_t para);
errStatus_t ModuleETH_CLIENTID(uint8_t para);
errStatus_t ModuleETH_USERPWD(uint8_t para);
errStatus_t ModuleETH_AUTOPUB(uint8_t para);
errStatus_t ModuleETH_AUTOSUB(uint8_t para);
errStatus_t ModuleETH_SET_IPCONFIG(uint8_t para);
errStatus_t ModuleETH_CLEANSESSION(uint8_t para);
errStatus_t ModuleETH_SET_ACKTIME(uint8_t para);
errStatus_t ModuleETH_SET_PORTTIME(uint8_t para);
errStatus_t ModuleETH_SET_RESTTIME(uint8_t para);
errStatus_t ModuleETH_SET_SAVE(uint8_t para);
errStatus_t ModuleETH_SET_REBOT(uint8_t para);
void ModuleETH_RxMsgHandle(uint8_t *pMsg,uint16_t length);

#endif
