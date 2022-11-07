/*
 * protocol_handle.c
 *
 *  Created on: 2021年5月10日
 *      Author: weridy
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "de_protocol.h"
#include "main.h"
#include "i03t_list.h"
#include "iap.h"
#include "module_sn.h"
#include "logger.h"



uint16_t start_cell_id = 1;

errStatus_t app_para_save(void);

void protocol_set_query_start_cell_id(uint16_t id) {
    start_cell_id = id;
}

uint16_t fill_query_status_cell_info(uint8_t i03t,uint8_t *p,uint8_t *remain) {
    
    #define FRMAE_MAX_CELL 100
    
    *remain = 0;
    
    if(i03t > CONFIG_MAX_I03T || i03t == 0) {
        return 0;
    }
    
    I03T_Info_t *i03t_node = i03t_node_find(i03t);
    if(i03t_node == NULL) {
        return 0;
    }

    uint16_t max_cells = pDEApp->device_config.i03t_nodes[i03t-1].sys_para.cell_number;
    
    uint16_t index = 0;
    p[index++] = 0;
    p[index++] = i03t;
    
    uint16_t cell_cnt = 0;
    uint16_t temp_start_id;
    
    for(temp_start_id = start_cell_id;temp_start_id <= max_cells && cell_cnt < FRMAE_MAX_CELL;temp_start_id++,cell_cnt++) {
        
        p[index++] = StatusDataId_CellInfo;
        p[index++] = temp_start_id >> 8;
        p[index++] = temp_start_id;
        
        p[index++] = i03t_node->hist.cells[temp_start_id-1].voltage >> 8;
        p[index++] = i03t_node->hist.cells[temp_start_id-1].voltage;
        
        p[index++] = i03t_node->hist.cells[temp_start_id-1].inter_res >> 8;
        p[index++] = i03t_node->hist.cells[temp_start_id-1].inter_res;
        
        p[index++] = i03t_node->hist.cells[temp_start_id-1].temperature >> 8;
        p[index++] = i03t_node->hist.cells[temp_start_id-1].temperature;
        
    }
    
    
    if(temp_start_id < max_cells) {
        *remain = 1;
    } 
    
    start_cell_id = temp_start_id;
    
    return index; 
}

uint16_t fill_query_status_cell_alarm(uint8_t i03t,uint8_t *p) {
    if(i03t > CONFIG_MAX_I03T || i03t == 0) {
        return 0;
    }
    
    I03T_Info_t *i03t_node = i03t_node_find(i03t);
    if(i03t_node == NULL) {
        return 0;
    }
    uint16_t index = 0;
    p[index++] = 0;
    p[index++] = i03t;
    
    uint16_t cell_start_id;
    uint16_t max_cells = pDEApp->device_config.i03t_nodes[i03t-1].sys_para.cell_number;
    
    for(cell_start_id = 1;cell_start_id <= max_cells;cell_start_id++) {
        
        p[index++] = StatusDataId_CellAlarmInfo;
        p[index++] = cell_start_id >> 8;
        p[index++] = cell_start_id;
        
        p[index++] = i03t_node->alarm.alarm.cell_alarm[cell_start_id-1] >> 8;
        p[index++] = i03t_node->alarm.alarm.cell_alarm[cell_start_id-1];
    }
    
    return index;
    
}


uint16_t fill_query_status_i03m(uint8_t *p) {
    
    uint16_t index = 0;
    p[index++] = 0;
    p[index++] = 0;

    p[index++] = StatusDataId_DryNodeStatus;
    p[index++] = 0;
    
    p[index++] = StatusDataId_CellNumber;
    p[index++] = pDEApp->device_config.i03m.cell_numbers >> 8;
    p[index++] = pDEApp->device_config.i03m.cell_numbers;
    
    p[index++] = StatusDataId_I03TNumber;
    p[index++] = pDEApp->device_config.i03m.i03t_number;

    p[index++] = StatusDataId_SoftVersion;
    p[index++] = I03M_SOFT_VERSION >> 8;
    p[index++] = (uint8_t)I03M_SOFT_VERSION;
    
    p[index++] = StatusDataId_HardVersion;
    p[index++] = I03M_HARD_VERSION >> 8;
    p[index++] = (uint8_t)I03M_HARD_VERSION;
    
    DATE_yymmddhhmmss_t now;
    Bsp_RtcGetTime(&now);
    p[index++] = StatusDataId_Time;
    p[index++] = now.bits.year;
    p[index++] = now.bits.month;
    p[index++] = now.bits.day;
    p[index++] = now.bits.hour;
    p[index++] = now.bits.min;
    p[index++] = now.bits.sec;
    
    
    return index;

      
}

uint16_t fill_query_status_i03t(uint8_t i03t,uint8_t *p) {
    
    int temp;
    if(i03t > CONFIG_MAX_I03T || i03t == 0) {
        return 0;
    }
    
    I03T_Info_t *i03t_node = i03t_node_find(i03t);
    if(i03t_node == NULL) {
        return 0;
    }
    
    uint16_t index = 0;
    p[index++] = 0;
    p[index++] = i03t;
    
    for(uint8_t i=0;i<CONFIG_MAX_CURRENT_CH;i++) {
        p[index++] = StatusDataId_BatGroupStatus;
        p[index++] = i+1;
        p[index++] = i03t_node->discharge.status;
    }

    
    temp = multiple(i03t_node->discharge.voltage,MODBUS_VOLT_CONV);
    p[index++] = StatusDataId_BatGroupVoltage;
    p[index++] = temp >> 8;
    p[index++] = temp;
    
    for(uint8_t i=0;i<CONFIG_MAX_CURRENT_CH;i++){
        temp = multiple(i03t_node->discharge.current[i],MODBUS_CURRENT_CONV); 
        p[index++] = StatusDataId_BatGroupCurrent;
        p[index++] = i+1;
        p[index++] = temp >> 8;
        p[index++] = temp;
    }

    
    for(uint8_t i=0;i<CONFIG_MAX_CURRENT_CH;i++) {
        p[index++] = StatusDataId_BatGroupSOC;
        p[index++] = i+1;
        p[index++] = i03t_node->discharge.soc >> 8;
        p[index++] = i03t_node->discharge.soc;
    }

    for(uint8_t i=0;i<CONFIG_MAX_CURRENT_CH;i++) {
        temp = multiple(i03t_node->discharge.available_time,1);
        p[index++] = StatusDataId_BatAvailableTime;
        p[index++] = i+1;
        p[index++] = temp >> 8;
        p[index++] = temp;
    }

    for(uint8_t i=0;i<CONFIG_MAX_CURRENT_CH;i++) {
        p[index++] = StatusDataId_SOH;
        p[index++] = i+1;
        p[index++] = i03t_node->discharge.soh >> 8;
        p[index++] = i03t_node->discharge.soh;
    }

    
    p[index++] = StatusDataId_DryNodeStatus;
    p[index++] = i03t_node->hist.total_info.dry_node_status;
    
    p[index++] = StatusDataId_CellNumber;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t-1].sys_para.cell_number >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t-1].sys_para.cell_number;
    
    p[index++] = StatusDataId_I03TNumber;
    p[index++] = pDEApp->device_config.i03m.i03t_number;
    
    p[index++] = StatusDataId_CellPeakInfo;
    p[index++] = i03t_node->cell_volt_peak.max >> 8;
    p[index++] = i03t_node->cell_volt_peak.max;
    p[index++] = i03t_node->cell_volt_peak.min >> 8;
    p[index++] = i03t_node->cell_volt_peak.min;
    p[index++] = i03t_node->cell_volt_peak.average >> 8;
    p[index++] = i03t_node->cell_volt_peak.average;
    p[index++] = i03t_node->cell_volt_peak.max_id >> 8;
    p[index++] = i03t_node->cell_volt_peak.max_id;
    p[index++] = i03t_node->cell_volt_peak.min_id >> 8;
    p[index++] = i03t_node->cell_volt_peak.min_id;
    p[index++] = i03t_node->cell_volt_peak.diff >> 8;
    p[index++] = i03t_node->cell_volt_peak.diff;
    
    p[index++] = StatusDataId_TempPeakInfo;
    p[index++] = i03t_node->cell_temp_peak.max >> 8;
    p[index++] = i03t_node->cell_temp_peak.max;
    p[index++] = i03t_node->cell_temp_peak.min >> 8;
    p[index++] = i03t_node->cell_temp_peak.min;
    p[index++] = i03t_node->cell_temp_peak.average >> 8;
    p[index++] = i03t_node->cell_temp_peak.average;
    p[index++] = i03t_node->cell_temp_peak.max_id >> 8;
    p[index++] = i03t_node->cell_temp_peak.max_id;
    p[index++] = i03t_node->cell_temp_peak.min_id >> 8;
    p[index++] = i03t_node->cell_temp_peak.min_id;
    p[index++] = i03t_node->cell_temp_peak.diff >> 8;
    p[index++] = i03t_node->cell_temp_peak.diff;
    
    p[index++] = StatusDataId_ResPeakInfo;
    p[index++] = i03t_node->cell_res_peak.max >> 8;
    p[index++] = i03t_node->cell_res_peak.max;
    p[index++] = i03t_node->cell_res_peak.min >> 8;
    p[index++] = i03t_node->cell_res_peak.min;
    p[index++] = i03t_node->cell_res_peak.average >> 8;
    p[index++] = i03t_node->cell_res_peak.average;
    p[index++] = i03t_node->cell_res_peak.max_id >> 8;
    p[index++] = i03t_node->cell_res_peak.max_id;
    p[index++] = i03t_node->cell_res_peak.min_id >> 8;
    p[index++] = i03t_node->cell_res_peak.min_id;
    p[index++] = i03t_node->cell_res_peak.diff >> 8;
    p[index++] = i03t_node->cell_res_peak.diff;
    
    p[index++] = StatusDataId_SoftVersion;
    p[index++] = i03t_node->soft_version >> 8;
    p[index++] = i03t_node->soft_version;
    
    p[index++] = StatusDataId_HardVersion;
    p[index++] = i03t_node->hard_version >> 8;
    p[index++] = i03t_node->hard_version;
    
    DATE_yymmddhhmmss_t now;
    Bsp_RtcGetTime(&now);
    p[index++] = StatusDataId_Time;
    p[index++] = now.bits.year;
    p[index++] = now.bits.month;
    p[index++] = now.bits.day;
    p[index++] = now.bits.hour;
    p[index++] = now.bits.min;
    p[index++] = now.bits.sec;
    
    p[index++] = StatusDataId_BatGroupAlarmInfo;
    p[index++] = i03t_node->alarm.alarm.bat_group_alarm1 >> 8;
    p[index++] = i03t_node->alarm.alarm.bat_group_alarm1;
    p[index++] = i03t_node->alarm.alarm.bat_group_alarm2 >> 8;
    p[index++] = i03t_node->alarm.alarm.bat_group_alarm2;
    p[index++] = i03t_node->alarm.alarm.bat_group_alarm3 >> 8;
    p[index++] = i03t_node->alarm.alarm.bat_group_alarm3;
    
    return index;

      
}

uint16_t fill_query_status(uint8_t i03t,uint8_t dataid,uint8_t *p,uint8_t *remain) {
    *remain = 0;
    if(i03t > CONFIG_MAX_I03T) {
        return 0;
    }
    
    if(i03t == 0) {
        return fill_query_status_i03m(p);
    }
    
    I03T_Info_t *i03t_node = i03t_node_find(i03t);
    if(i03t_node == NULL) {
        return 0;
    }
    
    switch(dataid) {
        case StatusDataId_CellInfo:
            return fill_query_status_cell_info(i03t,p,remain);

        case StatusDataId_CellAlarmInfo: 
            return fill_query_status_cell_alarm(i03t,p);

        default:
            return fill_query_status_i03t(i03t,p);
   
    }

}

uint16_t fill_query_config_sn(uint8_t i03t,uint8_t dataid,uint8_t *p,uint8_t *remain) {
    
    #define FRMAE_MAX_SN 20
    
    *remain = 0;
    
    if(i03t > CONFIG_MAX_I03T || i03t == 0) {
        return 0;
    }
    
    I03T_Info_t *i03t_node = i03t_node_find(i03t);
    if(i03t_node == NULL) {
        return 0;
    }

    uint16_t max_cells = CONFIG_MAX_CELL;
    
    uint16_t index = 0;
    p[index++] = 0;
    p[index++] = i03t;
    
    uint16_t cell_cnt = 0;
    uint16_t temp_start_id;
    
    for(temp_start_id = start_cell_id;temp_start_id <= max_cells && cell_cnt < FRMAE_MAX_SN;temp_start_id++,cell_cnt++) {
        
        p[index++] = dataid;//ConfigDataId_AddSN;
        p[index++] = temp_start_id >> 8;
        p[index++] = temp_start_id;
        
        SNStore_t *sn = module_sn_get_by_index(i03t,temp_start_id);
        if(sn == NULL) {
            temp_start_id = max_cells;
            break;
        }
        p[index++] = sn->cell_on_current_group;
        
        memcpy(p + index,sn->sn,CONFIG_SN_LENGTH);
        index += CONFIG_SN_LENGTH;
        
        if(dataid == ConfigDataId_AddSNCloudId) {
            memcpy(p + index,sn->cloud_id,CONFIG_CLOUD_ID_LENGTH);
            index += CONFIG_CLOUD_ID_LENGTH;
        }

        sys_free(sn);
    }
    
    
    if(temp_start_id < max_cells) {
        SNStore_t *sn = module_sn_get_by_index(i03t,temp_start_id);
        if(sn == NULL) {
            temp_start_id = max_cells;
            *remain = 0;
        } else {
            sys_free(sn);
            *remain = 1;
        }  
    } 
    
    start_cell_id = temp_start_id;
    
    return index; 
}

uint16_t fill_query_config_i03t(uint8_t addr,uint8_t *p,uint8_t query) {
    if(addr < 1 || addr > CONFIG_MAX_IO3T_MODBUS_ADDR) {
        return 0;
    }
    uint16_t index = 0;
    p[index++] = 0;
    p[index++] = addr;//
    
    uint8_t i03t_addr = addr - 1;
    
    p[index++] = ConfigDataId_TotalVoltageSamplePeriod;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sample_period.period_total_volt >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sample_period.period_total_volt;
    
    p[index++] = ConfigDataId_CellPullPeriod;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sample_period.period_cell_poll >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sample_period.period_cell_poll;
         
    p[index++] = ConfigDataId_ResSamplePeriod;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sample_period.period_int_res >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sample_period.period_int_res;
    
    p[index++] = ConfigDataId_CellNumber;     
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sys_para.cell_number >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sys_para.cell_number;
    
    p[index++] = ConfigDataId_CriticalCurrentLimits; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].charge_status_threshold.critical_current >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].charge_status_threshold.critical_current;
    
    p[index++] = ConfigDataId_FastToFloat;       
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].charge_status_threshold.fast_to_float >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].charge_status_threshold.fast_to_float;
    
    p[index++] = ConfigDataId_FloatToFast;   
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].charge_status_threshold.float_to_fast >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].charge_status_threshold.float_to_fast;
    
    p[index++] = ConfigDataId_ChargeToDischarge;   
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].charge_status_threshold.charge_to_discharge >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].charge_status_threshold.charge_to_discharge;    
    
    p[index++] = ConfigDataId_DrySwitch;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.dry_switch;        
    
    p[index++] = ConfigDataId_DryOutputThreshold;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.out_threshold >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.out_threshold;    
    
    p[index++] = ConfigDataId_DryResumeThreshold;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.output_resume_threshold >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.output_resume_threshold;
    
    p[index++] = ConfigDataId_DryDelay;          
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.delay;
    
    p[index++] = ConfigDataId_DryOutputMinVoltage;   
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.output_min_volt >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.output_min_volt;
    
    p[index++] = ConfigDataId_433WorkFreq;         
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].wireless.work_freq;

    p[index++] = ConfigDataId_433AssistFreq;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].wireless.assist_freq;    
    
    p[index++] = ConfigDataId_Protocol;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sys_para.protocol_type;  
    
    p[index++] = ConfigDataId_NominalCap; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.nominal_cap >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.nominal_cap;
    
    p[index++] = ConfigDataId_NominalVolt;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.nominal_volt;
    
    p[index++] = ConfigDataId_FloatVolt;   
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.float_volt >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.float_volt;
    
    p[index++] = ConfigDataId_TempCompensate;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.temp_compensate >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.temp_compensate;    
    
    p[index++] = ConfigDataId_DischargeMethod;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.discharge_method; 
    
    p[index++] = ConfigDataId_ChargeMethod;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.charge_method;     
    
    p[index++] = ConfigDataId_ChargeEfficiency;   
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.charge_efficiency >> 8;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.charge_efficiency;
    
    p[index++] = ConfigDataId_MaxDischargeCap; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.max_discharge_cap >> 8;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.max_discharge_cap;    
    
    p[index++] = ConfigDataId_CellMinVolt;   
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.cell_min_voltage >> 8;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.cell_min_voltage;    

    p[index++] = ConfigDataId_CellMaxVolt;    
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.cell_max_voltage >> 8;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.cell_max_voltage;  
    
    p[index++] = ConfigDataId_DischargeMaxCurrent;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.discharge_max_current >> 8;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.discharge_max_current;  
    
    p[index++] = ConfigDataId_DischargeMinCurrent;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.discharge_min_current >> 8;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.discharge_min_current;  
    
    p[index++] = ConfigDataId_MinCalculateTemp;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.min_calculate_temp >> 8;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.min_calculate_temp;  
    
    p[index++] = ConfigDataId_MaxCalculateTemp; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.max_calculate_temp >> 8;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.max_calculate_temp;      
    
    p[index++] = ConfigDataId_CapInitFlg; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.cap_init; 
    
    p[index++] = ConfigDataId_CurrentGroupNumber;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].cap.current_group_number;

    p[index++] = ConfigDataId_TotalVoltageThreshold;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.over.level_3_delay; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_voltage.low.level_3_delay; 
    
    
    p[index++] = ConfigDataId_ChargeOverCurrentThreshold;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_charge.level_3_delay; 
    

    p[index++] = ConfigDataId_DischargeOverCurrentThreshold;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.total_current.over_discharge.level_3_delay; 
    
    p[index++] = ConfigDataId_TempOverThreshold;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.temp.over.level_3_delay;     
    
    p[index++] = ConfigDataId_SocLowThreshold; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.soc.low.level_3_delay;    
    
    p[index++] = ConfigDataId_CellVoltOverThreshold;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.over.level_3_delay; 
    
    p[index++] = ConfigDataId_CellVoltLowThreshold;   
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.voltage.low.level_3_delay; 
    
    p[index++] = ConfigDataId_CellTempOverThreshold; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.temp.over.level_3_delay;     
    
    p[index++] = ConfigDataId_CellInterResOverThreshold; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.cell.res.over.level_3_delay;
    
    p[index++] = ConfigDataId_LeakThreshold; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_1 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_1;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_2 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_2;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_3 >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_3;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_1_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_1_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_2_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_2_resume;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_3_resume >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_3_resume; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_1_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_1_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_2_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_2_delay;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_3_delay >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].alarm_threshold.leak.leak.level_3_delay;  


    p[index++] = ConfigDataId_AlarmCorrelation;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.alarm_correlation.alarm_correlation >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.alarm_correlation.alarm_correlation;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.alarm_correlation.alarm_correlation_group >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.alarm_correlation.alarm_correlation_group;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.alarm_correlation.alarm_correlation_cells >> 8;
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].dry_node.alarm_correlation.alarm_correlation_cells;


    p[index++] = ConfigDataId_VoltageLevel;    
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sys_para.voltage_level;
    
    p[index++] = ConfigDataId_CurrentLevel;       
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sys_para.current_level;
    
    p[index++] = ConfigDataId_CellSNLength; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sys_para.sn_length;  
    
    p[index++] = ConfigDataId_AlarmMaxLevel;  
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sys_para.alarm_max_level;
    
    p[index++] = ConfigDataId_CellWorkMode; 
    p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].sys_para.cell_mode;    

    if(query) {
        p[index++] = ConfigDataId_VoltageCalib;  
        p[index++] = 0;//total voltage
        p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].calib.total_voltage.zero_offset >> 8;
        p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].calib.total_voltage.zero_offset;
        Convt_t conv = {0};
        conv.f = pDEApp->device_config.i03t_nodes[i03t_addr].calib.total_voltage.kb.k;
        p[index++] = conv.u >> 24;
        p[index++] = conv.u >> 16;
        p[index++] = conv.u >> 8;
        p[index++] = conv.u;
        conv.f = pDEApp->device_config.i03t_nodes[i03t_addr].calib.total_voltage.kb.b;
        p[index++] = conv.u >> 24;
        p[index++] = conv.u >> 16;
        p[index++] = conv.u >> 8;
        p[index++] = conv.u; 
        
        for(uint8_t i=0;i<CONFIG_MAX_CURRENT_CH;i++){
            p[index++] = ConfigDataId_CurrentCalib; 
            p[index++] = 1+i;//
            p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].calib.current[i].zero_offset >> 8;
            p[index++] = pDEApp->device_config.i03t_nodes[i03t_addr].calib.current[i].zero_offset;
            conv.f = pDEApp->device_config.i03t_nodes[i03t_addr].calib.current[i].kb.k;
            p[index++] = conv.u >> 24;
            p[index++] = conv.u >> 16;
            p[index++] = conv.u >> 8;
            p[index++] = conv.u;
            conv.f = pDEApp->device_config.i03t_nodes[i03t_addr].calib.current[i].kb.b;
            p[index++] = conv.u >> 24;
            p[index++] = conv.u >> 16;
            p[index++] = conv.u >> 8;
            p[index++] = conv.u;   
        }            
        
        p[index++] = ConfigDataId_TempCalib; 
        conv.f = pDEApp->device_config.i03t_nodes[i03t_addr].calib.temp.kb.k;
        p[index++] = conv.u >> 24;
        p[index++] = conv.u >> 16;
        p[index++] = conv.u >> 8;
        p[index++] = conv.u;
        conv.f = pDEApp->device_config.i03t_nodes[i03t_addr].calib.temp.kb.b;
        p[index++] = conv.u >> 24;
        p[index++] = conv.u >> 16;
        p[index++] = conv.u >> 8;
        p[index++] = conv.u;    
        
        p[index++] = ConfigDataId_Volt5VCalib;
        conv.f = pDEApp->device_config.i03t_nodes[i03t_addr].calib.voltage_5v.kb.k;
        p[index++] = conv.u >> 24;
        p[index++] = conv.u >> 16;
        p[index++] = conv.u >> 8;
        p[index++] = conv.u;
        conv.f = pDEApp->device_config.i03t_nodes[i03t_addr].calib.voltage_5v.kb.b;
        p[index++] = conv.u >> 24;
        p[index++] = conv.u >> 16;
        p[index++] = conv.u >> 8;
        p[index++] = conv.u;      
    }

    return index;
}

uint16_t fill_query_config_i03m(uint8_t *p) {
    uint16_t index = 0;
    p[index++] = 0;
    p[index++] = 0;//i03M addr
    
//  p[index++] = ConfigDataId_BaudRate;
//  p[index++] = pDEApp->device_config.i03m.baudrate.bits.baudrate;
//  p[index++] = pDEApp->device_config.i03m.baudrate.bits.data_bits;
//  p[index++] = pDEApp->device_config.i03m.baudrate.bits.stop_bits;
//  p[index++] = pDEApp->device_config.i03m.baudrate.bits.check_bits;
    
//    p[index++] = ConfigDataId_Protocol;
//    p[index++] = pDEApp->device_config.i03m.protocol_type;  
//    
    for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
        p[index++] = ConfigDataId_PortCfg;
        p[index++] = i + 1;
        p[index++] = pDEApp->device_config.i03m.rs485_cfg[i].port_type;
        p[index++] = pDEApp->device_config.i03m.rs485_cfg[i].protocol;
        p[index++] = pDEApp->device_config.i03m.rs485_cfg[i].baudrate.bits.baudrate;
        p[index++] = pDEApp->device_config.i03m.rs485_cfg[i].baudrate.bits.data_bits;
        p[index++] = pDEApp->device_config.i03m.rs485_cfg[i].baudrate.bits.stop_bits;
        p[index++] = pDEApp->device_config.i03m.rs485_cfg[i].baudrate.bits.check_bits;
    }
    
    p[index++] = ConfigDataId_AlarmCorrelation;
    p[index++] = pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation >> 8;
    p[index++] = pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation;
    p[index++] = pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation_group >> 8;
    p[index++] = pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation_group;
    p[index++] = pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation_cells >> 8;
    p[index++] = pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation_cells;

    p[index++] = ConfigDataId_StorePeriod;
    p[index++] = pDEApp->device_config.i03m.storage_period >> 8;
    p[index++] = pDEApp->device_config.i03m.storage_period;
    
    p[index++] = ConfigDataId_CloudInfo;
    p[index++] = pDEApp->device_config.i03m.mqtt_config.cloud_enable;
    
    p[index++] = pDEApp->device_config.i03m.mqtt_config.i03t_report_interval >> 8;
    p[index++] = pDEApp->device_config.i03m.mqtt_config.i03t_report_interval;
    p[index++] = pDEApp->device_config.i03m.mqtt_config.cells_report_interval >> 8;
    p[index++] = pDEApp->device_config.i03m.mqtt_config.cells_report_interval;
    
    memcpy(p+index,pDEApp->device_config.i03m.mqtt_config.mqtt_addr,64);
    index += 64;
    
    p[index++] = pDEApp->device_config.i03m.mqtt_config.mqtt_port >> 8;
    p[index++] = pDEApp->device_config.i03m.mqtt_config.mqtt_port;  
    memcpy(p+index,pDEApp->device_config.i03m.mqtt_config.user_name,64);
    index += 64;
    
    memcpy(p+index,pDEApp->device_config.i03m.mqtt_config.pass_word,32);
    index += 32;
    
    p[index++] = ConfigDataId_4GModuleType;
    p[index++] = pDEApp->device_config.i03m.module_type;
    
    p[index++] = ConfigDataId_SetLocalIpV4;
    memcpy(p+index,pDEApp->device_config.i03m.net_config.ipv4.ip,16);
    index += 16;
    memcpy(p+index,pDEApp->device_config.i03m.net_config.ipv4.mask,16);
    index += 16;
    memcpy(p+index,pDEApp->device_config.i03m.net_config.ipv4.gate,16);
    index += 16;
    
    p[index++] = ConfigDataID_DNSServer;
    memcpy(p+index,pDEApp->device_config.i03m.net_config.dns,40);
    index += 40;
    
    
    return index;
}

uint16_t fill_query_config(uint8_t addr,uint8_t *p) { 
    if(addr > CONFIG_MAX_IO3T_MODBUS_ADDR) return 0;
    
    if(!addr) {
        return fill_query_config_i03m(p);
    } else {
        return fill_query_config_i03t(addr,p,1);
    }
}

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
uint16_t fill_query_logger( COMM_TYPE_t commType,
                            uint8_t dest,
                            uint8_t ser,
                            uint8_t i03t_addr,
                            uint8_t data_id,
                            uint32_t start_time,
                            uint32_t end_time,
                            uint8_t *p) {
    uint16_t index = 0;
    
    I03T_Info_t *i03t = i03t_node_find(i03t_addr);
    if(i03t == NULL) {
        return 0;
    }
                               
    switch(data_id) {
        case DataId_Alarm_Header: 
            {
                uint32_t max_cnt = i03t->alarm.index > FILE_ALARM_DATA_MAX_CNT ? FILE_ALARM_DATA_MAX_CNT : i03t->alarm.index;
                p[index++] = 0x00; //addr
                p[index++] = i03t_addr;
                
                p[index++] = DataId_Alarm_Header;
                p[index++] = max_cnt >> 24;
                p[index++] = max_cnt >> 16;
                p[index++] = max_cnt >> 8;
                p[index++] = max_cnt;
                
                p[index++] = i03t->alarm.index >> 24;
                p[index++] = i03t->alarm.index >> 16;
                p[index++] = i03t->alarm.index >> 8;
                p[index++] = i03t->alarm.index;
            }
            break;
        case DataId_Hist_Header:
            {
                uint32_t max_cnt = i03t->hist.index > FILE_HIST_DATA_MAX_CNT ? FILE_HIST_DATA_MAX_CNT : i03t->hist.index;
                p[index++] = 0x00; //addr
                p[index++] = i03t_addr;
                
                p[index++] = DataId_Hist_Header;
                p[index++] = max_cnt >> 24;
                p[index++] = max_cnt >> 16;
                p[index++] = max_cnt >> 8;
                p[index++] = max_cnt;
                
                p[index++] = i03t->hist.index >> 24;
                p[index++] = i03t->hist.index >> 16;
                p[index++] = i03t->hist.index >> 8;
                p[index++] = i03t->hist.index;
            }
            break;
            
        case DataId_Discharge_Header: {
                uint32_t max_cnt = i03t->discharge.index > FILE_DISCHARGE_DATA_MAX_CNT ? FILE_DISCHARGE_DATA_MAX_CNT : i03t->discharge.index;
                p[index++] = 0x00; //addr
                p[index++] = i03t_addr;
                
                p[index++] = DataId_Discharge_Header;
                p[index++] = max_cnt >> 24;
                p[index++] = max_cnt >> 16;
                p[index++] = max_cnt >> 8;
                p[index++] = max_cnt;
                
                p[index++] = i03t->discharge.index >> 24;
                p[index++] = i03t->discharge.index >> 16;
                p[index++] = i03t->discharge.index >> 8;
                p[index++] = i03t->discharge.index;
            }
            break;

        
        case DataId_Alarm_Data:
            {
                LoggerQuery_t logger = {0};
                logger.start.date = start_time;
                logger.end.date = end_time;
                logger.commType = commType;
                logger.requestMsgId = ser & 0x3F;
                logger.dest = dest;
                logger.print = 0;
                storage_msg_put(i03t_addr,(uint8_t *)&logger,sizeof(LoggerQuery_t),StorageAlarmRead);
            }
            return 0;

        case DataId_Hist_Data:
            {
                LoggerQuery_t logger = {0};
                logger.start.date = start_time;
                logger.end.date = end_time;
                logger.commType = commType;
                logger.requestMsgId = ser & 0x3F;
                logger.dest = dest;
                logger.print = 0;
                storage_msg_put(i03t_addr,(uint8_t *)&logger,sizeof(LoggerQuery_t),StorageHistRead);
            }
            return 0;

        case DataId_Discharge_Data:
            {
                LoggerQuery_t logger = {0};
                logger.start.date = start_time;
                logger.end.date = end_time;
                logger.commType = commType;
                logger.requestMsgId = ser & 0x3F;
                logger.dest = dest;
                storage_msg_put(i03t_addr,(uint8_t *)&logger,sizeof(LoggerQuery_t),StorageDischargeRead);
            }
            return 0;
            
        default:
            break;
    }
    return  index;
}

bool appl_para_find_i03t_cloud_id(char *cloud_id);
void module_4g_msg_put(ACTION_TYPE_t action,uint8_t *pdata,uint16_t length);
void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length);
void soc_msg_put(uint8_t i03t_addr,uint8_t command);
void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
errStatus_t de_protocol_handle_config(uint8_t *pdata,uint16_t length) {
    #define MAX_STORE_SN_NUM 100
    uint16_t index = 0;

	int result_cnt = 0;

	uint8_t err_count = 0;
    uint8_t data_id = 0;

    int16_t target_addr = 0;
    
    SNStore_t *sn_store = NULL; 
    uint16_t sn_store_cnt = 0;
    
    bool add_sn = false;
    bool busy_flg = false;
    
    while(index < length) {
        data_id = pdata[index++];
        switch(data_id) {
            case 0:
                target_addr = pdata[index++];
                if(target_addr > CONFIG_MAX_IO3T_MODBUS_ADDR) {
                    err_count++;
                }
                break;
                
            case ConfigDataId_BaudRate:
                index += 4;
//                pDEApp->device_config.i03m.baudrate.bits.baudrate = pdata[index++];
//                pDEApp->device_config.i03m.baudrate.bits.data_bits = pdata[index++];
//                pDEApp->device_config.i03m.baudrate.bits.stop_bits = pdata[index++];
//                pDEApp->device_config.i03m.baudrate.bits.check_bits = pdata[index++];
//                if(pDEApp->device_config.i03m.baudrate.bits.baudrate > 5) {
//                    pDEApp->device_config.i03m.baudrate.bits.baudrate = 5;
//                }
//                if(pDEApp->device_config.i03m.baudrate.bits.check_bits > 2) {
//                    pDEApp->device_config.i03m.baudrate.bits.check_bits = 0;
//                }
//                command_msg_put(SysCmdSetBaudRate,NULL,0);
                break;
            
            case ConfigDataId_TotalVoltageSamplePeriod:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_total_volt = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(!limits(pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_total_volt,1,1000)) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_total_volt = 1;
                }
                index += 2;
                break;
            case ConfigDataId_CellPullPeriod:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_cell_poll = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(!limits(pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_cell_poll,10,1000)) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_cell_poll = 30;
                }
                index += 2;
                break;
            case ConfigDataId_ResSamplePeriod:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_int_res = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(!limits(pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_int_res,5,32000)) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].sample_period.period_int_res = 7200;
                }
                index += 2;
                break; 
                
            case ConfigDataId_CellNumber:
                if(target_addr < 1) {err_count++;break;} 
                {
                    uint16_t temp_num = (uint16_t)pdata[index] << 8 | pdata[index+1];
                    if(temp_num != pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.cell_number) {
                        pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.cell_number = temp_num;
                        
                        I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                        if(i03t_node == NULL) {
                            err_count++;
                            break;
                        } 
                        
                        i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
                        i03t_node->flag.bits.synch_sn_force = 1;
                    }
                    
                    if(pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.cell_number > CONFIG_MAX_CELL) {
                        pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.cell_number = CONFIG_MAX_CELL;
                    }
                }
                
                index += 2;
                break;
                
            case ConfigDataId_CriticalCurrentLimits:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.critical_current = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(!limits(pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.critical_current,5,50)) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.critical_current = 20;
                }
                index += 2;
                break;
                
            case ConfigDataId_FastToFloat:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.fast_to_float = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(!limits(pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.fast_to_float,10,100)) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.fast_to_float = 50;
                }
                index += 2;
                break;
                
            case ConfigDataId_FloatToFast:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.float_to_fast = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(!limits(pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.float_to_fast,20,200)) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.float_to_fast = 80;
                }
                index += 2;
                break;
                
            case ConfigDataId_ChargeToDischarge:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.charge_to_discharge = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(!limits(pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.charge_to_discharge,-50,-1)) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].charge_status_threshold.charge_to_discharge = -20;
                }
                index += 2;
                break;  
                
            case ConfigDataId_DrySwitch:
                if(target_addr == 0) {
                    
                } else {
                    pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.dry_switch = pdata[index];  
                    if(pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.dry_switch > 1 ) {
                        pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.dry_switch = 0;
                    }
                }
                index++;
                break;   
                
            case ConfigDataId_DryOutputThreshold:
                if(target_addr == 0) {
                   
                } else {
                    pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.out_threshold = (uint16_t)pdata[index] << 8 | pdata[index+1];     
                }
                index += 2; 
                break; 
                
            case ConfigDataId_DryResumeThreshold:
                if(target_addr == 0) {
                   
                } else {
                    pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.output_resume_threshold = (uint16_t)pdata[index] << 8 | pdata[index+1];     
                }
                index += 2; 
                break;   
            case ConfigDataId_DryDelay:
                if(target_addr == 0) {
                   
                } else {
                    pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.delay = (uint16_t)pdata[index] << 8 | pdata[index+1];     
                }
                index += 2; 
                break;             
            case ConfigDataId_DryOutputMinVoltage:
                if(target_addr == 0) {
                   
                } else {
                    pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.output_min_volt = (uint16_t)pdata[index] << 8 | pdata[index+1];     
                }
                index += 2; 
                break;  
            case ConfigDataId_433WorkFreq:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].wireless.work_freq = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].wireless.work_freq > 99) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].wireless.work_freq = 54;
                }
                break;          
            case ConfigDataId_433AssistFreq:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].wireless.assist_freq = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].wireless.assist_freq > 99) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].wireless.assist_freq  = 99;
                }
                break; 
                
            case ConfigDataId_Protocol:
                {
                    uint8_t protocol = pdata[index++];
                    if(PROTOCOL_TYPE_MAX <= protocol) {err_count++;break;}
                    if(target_addr == 0) {
                        pDEApp->device_config.i03m.protocol_type = protocol;
                    } else {
                        pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.protocol_type = protocol;     
                    }
                }
                
                break;
                
            case ConfigDataId_PortCfg://I03M
                {
                    uint8_t port_num = pdata[index++];
                    uint8_t port_type = pdata[index++];
                    uint8_t port_protocol = pdata[index++];
                    uint8_t port_baudrate = pdata[index++];
                    uint8_t port_databits = pdata[index++];
                    uint8_t port_stopbits = pdata[index++];
                    uint8_t port_checkbits = pdata[index++];
                    
                    if(port_num == 0 || port_num > CONFIG_MAX_485_PORT) {
                        {err_count++;break;}
                    }
                    if(port_type >= PORT_MAX) {
                        {err_count++;break;}
                    }
                    if(port_protocol >= PROTOCOL_TYPE_MAX) {
                        {err_count++;break;}
                    }
                    if(port_baudrate > BAUDRATE_115200) {
                        {err_count++;break;}
                    }
                    if(port_databits > DATA_BITS_9) {
                        {err_count++;break;}
                    }
                    if(port_stopbits > STOP_BITS_2) {
                        {err_count++;break;}
                    }
                    if(port_checkbits > CHECK_BIT_EVEN) {
                        {err_count++;break;}
                    }
                    
                    pDEApp->device_config.i03m.rs485_cfg[port_num-1].port_type = port_type;
                    pDEApp->device_config.i03m.rs485_cfg[port_num-1].protocol = port_protocol;
                    pDEApp->device_config.i03m.rs485_cfg[port_num-1].baudrate.all = 0;
                    pDEApp->device_config.i03m.rs485_cfg[port_num-1].baudrate.bits.baudrate = port_baudrate;
                    pDEApp->device_config.i03m.rs485_cfg[port_num-1].baudrate.bits.check_bits = port_checkbits;
                    pDEApp->device_config.i03m.rs485_cfg[port_num-1].baudrate.bits.stop_bits = port_stopbits;
                    pDEApp->device_config.i03m.rs485_cfg[port_num-1].baudrate.bits.data_bits = port_databits;
                }
                break;
                
            case ConfigDataId_NominalCap:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.nominal_cap = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].cap.nominal_cap == 0) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].cap.nominal_cap = 12000;
                }
                
                index += 2;
                break;  
                
            case ConfigDataId_NominalVolt:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.nominal_volt = pdata[index++];
                switch(pDEApp->device_config.i03t_nodes[target_addr-1].cap.nominal_volt) {
                    case 1:
                    case 3:
                    case 6:
                        break;
                    default:
                        pDEApp->device_config.i03t_nodes[target_addr-1].cap.nominal_volt = 1;
                        break;
                }
                break;   
                
            case ConfigDataId_FloatVolt:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.float_volt = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].cap.float_volt == 0) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].cap.float_volt = 2350;
                }
                index += 2;
                break;    
                
            case ConfigDataId_TempCompensate:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.temp_compensate = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                break;        
                
            case ConfigDataId_DischargeMethod:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.discharge_method = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].cap.discharge_method > 1) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].cap.discharge_method = 0;
                }
                break;             
                
            case ConfigDataId_ChargeMethod:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.charge_method = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].cap.charge_method > 1) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].cap.charge_method = 0;
                }
                break;            
                
            case ConfigDataId_ChargeEfficiency:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.charge_efficiency = (uint16_t)pdata[index] << 8 | pdata[index+1];
                
                index += 2;
                break;            
            case ConfigDataId_MaxDischargeCap:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.max_discharge_cap = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                break;             
            case ConfigDataId_CellMinVolt:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.cell_min_voltage = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].cap.cell_min_voltage == 0) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].cap.cell_min_voltage = 1800;
                }
                index += 2;
                break;                 
            case ConfigDataId_CellMaxVolt:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.cell_max_voltage = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].cap.cell_max_voltage == 0) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].cap.cell_max_voltage = 2400;
                }
                index += 2;
                break;                 
            case ConfigDataId_DischargeMaxCurrent:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.discharge_max_current = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                break;         
            case ConfigDataId_DischargeMinCurrent:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.discharge_min_current = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                break;         
            case ConfigDataId_MinCalculateTemp:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.min_calculate_temp = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                break;            
            case ConfigDataId_MaxCalculateTemp:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.max_calculate_temp = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                break;            
            case ConfigDataId_CapInitFlg:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.cap_init = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].cap.cap_init > 1) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].cap.cap_init = 1;
                }
                break; 

            case ConfigDataId_CurrentGroupNumber:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].cap.current_group_number = pdata[index++];
                if(!limits(pDEApp->device_config.i03t_nodes[target_addr-1].cap.current_group_number,1,CONFIG_MAX_CURRENT_CH)) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].cap.current_group_number  = 1;
                }
                break;                

            case ConfigDataId_TotalVoltageThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_voltage.low.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;

                break;   
            case ConfigDataId_ChargeOverCurrentThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_charge.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                break; 
                
            case ConfigDataId_DischargeOverCurrentThreshold:
                 if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.total_current.over_discharge.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                break;
                 
            case ConfigDataId_TempOverThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.temp.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                break;   
                
            case ConfigDataId_SocLowThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.soc.low.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                break; 
                
            case ConfigDataId_CellVoltOverThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;

                break;  
            
            case ConfigDataId_CellVoltLowThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.voltage.low.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                break;   
            
            case ConfigDataId_CellTempOverThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.temp.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;

                break; 
            
            case ConfigDataId_CellInterResOverThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.cell.res.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                break;
                
            case ConfigDataId_LeakThreshold:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_1 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_2 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_3 = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                pDEApp->device_config.i03t_nodes[target_addr-1].alarm_threshold.leak.leak.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index+1];index += 2;
                break;


            case ConfigDataId_AlarmCorrelation:
                if(target_addr == 0) {
                    pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation = (uint16_t)pdata[index] << 8 | pdata[index + 1];index += 2;
                    pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation_group = (uint16_t)pdata[index] << 8 | pdata[index + 1];index += 2;
                    pDEApp->device_config.i03m.dry_node.alarm_correlation.alarm_correlation_cells = (uint16_t)pdata[index] << 8 | pdata[index + 1];index += 2;
                } else {
                    pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.alarm_correlation.alarm_correlation = (uint16_t)pdata[index] << 8 | pdata[index + 1];index += 2;
                    pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.alarm_correlation.alarm_correlation_group = (uint16_t)pdata[index] << 8 | pdata[index + 1];index += 2;
                    pDEApp->device_config.i03t_nodes[target_addr-1].dry_node.alarm_correlation.alarm_correlation_cells = (uint16_t)pdata[index] << 8 | pdata[index + 1];index += 2;
                }

                break;
                
            case ConfigDataId_VoltageLevel:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.voltage_level = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.voltage_level > 3) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.voltage_level = 1;
                }
                break; 
                
            case ConfigDataId_CurrentLevel:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.current_level = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.current_level > 1) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.current_level = 0;
                }
                break;        
            case ConfigDataId_CellSNLength:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.sn_length = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.sn_length > 20) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.sn_length = 12;
                }
                break;      
                
            case ConfigDataId_AlarmMaxLevel:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.alarm_max_level = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.alarm_max_level > 3 ||
                    pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.alarm_max_level < 1) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.alarm_max_level = 2;
                }
                break;       
            case ConfigDataId_CellWorkMode:
                if(target_addr < 1) {err_count++;break;};
                pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.cell_mode = pdata[index++];
                if(pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.cell_mode > 1) {
                    pDEApp->device_config.i03t_nodes[target_addr-1].sys_para.cell_mode = 0;
                }
                break;        
            case ConfigDataId_VoltageCalib:
                if(target_addr < 1) {err_count++;break;};
                {
                    uint16_t ch = pdata[index++];
                    Convt_t conv = {0};
                    switch(ch) {
                        case 0:
                            pDEApp->device_config.i03t_nodes[target_addr-1].calib.total_voltage.zero_offset = (uint16_t)pdata[index] << 8 | pdata[index+1];
                            index += 2;

                            conv.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index+1] << 16 | (uint16_t)pdata[index+2] << 8 | pdata[index+3];
                            index += 4;
                            pDEApp->device_config.i03t_nodes[target_addr-1].calib.total_voltage.kb.k = conv.f;
                            
                            conv.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index+1] << 16 | (uint16_t)pdata[index+2] << 8 | pdata[index+3];
                            index += 4;
                            pDEApp->device_config.i03t_nodes[target_addr-1].calib.total_voltage.kb.b = conv.f;
                            break;
                        default:
                            break;
                    }
                }
                
                break;        
            case ConfigDataId_CurrentCalib:
                if(target_addr < 1) {err_count++;break;};
                {
                    uint16_t ch = pdata[index++];
                    if(ch > CONFIG_MAX_CURRENT_CH || ch < 1) {
                        err_count++;
                        break;
                    }
                    Convt_t conv = {0};
                    ch--;
                    
                    pDEApp->device_config.i03t_nodes[target_addr-1].calib.current[ch].zero_offset = (uint16_t)pdata[index] << 8 | pdata[index+1];
                    index += 2;
                   
                    
                    conv.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index+1] << 16 | (uint16_t)pdata[index+2] << 8 | pdata[index+3];
                    index += 4;
                    pDEApp->device_config.i03t_nodes[target_addr-1].calib.current[ch].kb.k = conv.f;
                    
                    conv.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index+1] << 16 | (uint16_t)pdata[index+2] << 8 | pdata[index+3];
                    index += 4;
                    pDEApp->device_config.i03t_nodes[target_addr-1].calib.current[ch].kb.b = conv.f;
                }
                break;        
            case ConfigDataId_TempCalib: 
                if(target_addr < 1) {err_count++;break;};
                {
                    Convt_t conv = {0};
                    conv.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index+1] << 16 | (uint16_t)pdata[index+2] << 8 | pdata[index+3];
                    index += 4;
                    pDEApp->device_config.i03t_nodes[target_addr-1].calib.temp.kb.k = conv.f;
                    
                    conv.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index+1] << 16 | (uint16_t)pdata[index+2] << 8 | pdata[index+3];
                    index += 4;
                    pDEApp->device_config.i03t_nodes[target_addr-1].calib.temp.kb.b = conv.f;
                }
                break;           
            case ConfigDataId_Volt5VCalib:
                if(target_addr < 1) {err_count++;break;}
                {
                    Convt_t conv = {0};

                    conv.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index+1] << 16 | (uint16_t)pdata[index+2] << 8 | pdata[index+3];
                    index += 4;
                    pDEApp->device_config.i03t_nodes[target_addr-1].calib.voltage_5v.kb.k = conv.f;
                    
                    conv.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index+1] << 16 | (uint16_t)pdata[index+2] << 8 | pdata[index+3];
                    index += 4;
                    pDEApp->device_config.i03t_nodes[target_addr-1].calib.voltage_5v.kb.b = conv.f;
                }
                break; 
            case ConfigDataId_AddSN:  {
#if 1
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node == NULL) {
                        err_count++;
                        break;
                    }
                    
                    SNStore_t sn_info = {0};
                    sn_info.cell_id = (uint16_t)pdata[index] << 8 | pdata[index+1];
                    index += 2;
                    
                    sn_info.cell_on_current_group = pdata[index++];
                    
                    memcpy(sn_info.sn,pdata + index,CONFIG_SN_LENGTH);
                    index += 20;
                    sn_info.i03t_addr = target_addr;
                    
                    if(!busy_flg) {
                        busy_flg = true; 
                        osDelay(200);
                    }
                    
                    if(module_sn_add(&sn_info) != errOK) {
                        err_count++;
                    }
                    if(module_sn_find(sn_info.sn) != errOK) {
                        err_count++;
                    }

                    module_sn_get_count(0,false);
                    
                    add_sn = true;
                    sn_store_cnt++;
                    

                    i03t_node->flag.bits.synch_sn_need = 1; 
                    i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
#else
                   if(sn_store == NULL) {
                        sn_store = sys_malloc(sizeof(SNStore_t) * MAX_STORE_SN_NUM);
                        if(sn_store == NULL) {

							err_count++;
                            break;
                        }
                    }
                    sn_store[sn_store_cnt].cell_id = (uint16_t)pdata[index] << 8 | pdata[index+1];
                    index += 2;
                    memcpy(sn_store[sn_store_cnt].sn,pdata + index,CONFIG_SN_LENGTH);
                    index += 20;
                    sn_store[sn_store_cnt].i03t_addr = target_addr;                  
                    sn_store_cnt++;                  
#endif
                    //
                }
                break;
                
            case ConfigDataId_AddSNCloudId:
                {
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node == NULL) {
                        err_count++;
                        break;
                    }
                    
                    SNStore_t sn_info = {0};
                    sn_info.cell_id = (uint16_t)pdata[index] << 8 | pdata[index+1];
                    index += 2;
                    
                    sn_info.cell_on_current_group = pdata[index++];
                    
                    memcpy(sn_info.sn,pdata + index,CONFIG_SN_LENGTH);
                    index += CONFIG_SN_LENGTH;
                    sn_info.i03t_addr = target_addr;
                    
                    memcpy(sn_info.cloud_id,pdata + index,CONFIG_CLOUD_ID_LENGTH);
                    index += CONFIG_CLOUD_ID_LENGTH;
                    
                    if(!busy_flg) {
                        busy_flg = true; 
                        osDelay(200);
                    }
                    
                    if(module_sn_add(&sn_info) != errOK) {
                        err_count++;
                    }
                    if(module_sn_find(sn_info.sn) != errOK) {
                        err_count++;
                    }

                    module_sn_get_count(0,false);
                    
                    add_sn = true;
                    sn_store_cnt++;
                    

                    i03t_node->flag.bits.synch_sn_need = 1; 
                    i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
                }
                break;
            case ConfigDataId_DelSN: 
                {
                    SNStore_t sn_info = {0};
                    memcpy(sn_info.sn,pdata + index,CONFIG_SN_LENGTH);
                    index += 20;
                    sn_info.i03t_addr = target_addr;
                    if(module_sn_delete(&sn_info) != errOK) {
                        err_count++;
                    }
                    
                    if(module_sn_find(sn_info.sn) == errOK) {
                        err_count++;
                    }
                    
                    module_sn_get_count(0,false);
                }
                break;    
                
            case ConfigDataId_DelAllSN:
                {
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node == NULL) {
                        err_count++;
                        break;
                    }
                    module_sn_clear(target_addr);
                    module_sn_get_count(0,false);
                    i03t_node->flag.bits.synch_sn_need = 1; 
                    i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
                    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_DEL_ALL_SN,target_addr,NULL,__FILE__,__LINE__);

                }
                index += 20;
                break;              
            case ConfigDataId_NewSN: {
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node == NULL) {
                        err_count++;
                        break;
                    }
                    
                    uint8_t sn_new[CONFIG_SN_LENGTH] = {0};
                    uint8_t sn_old[CONFIG_SN_LENGTH] = {0};
                    memcpy(sn_old,pdata + index,CONFIG_SN_LENGTH);
                    index += 20;
                    memcpy(sn_new,pdata + index,CONFIG_SN_LENGTH);
                    index += 20;
                    if(module_sn_replace(target_addr,sn_new,sn_old) != errOK) {
                        err_count++;
                    }
                    if(module_sn_find(sn_new) == errOK) {
                        err_count++;
                    }
                    
                    module_sn_get_count(0,false);
                    
                    i03t_node->flag.bits.synch_sn_need = 1; 
                    i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
                }         
                break;               
                            
            case ConfigDataId_Time: 
                {
                    DATE_yymmddhhmmss_t time = {0};
                    time.bits.year = pdata[index++];
                    time.bits.month = pdata[index++];
                    time.bits.day = pdata[index++];
                    time.bits.hour = pdata[index++];
                    time.bits.min = pdata[index++];
                    time.bits.sec = pdata[index++];
                    
                    Bsp_RtcSetTime(&time);
                    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,time.date,NULL,__FILE__,__LINE__);
                }  
                break;          
            
            case ConfigDataId_StorePeriod:
                pDEApp->device_config.i03m.storage_period = (uint16_t)pdata[index] << 8 | pdata[index+1];
                if(pDEApp->device_config.i03m.storage_period < 10) {
                    pDEApp->device_config.i03m.storage_period = 300;
                }
                index += 2;
                break;  
            
            case ConfigDataId_Address: 
                pDEApp->device_config.i03m.i03m_addr = pdata[index++];
                break;          
                
            case ConfigDataId_AddI03T:
                {
                    uint8_t i03t_addr = pdata[index++];
                    uint8_t mount = pdata[index++];
                    uint8_t cloud_id[CONFIG_CLOUD_ID_LENGTH + 1] = {0};
                    
                    memcpy(cloud_id,pdata + index, CONFIG_CLOUD_ID_LENGTH);
                    index += CONFIG_CLOUD_ID_LENGTH;
                    
                    if(i03t_addr > CONFIG_MAX_IO3T_MODBUS_ADDR || i03t_addr == 0) {err_count++;break;}
                    if(mount > CONFIG_MAX_IO3T_MODBUS_ADDR) {err_count++;break;}
                    if(mount > 0) {
                        if(i03t_node_find(mount) == NULL) {err_count++;break;}
                    }
                    if(i03t_node_find(i03t_addr) != NULL) {err_count++;break;}
                    
                    if(cloud_id[0] != 0x00) {
                        if(appl_para_find_i03t_cloud_id((char *)cloud_id)) {
                            err_count++;
                            break;
                        }
                    }

                    memset(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,0,sizeof(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id));
                    memcpy(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,cloud_id, CONFIG_CLOUD_ID_LENGTH);

                    pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.i03t_addr = i03t_addr;
                    pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.mount = mount;
                    if(i03t_node_find(i03t_addr) == NULL) {
                        i03t_node_add_addr(i03t_addr,mount);
                        I03T_Info_t *i03t = (I03T_Info_t *)i03t_node_find(i03t_addr);
                        if(i03t != NULL) {
                            
                            i03t->discharge.soc = 1000;
                            i03t->discharge.soh = 1000;
                            i03t->discharge.index = 0;
                            i03t->charge.index = 0;
                            i03t->alarm.index = 0;
                            i03t->hist.index = 0;
                            i03t->flag.all = 0;
                        }
                        storage_msg_put(i03t_addr,NULL,0,StorageFileCheck);
                        logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_ADD_I03T,i03t_addr,NULL,__FILE__,__LINE__);

                    }
                    i03t_node_update();
                }
                break;
            case ConfigDataId_DelI03T:{
                    void FileIndexClear(uint8_t i03t_addr);
                    uint8_t i03t_addr = pdata[index++];
                    if(i03t_addr > CONFIG_MAX_IO3T_MODBUS_ADDR || i03t_addr == 0) {err_count++;break;};
                    
                    pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.i03t_addr = 0;
                    pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.mount = 0;
                    memset(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,0,sizeof(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id));
                    module_sn_clear(i03t_addr);
					module_sn_get_count(0, false);
                    i03t_node_remove_addr(i03t_addr);
                    storage_msg_put(i03t_addr,NULL,0,StorageFileFlush);
                    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_DEL_I03T,i03t_addr,NULL,__FILE__,__LINE__);

                    FileIndexClear(i03t_addr);
                    i03t_node_update();
                    soc_msg_put(i03t_addr,I03T_DELETE);
                }
                break;
            case ConfigDataId_SetSoc:
                {
                    if(target_addr < 1) {err_count++;break;};
                    
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node == NULL) {
                        err_count++;
                        break;
                    }
                    
                    uint16_t temp =  (uint16_t)pdata[index] << 8 | pdata[index+1];
                    if(temp > 1000) {
                        temp = 1000;
                    }
                    i03t_node->discharge.soc = temp;
                    index += 2;
                }
                break;
                
            case ConfigDataId_SetSoh:
                {
                    if(target_addr < 1) {err_count++;break;};
                    
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node == NULL) {
                        err_count++;
                        break;
                    }
                    
                    uint16_t temp =  (uint16_t)pdata[index] << 8 | pdata[index+1];
                    if(temp > 1000) {
                        temp = 1000;
                    }
                    
                    
                    i03t_node->discharge.soh = temp;
                    
                    index += 2;
                    
                    pDEApp->device_config.i03t_nodes[target_addr-1].base_para.bat_life = pdata[index++];
                    if(pDEApp->device_config.i03t_nodes[target_addr-1].base_para.bat_life == 0) {
                        pDEApp->device_config.i03t_nodes[target_addr-1].base_para.bat_life = 10;
                    }
                    i03t_node->discharge.deep_discharge_cycle = (uint16_t)pdata[index] << 8 | pdata[index+1];
                    index += 2;
                    if(i03t_node->discharge.deep_discharge_cycle == 0) {
                        i03t_node->discharge.deep_discharge_cycle = CONFIG_DEEP_DISCHARGE_CYCLE;
                    }
                    soc_msg_put(target_addr,I03T_SOH_INIT);
                }
                break;
                
            case ConfigDataId_CloudInfo:
                pDEApp->device_config.i03m.mqtt_config.cloud_enable = pdata[index++];
                
                pDEApp->device_config.i03m.mqtt_config.i03t_report_interval = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                if(pDEApp->device_config.i03m.mqtt_config.i03t_report_interval < (MODULE_4G_I03T_TX_CYCLE / 1000)) {
                    pDEApp->device_config.i03m.mqtt_config.i03t_report_interval = MODULE_4G_I03T_TX_CYCLE / 1000;
                }
                
                pDEApp->device_config.i03m.mqtt_config.cells_report_interval = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                if(pDEApp->device_config.i03m.mqtt_config.cells_report_interval < (MODULE_4G_CELL_TX_CYCLE / 1000)) {
                    pDEApp->device_config.i03m.mqtt_config.cells_report_interval = MODULE_4G_CELL_TX_CYCLE / 1000;
                }

                memset(pDEApp->device_config.i03m.mqtt_config.mqtt_addr,0,sizeof(pDEApp->device_config.i03m.mqtt_config.mqtt_addr));
                memcpy(pDEApp->device_config.i03m.mqtt_config.mqtt_addr,pdata + index, 64);
                index += 64;
                pDEApp->device_config.i03m.mqtt_config.mqtt_port = (uint16_t)pdata[index] << 8 | pdata[index+1];
                index += 2;
                memset(pDEApp->device_config.i03m.mqtt_config.user_name,0,sizeof(pDEApp->device_config.i03m.mqtt_config.user_name));
                memcpy(pDEApp->device_config.i03m.mqtt_config.user_name,pdata + index, 64);
                index += 64;
                memset(pDEApp->device_config.i03m.mqtt_config.pass_word,0,sizeof(pDEApp->device_config.i03m.mqtt_config.pass_word));
                memcpy(pDEApp->device_config.i03m.mqtt_config.pass_word,pdata + index, 32);
                index += 32;
                break;
            
            case ConfigDataId_4GModuleType:
                pDEApp->device_config.i03m.module_type = pdata[index++];
                if(pDEApp->device_config.i03m.module_type >= MODULE_4G_TYPE_MAX
                  || pDEApp->device_config.i03m.module_type == MODULE_TYPE_NONE) {
                    pDEApp->device_config.i03m.module_type = MODULE_4G_ZLG;
                }
                module_4g_msg_put(ACTION_TYPE_REGISTER,NULL,0);
                break;
                
            case ConfigDataId_SynchI03TTime:
                pDEApp->device_config.i03m.i03t_time_synch = pdata[index++];
                break;
            
            case ConfigDataId_SetLocalIpV4:
                {
                    memset(&pDEApp->device_config.i03m.net_config.ipv4,0,sizeof(pDEApp->device_config.i03m.net_config.ipv4));
                    memcpy(pDEApp->device_config.i03m.net_config.ipv4.ip,pdata + index,16); index += 16;
                    memcpy(pDEApp->device_config.i03m.net_config.ipv4.mask,pdata + index,16); index += 16;
                    memcpy(pDEApp->device_config.i03m.net_config.ipv4.gate,pdata + index,16); index += 16;
                }
                break;
            case ConfigDataID_DNSServer:
                memset(&pDEApp->device_config.i03m.net_config.dns,0,sizeof(pDEApp->device_config.i03m.net_config.dns));
                memcpy(pDEApp->device_config.i03m.net_config.dns,pdata + index,40);
                index += 40;
                break;
            
            default:
                err_count++;
                break;
        }
        
       
		result_cnt++;

		if(err_count) {
			break;
		}
    }
    
    if(sn_store != NULL) {
         if(sn_store_cnt > 0) {
            if(module_sn_add_mult(sn_store,sn_store_cnt) != errOK) {
                err_count++;
            } 
            
            module_sn_get_count(0,false);
        }
         
        sys_free(sn_store);
        sn_store = NULL;
    }
    
    
    if(add_sn) {
        char temp_buf[10] = {0};
        sprintf(temp_buf,"添加SN:%d",sn_store_cnt);
        logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_ADD_SN,target_addr,NULL,temp_buf,0);
        module_sn_get_count(0,false);
    } else {
        if(!err_count) {
            storage_msg_put(0,NULL,0,StorageParaInfor);
        }  

        if(target_addr > 0 && target_addr <= CONFIG_MAX_I03T) {
            I03T_Info_t *i03t_node = i03t_node_find(target_addr);
            if(i03t_node != NULL) {
                i03t_node->flag.bits.synch_config = 0;
            }
        }
    }
    
    errStatus_t appl_rs485_conflict_check(void);
    if(appl_rs485_conflict_check() != errOK) {
        return errErr;
    }
    
    return err_count ? errErr : errOK;
}

void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length);
errStatus_t de_protocol_handle_control(uint8_t *pdata,uint16_t length) {
    uint16_t index = 0;

	int result_cnt = 0;

	uint8_t err_count = 0;
    uint8_t data_id = 0;
    
    uint8_t target_addr = 0;

    while(index < length) {
        data_id = pdata[index++];
        switch(data_id) {
            case 0:
                target_addr = pdata[index++];
                break;
            case ControlDataId_Reset:
                if(target_addr == 0) {
                    command_msg_put(SysCmdReset,NULL,0);
                } else {
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node != NULL) {
                        i03t_node->flag.bits.i03t_reset = 1;
                    }
                }
                
                index++;
                break;
            case ControlDataId_Format:
                storage_msg_put(0,NULL,0,StorageFsFormat);
                index++;
                break;
            case ControlDataId_Factory:
                storage_msg_put(0,NULL,0,StorageParaFactory);
                index++;
                break;
            case ControlDataId_SampleIntRes:
                if(target_addr > 0 && target_addr <= CONFIG_MAX_I03T) {
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node != NULL) {
                        i03t_node->flag.bits.i03t_sample_res = 1;
                    }
                } else {
                    err_count++;
                }
                index++;
                break;
            case ControlDataId_ClearData: 
                if(target_addr > 0 && target_addr <= CONFIG_MAX_I03T) {
                    I03T_Info_t *i03t_node = i03t_node_find(target_addr);
                    if(i03t_node != NULL) {
                        uint8_t temp = pdata[index++];
                        switch(temp) {
                            case 1:
                                storage_msg_put(target_addr,NULL,0,StorageFileDeleteHist);
                                break;
                            case 2:
                                storage_msg_put(target_addr,NULL,0,StorageFileDeleteAlarm);
                                break;
                            case 3:
                                storage_msg_put(target_addr,NULL,0,StorageFileDeleteDischarge);
                                break;
                            default:
                                err_count++;
                                break;
                        }
                    }
                } else {
                    err_count++;
                }
                break;
            case ControlDataId_UDiskExport:
                {
                    uint8_t temp = pdata[index++];
                    if(!pDEApp->Flag.bits.usb_con) {
                        err_count ++;
                        break;
                    }
                    
                    switch(temp) {
                        case 0:
                            storage_msg_put(target_addr,NULL,0,StorageUDiskExportAll);
                            break;
                        case 1:
                            storage_msg_put(target_addr,NULL,0,StorageUDiskExportHist);
                            break;
                        case 2:
                            storage_msg_put(target_addr,NULL,0,StorageUDiskExportAlarm);
                            break;
                        case 3:
                            storage_msg_put(target_addr,NULL,0,StorageUDiskExportDischarge);
                            break;
                        case 4:
                            storage_msg_put(target_addr,NULL,0,StorageUDiskExportSN);
                            break;
                        default:
                            err_count++;
                            break;
                    }
                }
                break;
            case ControlDataId_UDiskUpgrade:
                index++;
                if(!pDEApp->Flag.bits.usb_con) {
                    err_count ++;
                    break;
                }
                storage_msg_put(0,NULL,0,StorageUDiskUpgrade);
                break;
            case ControlDataId_UDiskStatus:
                index++;
                if(!pDEApp->Flag.bits.usb_con) {
                    err_count ++;
                }
                break;
            default:
                err_count ++;
                break;
        }
        
        result_cnt++;

		if(err_count) {
			break;
		}
    }
    
    return err_count ? errErr : errOK;
}

void protocol_handle_control(uint8_t *pdata,uint16_t length,HandleResult *result,int *resultcnt) {
	
    int result_cnt = 0;

    errStatus_t  err = de_protocol_handle_control(pdata,length);

    if(err != errOK) {
        result[result_cnt].result = Result_MsgErr;
    } else {
        result[result_cnt].result = Result_OK;
    }
    result[result_cnt].DataId = 0;
    result[result_cnt].extLength = 0;

    result_cnt++;
    
	*resultcnt = result_cnt;

}

void protocol_handle_config(uint8_t *pdata,uint16_t length,HandleResult *result,int *resultcnt) {

	int result_cnt = 0;

    errStatus_t  err = de_protocol_handle_config(pdata,length);

    if(err != errOK) {
        result[result_cnt].result = Result_MsgErr;
    } else {
        result[result_cnt].result = Result_OK;
    }
    result[result_cnt].DataId = 0;
    result[result_cnt].extLength = 0;

    result_cnt++;
    
	*resultcnt = result_cnt;
}
//#include "app_iap.h"

void protocol_handle_upgrade(uint8_t *pdata,uint16_t length,HandleResult *result,int *resultcnt) {
	uint16_t index = 0;

	int result_cnt = 0;
	uint8_t err_count = 0;

	uint8_t *p = NULL;

    bool ret = true;
	
    {
		uint8_t data_id = pdata[index++];

		switch(data_id) {
			case DataId_Upgrade_Header:
                {
                    uint32_t file_size;
                    uint32_t file_sum;
                    uint16_t pack_size;
                    if(pdata[index++] != 0x55) {
                        ret = false;
                        break;
                    }
                    
                    if(pdata[index++] != 0xAA) {
                        ret = false;
                        break;
                    }

                    //file size
                    file_size =  (uint32_t)pdata[25] << 24 \
                                |(uint32_t)pdata[26] << 16 \
                                |(uint32_t)pdata[27] << 8 \
                                |(uint32_t)pdata[28];
                    
                    file_sum =   (uint32_t)pdata[29] << 24 \
                                |(uint32_t)pdata[30] << 16 \
                                |(uint32_t)pdata[31] << 8 \
                                |(uint32_t)pdata[32];
                    
                    pack_size = (uint16_t)pdata[33] << 8 \
                                |pdata[34];
                    
                    
                    if(IAP_Init(file_size,file_sum,pack_size,1) != errOK) {
                        ret = false;
                        break;
                    }
                    uint8_t *pack_data = sys_malloc(pack_size);
                    if(pack_data == NULL) {
                        ret = false;
                        break;
                    }
                    
                    memcpy(pack_data,pdata + 1,pack_size);
                    
                    IAP_Process(pack_data,pack_size,0);
                    
                    sys_free(pack_data);
                    
                }
				break;


			case DataId_Upgrade_FileData: 
                {
                    uint16_t pack_number = (uint16_t)pdata[index] << 8 | pdata[index+1];
                    index += 2;
                    uint16_t pack_size = (uint16_t)pdata[index] << 8 | pdata[index+1];
                    index += 2;
                    if(pack_size > IAP_GetPackSize()) {
                        ret = false;
                        break;
                    }
                    uint8_t *pack_data = sys_malloc(pack_size);
                    if(pack_data == NULL) {
                        ret = false;
                        break;
                    }
                    
                    memcpy(pack_data,pdata + index,pack_size);

                    errStatus_t err = IAP_Process(pack_data,pack_size,pack_number);
                    if(err != errOK) {
                        ret = false;
                    }
                    
                    sys_free(pack_data);
                }
                
				break;


			default:
				err_count++;
				break;

		}

		if(err_count > 0) {
			result[result_cnt].result = Result_MsgErr;
		} else {
			result[result_cnt].result = Result_OK;
		}
		result[result_cnt].DataId = data_id;
		result[result_cnt].extLength = 0;

		result_cnt++;
	}

	if(!ret) {
		result[result_cnt].result = Result_MsgErr;
		result[result_cnt].DataId = 0x00;
		result[result_cnt].extLength = 0;
		result_cnt++;
	}

	if(p != NULL) {
		sys_free(p);
		p = NULL;
	}
    
	*resultcnt = result_cnt;
}


