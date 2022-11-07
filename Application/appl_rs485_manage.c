#include "typedef.h"
#include "bsp_rs485.h"
#include "bsp_gpio.h"
#include "appl_rs485_manage.h"

const static comm_tx_handle tx_handles[CONFIG_MAX_485_PORT] = {
    Bsp_Rs485_1_SendBytes,
    Bsp_Rs485_2_SendBytes,
    Bsp_Rs485_3_SendBytes,
    Bsp_Rs485_4_SendBytes
};

const static power_on rs485_power_on[CONFIG_MAX_485_PORT] = {
    Bsp_Com1Enable,
    Bsp_Com2Enable,
    Bsp_Com3Enable,
    Bsp_Com4Enable,
};

const static power_off rs485_power_off[CONFIG_MAX_485_PORT] = {
    Bsp_Com1Disable,
    Bsp_Com2Disable,
    Bsp_Com3Disable,
    Bsp_Com4Disable,
};

const static port_cfg rs485_cfg[CONFIG_MAX_485_PORT] = {
    Bsp_Rs485_1_Init,
    Bsp_Rs485_2_Init,
    Bsp_Rs485_3_Init,
    Bsp_Rs485_4_Init,
};

comm_tx_handle appl_rs485_search_port_type_tx_handle(uint8_t port_type) {
    for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
        if(pDEApp->device_config.i03m.rs485_cfg[i].port_type == port_type) {
            return tx_handles[i];
        }
    }
    
    return NULL;
}

uint8_t appl_rs485_get_port_type_index(uint8_t port_type) {
    for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
        if(pDEApp->device_config.i03m.rs485_cfg[i].port_type == port_type) {
            return i + 1;
        }
    }
    
    return 0;
}

power_on appl_rs485_search_port_type_power_on(uint8_t port_type) {
    for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
        if(pDEApp->device_config.i03m.rs485_cfg[i].port_type == port_type) {
            return rs485_power_on[i];
        }
    }
    
    return NULL;
}

power_off appl_rs485_search_port_type_power_off(uint8_t port_type) {
    for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
        if(pDEApp->device_config.i03m.rs485_cfg[i].port_type == port_type) {
            return rs485_power_off[i];
        }
    }
    
    return NULL;
}

port_cfg appl_rs485_search_port_type_cfg(uint8_t port_type) {
    for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
        if(pDEApp->device_config.i03m.rs485_cfg[i].port_type == port_type) {
            return rs485_cfg[i];
        }
    }
    
    return NULL;
}

errStatus_t appl_rs485_conflict_check(void) {
    uint8_t type_4g_cnt = 0;
    
    
    for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
        if(pDEApp->device_config.i03m.rs485_cfg[i].port_type == PORT_4G ||\
           pDEApp->device_config.i03m.rs485_cfg[i].port_type == PORT_ETH ) {
            type_4g_cnt++;
        }
    }

    if(type_4g_cnt > 1) {
        for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
            pDEApp->device_config.i03m.rs485_cfg[i].baudrate.bits.baudrate = BAUDRATE_115200;
            pDEApp->device_config.i03m.rs485_cfg[i].protocol = PROTOCOL_TYPE_DE_MODBUS;
            pDEApp->device_config.i03m.rs485_cfg[i].port_type = PORT_485;
        }
        
        pDEApp->device_config.i03m.rs485_cfg[0].port_type = PORT_4G;
        
        return errErr;
    }
    
    return errOK;
}
