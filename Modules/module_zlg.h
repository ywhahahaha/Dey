#ifndef __MODULE_ZLG_H__
#define __MODULE_ZLG_H__

#include <stdbool.h>
#include <stdint.h>

#include "bsp_gpio.h"

#include "typedef.h"


typedef enum
{
    AT_ZLG_ENTER_CONFIG = 0,
    AT_ZLG_SERS,
    AT_ZLG_SERBAUD,

    AT_ZLG_IPPORT,
    AT_ZLG_CLIENTID,
    AT_ZLG_USERPWD,
    AT_ZLG_AUTOPUB,
    AT_ZLG_MQTTKEEP,
    AT_ZLG_CLEANSESSION,

    AT_ZLG_MODE,
    AT_ZLG_TRNMODE,
    AT_ZLG_EXIT_CONFIG,
    AT_ZLG_NONE,
     
}AT_ZLG_t;

#define ModuleZLG_AT_TIMEOUT     800ul

extern void module_4g_power_on(void);
extern void module_4g_power_off(void);

#define ModuleZLG_POWER_EN_H()   
#define ModuleZLG_POWER_EN_L()   
#define ModuleZLG_POWERKEY_H()   
#define ModuleZLG_POWERKEY_L()   
#define ModuleZLG_RESET_H()      module_4g_power_on()
#define ModuleZLG_RESET_L()	     module_4g_power_off()


void ModuleZLG_Reset(void);

void ModuleZLG_Init(void);
errStatus_t ModuleZLG_Register(void);


errStatus_t ModuleZLG_GMR(uint8_t para);
errStatus_t ModuleZLG_EnterConfig(uint8_t para);
errStatus_t ModuleZLG_ExitConfig(uint8_t para);
errStatus_t ModuleZLG_AUTOATO(uint8_t para);
errStatus_t ModuleZLG_AUTOSTATUS(uint8_t para);
errStatus_t ModuleZLG_IPPORT(uint8_t para);
errStatus_t ModuleZLG_CLIENTID(uint8_t para);
errStatus_t ModuleZLG_USERPWD(uint8_t para);
errStatus_t ModuleZLG_AUTOPUB(uint8_t para);
errStatus_t ModuleZLG_MQTTKEEP(uint8_t para);
errStatus_t ModuleZLG_CLEANSESSION(uint8_t para);
errStatus_t ModuleZLG_CACHE(uint8_t para);
errStatus_t ModuleZLG_DTUMODE(uint8_t para);
errStatus_t ModuleZLG_SAVE(uint8_t para);
errStatus_t ModuleZLG_CFUN(uint8_t para);
errStatus_t ModuleZLG_CSQ(uint8_t para);
errStatus_t ModuleZLG_ASKCONNECT(uint8_t para);
errStatus_t ModuleZLG_TIME(uint8_t para);
errStatus_t ModuleZLG_DTUPACKET(uint8_t para);
void ModuleZLG_RxMsgHandle(uint8_t *pMsg,uint16_t length);
errStatus_t ModuleZLG_GETDTUSTATE(uint8_t para);

#endif
