#ifndef __I03T_LIST_H_
#define __I03T_LIST_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "my_list.h"
#include "module_alarm.h"
#include "module_hist.h"
#include "module_discharge.h"

typedef struct {
    uint32_t i03t_comm_once  :  1;
    uint32_t i03t_comm_err   :  1;
    uint32_t cell_alarm      :  1;
    uint32_t i03t_soc_init   :  1;
    uint32_t i03t_sample_res :  1;
    uint32_t i03t_reset      :  1;
    uint32_t i03t_synch_time :  1;
    
    
    uint32_t synch_config    :  1;
    uint32_t synch_sn_need   :  1;
    uint32_t synch_sn_force  :  1;
    
    

}__I03TFlag_t;

typedef union {
    volatile __I03TFlag_t bits;
    volatile uint32_t     all;
}I03TFlag_t;

typedef struct {
    uint8_t             i03t_addr;
    uint8_t             mount;
    uint8_t             comm_err_cnt;
    uint16_t            spare;
    I03TFlag_t          flag;
    AlarmStorage_t      alarm;
    AlarmMsg_t          alarm_msg[AlarmTypeMax][AlarmLevel_Max];
    DATE_yymmddhhmmss_t sample_time; 
    CellHistStorage_t   hist;
    Discharge_t         discharge;
    Charge_t            charge;
    uint32_t            discharge_store_time;
    uint32_t            charge_store_time;
    DATE_yymmddhhmmss_t soc_store_time;
    SNRequest_t         sn_request;
    uint32_t            cell_data_pull_time_tick;
    
    PeakInfo_t          cell_volt_peak;
    PeakInfo_t          cell_temp_peak;
    PeakInfo_t          cell_res_peak;
    uint16_t            soft_version;
    uint16_t            hard_version;
    uint16_t            hist_cell_cnt;
    
}I03T_Info_t;


typedef struct {
    I03T_Info_t      node;
    struct list_head list;
}I03TNodeList_t;


void i03t_node_load(void);

I03T_Info_t *i03t_node_find(uint16_t addr);
void i03t_node_update(void);
bool i03t_node_add(uint16_t addr);
bool i03t_node_add_addr(uint16_t addr,uint8_t mount);

bool i03t_node_remove(uint16_t addr);
void i03t_node_remove_addr(uint16_t addr);
#endif
