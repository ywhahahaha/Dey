#ifndef __APPL_RS485_MANAGE_H_
#define __APPL_RS485_MANAGE_H_


typedef void (*comm_tx_handle)(uint8_t *,uint16_t);
comm_tx_handle appl_rs485_search_port_type_tx_handle(uint8_t port_type);

typedef void (*power_on)(void);
typedef void (*power_off)(void);

typedef void (*port_cfg)(BaudRate_t baudrate);

power_on appl_rs485_search_port_type_power_on(uint8_t port_type);
power_off appl_rs485_search_port_type_power_off(uint8_t port_type);
uint8_t appl_rs485_get_port_type_index(uint8_t port_type);
port_cfg appl_rs485_search_port_type_cfg(uint8_t port_type);

#endif
