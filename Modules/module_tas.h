#ifndef __MODULE_TAS__
#define __MODULE_TAS__

#include <stdbool.h>
#include <stdint.h>
#include "bsp_gpio.h"
#include "typedef.h"


typedef enum
{
    AT_TAS_ENTER_CONFIG = 0,
    AT_TAS_GMR,
    AT_TAS_DTUPACKET,
    AT_TAS_AUTOATO,
    AT_TAS_AUTOSTATUS,
    AT_TAS_IPPORT,
    AT_TAS_CLIENTID,
    AT_TAS_USERPWD,
    AT_TAS_AUTOPUB,
    AT_TAS_MQTTKEEP,
    AT_TAS_CLEANSESSION,
    AT_TAS_CACHE,
    AT_TAS_DTUMODE,
    
    AT_TAS_W,
    AT_TAS_CFUN,
    AT_TAS_NONE,
     
}AT_TAS_t;

#define ModuleTAS_AT_TIMEOUT     800ul

extern void module_4g_power_on(void);
extern void module_4g_power_off(void);

#define ModuleTAS_POWER_EN_H()   
#define ModuleTAS_POWER_EN_L()   
#define ModuleTAS_POWERKEY_H()   
#define ModuleTAS_POWERKEY_L()   
#define ModuleTAS_RESET_H()      module_4g_power_on()
#define ModuleTAS_RESET_L()	     module_4g_power_off()


void ModuleTAS_Reset(void);

void ModuleTAS_Init(void);
errStatus_t ModuleTAS_Register(void);


errStatus_t ModuleTAS_GMR(uint8_t para);
errStatus_t ModuleTAS_EnterConfig(uint8_t para);
errStatus_t ModuleTAS_ExitConfig(uint8_t para);
errStatus_t ModuleTAS_AUTOATO(uint8_t para);
errStatus_t ModuleTAS_AUTOSTATUS(uint8_t para);
errStatus_t ModuleTAS_IPPORT(uint8_t para);
errStatus_t ModuleTAS_CLIENTID(uint8_t para);
errStatus_t ModuleTAS_USERPWD(uint8_t para);
errStatus_t ModuleTAS_AUTOPUB(uint8_t para);
errStatus_t ModuleTAS_MQTTKEEP(uint8_t para);
errStatus_t ModuleTAS_CLEANSESSION(uint8_t para);
errStatus_t ModuleTAS_CACHE(uint8_t para);
errStatus_t ModuleTAS_DTUMODE(uint8_t para);
errStatus_t ModuleTAS_SAVE(uint8_t para);
errStatus_t ModuleTAS_CFUN(uint8_t para);
errStatus_t ModuleTAS_CSQ(uint8_t para);
errStatus_t ModuleTAS_ASKCONNECT(uint8_t para);
errStatus_t ModuleTAS_TIME(uint8_t para);
errStatus_t ModuleTAS_DTUPACKET(uint8_t para);


#endif
