#include "de_modbus_protocol.h"

#include <stdbool.h>
#include <time.h>
#include "typedef.h"
#include "program.h"
#include "i03t_list.h"
#include "main.h"

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);

bool modbus_crc16_check(uint8_t *pdata,uint16_t length) {

	if(length < 8) return false;
	modbus_request_t *request = (modbus_request_t *)pdata;
	uint16_t crc16 = CheckCRCModBus2(pdata,length-2);
	if(crc16 != (request->crc16_hi << 8 | request->crc16_lo)) {
		return false;
	}
	return true;

}

bool modbus_crc16_check_conti_write(uint8_t *pdata,uint16_t length) {
    if(length < 11) return false;
    modbus_request_conti_write_t *request = (modbus_request_conti_write_t *)pdata;
    
    if(request->function != FUNCTION_WRTIE_REGS) {
        return false;
    }
    
    if(request->bytes != 2 * (request->regs_num_hi << 8 | request->regs_num_lo)) {
        return false;
    }
    
	uint16_t crc16 = CheckCRCModBus2(pdata,length-2);
    
    uint8_t bytes = request->bytes;
    if(length < 9 + bytes)  {
        return false;
    }
    
    if(crc16 != (request->data[bytes] << 8 | request->data[bytes + 1])) {
        return false;
    }
    
    return true;
    
}

uint16_t modbus_response_package(uint8_t errFlg,modbus_request_t *request,uint8_t *buffer,uint8_t *pdata,uint16_t length) {

	uint16_t crc16;
	uint16_t index = 0;
	buffer[index++] = request->address;//pDEApp->device_config.i03m.i03m_addr;  // addr
    
    if(errFlg) {
        buffer[index++] = request->function | 0x80;
    } else {
        buffer[index++] = request->function;
    }
	

	memcpy(buffer+index,pdata,length);
	index += length;
	crc16 = CheckCRCModBus2(buffer,index);
	buffer[index++] = crc16 >> 8;
	buffer[index++] = crc16;

	return index;
}

uint16_t modbus_response_package_fun_03_04(uint8_t errFlg,modbus_request_t *request,uint8_t *buffer,uint8_t *pdata,uint16_t length) {

	uint16_t crc16;
	uint16_t index = 0;
	buffer[index++] = request->address;//pDEApp->device_config.i03m.i03m_addr;  // addr
    
    if(errFlg) {
        buffer[index++] = request->function | 0x80;
    } else {
        buffer[index++] = request->function;
    }
    
    buffer[index++] = length;
	

	memcpy(buffer+index,pdata,length);
	index += length;
	crc16 = CheckCRCModBus2(buffer,index);
	buffer[index++] = crc16 >> 8;
	buffer[index++] = crc16;

	return index;
}


typedef struct {
	uint8_t  funtion;
	uint16_t (*modbus_fun)(modbus_request_t *request,uint8_t *buffer);
}modbus_requests_t;



uint16_t modbus_read_do_handler(modbus_request_t *request,uint8_t *buffer) {

	uint16_t length = 0;
	uint16_t reg = request->reg_addr_hi << 8 | request->reg_addr_lo;
    switch(reg) {
    

    default:
    	break;
    }

    return length;
}

uint16_t modbus_read_di_handler(modbus_request_t *request,uint8_t *buffer) {
	uint16_t length = 0;
	uint16_t reg = request->reg_addr_hi << 8 | request->reg_addr_lo;
	switch(reg) {
        default: {
            break;

        }
    }

	return length;
}
void soc_msg_put(uint8_t i03t_addr,uint8_t command);
void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length);
errStatus_t modbus_set_i03t_rw_reg_value(uint8_t i03t_addr,uint16_t reg,uint16_t reg_value)  {
	
	I03T_Info_t *i03t_node = (I03T_Info_t *)i03t_node_find(i03t_addr);
	if(i03t_node == NULL) {
		return errErr;
	}
   
	uint8_t i03t_index = i03t_addr - 1;

	switch(reg) {
    case RWReg_TotalVoltageSamplePeriod: 
        pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_total_volt = reg_value;
        if(!limits(pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_total_volt,1,1000)) {
            pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_total_volt = 1;
        }
        break;
    case RWReg_CellPullPeriod: 
        pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_cell_poll = reg_value; 
        if(!limits(pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_cell_poll,10,1000)) {
            pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_cell_poll = 30;
        }
        break;          
    case RWReg_ResSamplePeriod: 
        pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_int_res = reg_value; 
        if(!limits(pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_int_res,5,32000)) {
            pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_int_res = 7200;
        }
        break;         
    case RWReg_CellNumber: 
        pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number > CONFIG_MAX_CELL) {
            pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number = CONFIG_MAX_CELL;
        }
        break;              
    case RWReg_CriticalCurrentLimits: 
        pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.critical_current = reg_value; 
        if(!limits(pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.critical_current,5,50)){
            pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.critical_current = 20;
        }
        break;   
    case RWReg_FastToFloat: 
        pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.fast_to_float = reg_value; 
        if(!limits(pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.fast_to_float,10,100)) {
            pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.fast_to_float = 50;
        }
        break;             
    case RWReg_FloatToFast: 
        pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.float_to_fast = reg_value; 
        if(!limits(pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.float_to_fast,20,200)) {
            pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.float_to_fast = 80;
        }
        break;             
    case RWReg_ChargeToDischarge: 
        pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.charge_to_discharge = reg_value; 
        if(!limits(pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.charge_to_discharge,-50,-1)){
            pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.charge_to_discharge = -20;
        }
        break;       
    case RWReg_DrySwitch: 
        pDEApp->device_config.i03t_nodes[i03t_index].dry_node.dry_switch = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].dry_node.dry_switch > 1) {
            pDEApp->device_config.i03t_nodes[i03t_index].dry_node.dry_switch = 0;
        }
        break;               
    case RWReg_DryOutputThreshold: 
        pDEApp->device_config.i03t_nodes[i03t_index].dry_node.out_threshold = reg_value; 
        break;      
    case RWReg_DryResumeThreshold: 
        pDEApp->device_config.i03t_nodes[i03t_index].dry_node.output_resume_threshold = reg_value; 
        break;      
    case RWReg_DryDelay: 
        pDEApp->device_config.i03t_nodes[i03t_index].dry_node.delay = reg_value; 
        break;                
    case RWReg_DryOutputMinVoltage: 
        pDEApp->device_config.i03t_nodes[i03t_index].dry_node.output_min_volt = reg_value; 
        break;     
    case RWReg_433WorkFreq: 
        pDEApp->device_config.i03t_nodes[i03t_index].wireless.work_freq = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].wireless.work_freq > 99) {
            pDEApp->device_config.i03t_nodes[i03t_index].wireless.work_freq = 54;
        }
        break;             
    case RWReg_433AssistFreq: 
        pDEApp->device_config.i03t_nodes[i03t_index].wireless.assist_freq = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].wireless.assist_freq > 99) {
            pDEApp->device_config.i03t_nodes[i03t_index].wireless.assist_freq = 99;
        }
        break;
    case RWReg_SOC: 
        i03t_node->discharge.soc = reg_value;
        if(i03t_node->discharge.soc > 1000) {
            i03t_node->discharge.soc = 1000;
        }
        break;
    case RWReg_SOH: 
        i03t_node->discharge.soh = reg_value; 
        if(i03t_node->discharge.soh > 1000) {
            i03t_node->discharge.soh = 1000;
        }
        soc_msg_put(i03t_addr,I03T_SOH_INIT);
        break;
																																 
    case RWReg_NominalCap: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.nominal_cap = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].cap.nominal_cap == 0) {
            pDEApp->device_config.i03t_nodes[i03t_index].cap.nominal_cap  = 12000;
        }
        break;       
    case RWReg_NominalVolt: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.nominal_volt = reg_value; 
        switch(pDEApp->device_config.i03t_nodes[i03t_index].cap.nominal_volt) {
            case 1:
            case 3:
            case 6:
                break;
            default:
                pDEApp->device_config.i03t_nodes[i03t_index].cap.nominal_volt = 1;
                break;
        }
        break;    
    
    case RWReg_FloatVolt: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.float_volt = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].cap.float_volt == 0) {
            pDEApp->device_config.i03t_nodes[i03t_index].cap.float_volt = 2350;
        }
        break;  
    
    case RWReg_TempCompensate: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.temp_compensate = reg_value; 
        break;    
    
    case RWReg_DischargeMethod: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.discharge_method = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].cap.discharge_method > 1) {
            pDEApp->device_config.i03t_nodes[i03t_index].cap.discharge_method = 0;
        }
        break; 
        
    case RWReg_ChargeMethod: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.charge_method = reg_value;
        if(pDEApp->device_config.i03t_nodes[i03t_index].cap.charge_method > 1) {
            pDEApp->device_config.i03t_nodes[i03t_index].cap.charge_method = 0;
        }
        break;    
        
    case RWReg_ChargeEfficiency: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.charge_efficiency = reg_value; break;        
    case RWReg_MaxDischargeCap: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.max_discharge_cap = reg_value; break;         
    case RWReg_CellMinVolt: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.cell_min_voltage = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].cap.cell_min_voltage == 0) {
            pDEApp->device_config.i03t_nodes[i03t_index].cap.cell_min_voltage = 1800;
        }
        break;             
    case RWReg_CellMaxVolt: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.cell_max_voltage = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].cap.cell_max_voltage == 0) {
            pDEApp->device_config.i03t_nodes[i03t_index].cap.cell_max_voltage = 2400;
        }
        break;             
    case RWReg_DischargeMaxCurrent: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.discharge_max_current = reg_value; break;     
    case RWReg_DischargeMinCurrent: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.discharge_min_current = reg_value; break;     
    case RWReg_MinCalculateTemp: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.min_calculate_temp = reg_value; break;        
    case RWReg_MaxCalculateTemp: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.max_calculate_temp = reg_value; break;        
    case RWReg_CapInitFlg: 
        pDEApp->device_config.i03t_nodes[i03t_index].cap.cap_init = reg_value; 
        if(pDEApp->device_config.i03t_nodes[i03t_index].cap.cap_init > 1) {
            pDEApp->device_config.i03t_nodes[i03t_index].cap.cap_init = 1;
        }
        break; 
	
    case RWReg_TotalVoltageOverThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_1 = reg_value; break;
	case RWReg_TotalVoltageOverThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_2 = reg_value; break;
	case RWReg_TotalVoltageOverThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_3 = reg_value; break;
	case RWReg_TotalVoltageOverThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_1_resume = reg_value; break;
	case RWReg_TotalVoltageOverThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_2_resume = reg_value; break;
	case RWReg_TotalVoltageOverThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_3_resume = reg_value; break;
	case RWReg_TotalVoltageOverThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_1_delay = reg_value; break;
	case RWReg_TotalVoltageOverThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_2_delay = reg_value; break;
	case RWReg_TotalVoltageOverThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_3_delay = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_1 = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_2 = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_3 = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_1_resume = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_2_resume = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_3_resume = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_1_delay = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_2_delay = reg_value; break;
	case RWReg_TotalVoltageLowThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_3_delay = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_1 = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_2 = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_3 = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_1_resume = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_2_resume = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_3_resume = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_1_delay = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_2_delay = reg_value; break;
	case RWReg_ChargeOverCurrentThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_3_delay = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_1 = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_2 = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_3 = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_1_resume = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_2_resume = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_3_resume = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_1_delay = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_2_delay = reg_value; break;
	case RWReg_DischargeOverCurrentThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_3_delay = reg_value; break;
																																															 
	case RWReg_TempOverThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_1 = reg_value; break;
	case RWReg_TempOverThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_2 = reg_value; break;
	case RWReg_TempOverThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_3 = reg_value; break;
	case RWReg_TempOverThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_1_resume = reg_value; break;
	case RWReg_TempOverThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_2_resume = reg_value; break;
	case RWReg_TempOverThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_3_resume = reg_value; break;
	case RWReg_TempOverThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_1_delay = reg_value; break;
	case RWReg_TempOverThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_2_delay = reg_value; break;
	case RWReg_TempOverThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_3_delay = reg_value; break;
																																															 
    case RWReg_SocLowThreshold_Level_1:  pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_1 = reg_value; break;
	case RWReg_SocLowThreshold_Level_2:  pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_2 = reg_value; break;
	case RWReg_SocLowThreshold_Level_3:  pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_3 = reg_value; break;
	case RWReg_SocLowThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_1_resume = reg_value; break;
	case RWReg_SocLowThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_2_resume = reg_value; break;
	case RWReg_SocLowThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_3_resume = reg_value; break;
	case RWReg_SocLowThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_1_delay = reg_value; break;
	case RWReg_SocLowThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_2_delay = reg_value; break;
	case RWReg_SocLowThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_3_delay = reg_value; break;
																																															 
    case RWReg_CellVoltOverThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_1 = reg_value; break;
	case RWReg_CellVoltOverThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_2 = reg_value; break;
	case RWReg_CellVoltOverThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_3 = reg_value; break;
	case RWReg_CellVoltOverThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_1_resume = reg_value; break;
	case RWReg_CellVoltOverThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_2_resume = reg_value; break;
	case RWReg_CellVoltOverThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_3_resume = reg_value; break;
	case RWReg_CellVoltOverThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_1_delay = reg_value; break;
	case RWReg_CellVoltOverThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_2_delay = reg_value; break;
	case RWReg_CellVoltOverThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_3_delay = reg_value; break;
																																															 
    case RWReg_CellVoltLowThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_1 = reg_value; break;
	case RWReg_CellVoltLowThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_2 = reg_value; break;
	case RWReg_CellVoltLowThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_3 = reg_value; break;
	case RWReg_CellVoltLowThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_1_resume = reg_value; break;
	case RWReg_CellVoltLowThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_2_resume = reg_value; break;
	case RWReg_CellVoltLowThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_3_resume = reg_value; break;
	case RWReg_CellVoltLowThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_1_delay = reg_value; break;
	case RWReg_CellVoltLowThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_2_delay = reg_value; break;
	case RWReg_CellVoltLowThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_3_delay = reg_value; break;
																																												
    case RWReg_CellTempOverThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_1 = reg_value; break;
	case RWReg_CellTempOverThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_2 = reg_value; break;
	case RWReg_CellTempOverThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_3 = reg_value; break;
	case RWReg_CellTempOverThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_1_resume = reg_value; break;
	case RWReg_CellTempOverThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_2_resume = reg_value; break;
	case RWReg_CellTempOverThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_3_resume = reg_value; break;
	case RWReg_CellTempOverThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_1_delay = reg_value; break;
	case RWReg_CellTempOverThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_2_delay = reg_value; break;
	case RWReg_CellTempOverThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_3_delay = reg_value; break;
																																															 
    case RWReg_CellInterResOverThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_1 = reg_value; break;
	case RWReg_CellInterResOverThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_2 = reg_value; break;
	case RWReg_CellInterResOverThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_3 = reg_value; break;
	case RWReg_CellInterResOverThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_1_resume = reg_value; break;
	case RWReg_CellInterResOverThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_2_resume = reg_value; break;
	case RWReg_CellInterResOverThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_3_resume = reg_value; break;
	case RWReg_CellInterResOverThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_1_delay = reg_value; break;
	case RWReg_CellInterResOverThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_2_delay = reg_value; break;
	case RWReg_CellInterResOverThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_3_delay = reg_value; break;

	case RWRegExtCfg_LeakThreshold_Level_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_1 = reg_value; break;
	case RWRegExtCfg_LeakThreshold_Level_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_2 = reg_value; break;
	case RWRegExtCfg_LeakThreshold_Level_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_3 = reg_value; break;
	case RWRegExtCfg_LeakThreshold_ResumeLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_1_resume = reg_value; break;
	case RWRegExtCfg_LeakThreshold_ResumeLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_2_resume = reg_value; break;
	case RWRegExtCfg_LeakThreshold_ResumeLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_3_resume = reg_value; break;
	case RWRegExtCfg_LeakThreshold_DelayLevel_1: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_1_delay = reg_value; break;
	case RWRegExtCfg_LeakThreshold_DelayLevel_2: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_2_delay = reg_value; break;
	case RWRegExtCfg_LeakThreshold_DelayLevel_3: pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_3_delay = reg_value; break;

    case RWReg_AlarmCorrelation:
        pDEApp->device_config.i03t_nodes[i03t_index].dry_node.alarm_correlation.alarm_correlation = reg_value; 
        break;
    
    case RWReg_AlarmCorrelationGroup:
        pDEApp->device_config.i03t_nodes[i03t_index].dry_node.alarm_correlation.alarm_correlation_group = reg_value; 
        break;
    case RWReg_AlarmCorrelationCells:
        pDEApp->device_config.i03t_nodes[i03t_index].dry_node.alarm_correlation.alarm_correlation_cells = reg_value; 
        break;
    
    case RWReg_VoltageLevel: 
        pDEApp->device_config.i03t_nodes[i03t_index].sys_para.voltage_level = reg_value;
        if(pDEApp->device_config.i03t_nodes[i03t_index].sys_para.voltage_level > 3) {
            pDEApp->device_config.i03t_nodes[i03t_index].sys_para.voltage_level = 1;
        }
        break;
    case RWReg_CurrentLevel: 
        pDEApp->device_config.i03t_nodes[i03t_index].sys_para.current_level = reg_value;
        if(pDEApp->device_config.i03t_nodes[i03t_index].sys_para.current_level > 1) {
            pDEApp->device_config.i03t_nodes[i03t_index].sys_para.current_level  = 0;
        }
        break;
    case RWReg_CellSNLength: 
        pDEApp->device_config.i03t_nodes[i03t_index].sys_para.sn_length = reg_value;
        if(pDEApp->device_config.i03t_nodes[i03t_index].sys_para.sn_length > 20) {
            pDEApp->device_config.i03t_nodes[i03t_index].sys_para.sn_length = 12;
        }
        break;
    case RWReg_AlarmMaxLevel:
        pDEApp->device_config.i03t_nodes[i03t_index].sys_para.alarm_max_level = reg_value;
        if(pDEApp->device_config.i03t_nodes[i03t_index].sys_para.alarm_max_level > 3 ||
            pDEApp->device_config.i03t_nodes[i03t_index].sys_para.alarm_max_level < 1) {
                pDEApp->device_config.i03t_nodes[i03t_index].sys_para.alarm_max_level = 2;
            }
        break;
    case RWReg_CellWorkMode:
        pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_mode = reg_value;
        if(pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_mode > 1) {
            pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_mode = 0;
        }
        break;


    case RWReg_ResetI03M:
        if(reg_value != 11) {
            return errErr;
        }
        command_msg_put(SysCmdReset,NULL,0);
        break;

	case RWReg_Format:
        if(reg_value != 44) {
            return errErr;
        }
        storage_msg_put(0,NULL,0,StorageFsFormat);
        break;
        

	default: 
		return errErr;


	}
    return errOK;
}

errStatus_t modbus_get_i03m_ro_reg_value(uint16_t reg,uint16_t *reg_value) {
    *reg_value = 0;
    DATE_yymmddhhmmss_t now = {0};
    Bsp_RtcGetTime(&now);
    switch(reg) {
        case ROReg_DeviceNum:
            *reg_value = pDEApp->device_config.i03m.i03t_number;
            break;
        case ROReg_TotalCellNum:
            *reg_value = pDEApp->device_config.i03m.cell_numbers;
            break;
        case ROReg_SoftVersionI03M:
            *reg_value = I03M_SOFT_VERSION;
            break;
        case ROReg_HardVersionI03M:
            *reg_value = I03M_HARD_VERSION;
            break;
         case ROReg_YearI03M: 
            *reg_value = now.bits.year + 2000;
            break;
        case ROReg_MonthI03M: 
            *reg_value = now.bits.month;
            break;
        case ROReg_DayI03M: 
            *reg_value = now.bits.day;
            break;
        case ROReg_HourI03M: 
            *reg_value = now.bits.hour;
            break;
        case ROReg_MinI03M: 
            *reg_value = now.bits.min;
            break;
        case ROReg_SecI03M: 
            *reg_value = now.bits.sec;
            break;
        default:
            *reg_value = 0;
            break;
        
    }
    
    return errOK;
}

errStatus_t modbus_get_i03m_rw_reg_value(uint16_t reg,uint16_t *reg_value) {
    *reg_value = 0;
    DATE_yymmddhhmmss_t now = {0};
    Bsp_RtcGetTime(&now);
    switch(reg) {
        case RWReg_Addr: 
            *reg_value = pDEApp->device_config.i03m.i03m_addr;
            break;
        case RWReg_BaudRate:
            //*reg_value = pDEApp->device_config.i03m.baudrate.bits.baudrate;
            break;
        case RWReg_DataBits:
            //*reg_value = pDEApp->device_config.i03m.baudrate.bits.data_bits;
            break;
        case RWReg_StopBits:
            //*reg_value = pDEApp->device_config.i03m.baudrate.bits.stop_bits;
            break;
        case RWReg_ParityBits:
            //*reg_value = pDEApp->device_config.i03m.baudrate.bits.check_bits;
            break;
        case RWReg_YearI03M: 
            *reg_value = now.bits.year + 2000;
            break;
        case RWReg_MonthI03M: 
            *reg_value = now.bits.month;
            break;
        case RWReg_DayI03M: 
            *reg_value = now.bits.day;
            break;
        case RWReg_HourI03M: 
            *reg_value = now.bits.hour;
            break;
        case RWReg_MinI03M: 
            *reg_value = now.bits.min;
            break;
        case RWReg_SecI03M: 
            *reg_value = now.bits.sec;
            break;
        case RW_StorePeriod:
            *reg_value = pDEApp->device_config.i03m.storage_period;
            break;
        case RWReg_SecH:{
            struct tm time1;     
            time1.tm_year = now.bits.year + 2000 - 1900;
            time1.tm_mon = now.bits.month - 1;
            time1.tm_mday = now.bits.day;
            time1.tm_hour = now.bits.hour;
            time1.tm_min = now.bits.min;
            time1.tm_sec = now.bits.sec;
            time1.tm_isdst = 0;
            uint32_t sec = (uint32_t)mktime(&time1) - 8 * 3600ul;
            *reg_value = sec >> 16;
            }
            break;
        case RWReg_SecL: {
            struct tm time1;     
            time1.tm_year = now.bits.year + 2000 - 1900;
            time1.tm_mon = now.bits.month - 1;
            time1.tm_mday = now.bits.day;
            time1.tm_hour = now.bits.hour;
            time1.tm_min = now.bits.min;
            time1.tm_sec = now.bits.sec;
            time1.tm_isdst = 0;
            uint32_t sec = (uint32_t)mktime(&time1) - 8 * 3600ul;
            *reg_value = sec;
            }
            break;
        default:
            *reg_value = 0;
            break;
        
    }
    
    return errOK;
}

errStatus_t modbus_set_i03m_rw_reg_value(uint16_t reg,uint16_t reg_value) {
    static uint32_t unix_sec = 0;
    switch(reg) {
        case RWReg_Addr: 
            pDEApp->device_config.i03m.i03m_addr = reg_value;
            return errOK;
        
         case RWReg_BaudRate:
//            pDEApp->device_config.i03m.baudrate.bits.baudrate = reg_value;
//            if(pDEApp->device_config.i03m.baudrate.bits.baudrate > 5) {
//                pDEApp->device_config.i03m.baudrate.bits.baudrate = 5;
//            }
//            command_msg_put(SysCmdSetBaudRate,NULL,0);
            break;
         
        case RWReg_DataBits:
//            pDEApp->device_config.i03m.baudrate.bits.data_bits = reg_value;
//            if(pDEApp->device_config.i03m.baudrate.bits.data_bits > 1) {
//                pDEApp->device_config.i03m.baudrate.bits.data_bits = 0;
//            }
            break;
        
        case RWReg_StopBits:
//            pDEApp->device_config.i03m.baudrate.bits.stop_bits = reg_value;
//            if(pDEApp->device_config.i03m.baudrate.bits.stop_bits > 1) {
//                pDEApp->device_config.i03m.baudrate.bits.stop_bits = 0;
//            }
            break;
        
        case RWReg_ParityBits:
//            pDEApp->device_config.i03m.baudrate.bits.check_bits = reg_value;
//            if(pDEApp->device_config.i03m.baudrate.bits.check_bits > 2) {
//                pDEApp->device_config.i03m.baudrate.bits.check_bits = 0;
//            }
//            command_msg_put(SysCmdSetBaudRate,NULL,0);
            break;

        case RWReg_YearI03M: {
                DATE_yymmddhhmmss_t time;
                Bsp_RtcGetTime(&time);
                time.bits.year = reg_value % 100;
                Bsp_RtcSetTime(&time);
                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,time.date,NULL,__FILE__,__LINE__);
            }
            return errOK;
        case RWReg_MonthI03M: {
                DATE_yymmddhhmmss_t time;
                Bsp_RtcGetTime(&time);
                time.bits.month = reg_value;
                Bsp_RtcSetTime(&time);
                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,time.date,NULL,__FILE__,__LINE__);
            }
            return errOK;
        case RWReg_DayI03M: 
            {
                DATE_yymmddhhmmss_t time;
                Bsp_RtcGetTime(&time);
                time.bits.day = reg_value;
                Bsp_RtcSetTime(&time);
                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,time.date,NULL,__FILE__,__LINE__);
            }
            return errOK;
        case RWReg_HourI03M: {
                DATE_yymmddhhmmss_t time;
                Bsp_RtcGetTime(&time);
                time.bits.hour = reg_value;
                Bsp_RtcSetTime(&time);
                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,time.date,NULL,__FILE__,__LINE__);
            }
            break;
        case RWReg_MinI03M: 
            {
                DATE_yymmddhhmmss_t time;
                Bsp_RtcGetTime(&time);
                time.bits.min = reg_value;
                Bsp_RtcSetTime(&time);
                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,time.date,NULL,__FILE__,__LINE__);
            }
            return errOK;
            
        case RWReg_SecI03M: 
            {
                DATE_yymmddhhmmss_t time;
                Bsp_RtcGetTime(&time);
                time.bits.sec = reg_value;
                Bsp_RtcSetTime(&time);
                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,time.date,NULL,__FILE__,__LINE__);
            }
            return errOK;
            
        case RWReg_SecH:
            {
                unix_sec = (uint32_t)reg_value << 16;
            }
            break;
        case RWReg_SecL:
            {
                DATE_yymmddhhmmss_t now;
                unix_sec = unix_sec + reg_value;
                unix_sec += 8 * 3600;
                struct tm *time2 = localtime(&unix_sec);
                now.bits.year = time2->tm_year - 100;
                now.bits.month = time2->tm_mon + 1;
                now.bits.day = time2->tm_mday;
                now.bits.hour = time2->tm_hour;
                now.bits.min = time2->tm_min;
                now.bits.sec = time2->tm_sec;
                
                Bsp_RtcSetTime(&now);
                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_CONFIGTIME,now.date,NULL,__FILE__,__LINE__);
            }
            break;
            
        case RW_StorePeriod:
            pDEApp->device_config.i03m.storage_period = reg_value;
            if(pDEApp->device_config.i03m.storage_period < 10) {
                pDEApp->device_config.i03m.storage_period = 300;
            }
            break;
            
        case RWReg_ResetI03M:
            if(reg_value != 11) {
                return errErr;
            }
            command_msg_put(SysCmdReset,NULL,0);
            break;

        case RWReg_Format:
            if(reg_value != 44) {
                return errErr;
            }
            storage_msg_put(0,NULL,0,StorageFsFormat);
            break;
            
        case RWReg_ClearHist:
            break;
        default:
            return errErr;
        
    }
    
    return errOK;
}

errStatus_t modbus_get_i03t_rw_reg_value(uint8_t i03t_addr,uint16_t reg,uint16_t *reg_value)  {
	*reg_value = 0;
	I03T_Info_t *i03t_node = (I03T_Info_t *)i03t_node_find(i03t_addr);
	if(i03t_node == NULL) {
		return errErr;
	}
   
	uint8_t i03t_index = i03t_addr - 1;
    
    Convt_t conv = {0};

	switch(reg) {
    case RWReg_ModbusAddr:*reg_value = i03t_addr;break;

    case RWReg_TotalVoltageSamplePeriod:*reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_total_volt;break;
    case RWReg_CellPullPeriod:          *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_cell_poll;break;          
    case RWReg_ResSamplePeriod:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sample_period.period_int_res;break;         
    case RWReg_CellNumber:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number;break;              
    case RWReg_CriticalCurrentLimits:   *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.critical_current;break;   
    case RWReg_FastToFloat:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.fast_to_float;break;             
    case RWReg_FloatToFast:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.float_to_fast;break;             
    case RWReg_ChargeToDischarge:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].charge_status_threshold.charge_to_discharge;break;       
    case RWReg_DrySwitch:               *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].dry_node.dry_switch;break;               
    case RWReg_DryOutputThreshold:      *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].dry_node.out_threshold;break;      
    case RWReg_DryResumeThreshold:      *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].dry_node.output_resume_threshold;break;      
    case RWReg_DryDelay:                *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].dry_node.delay;break;                
    case RWReg_DryOutputMinVoltage:     *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].dry_node.output_min_volt;break;     
    case RWReg_SOC:                     *reg_value = i03t_node->discharge.soc;break;
    case RWReg_SOH:                     *reg_value = i03t_node->discharge.soh;break; 
    case RWReg_433WorkFreq:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].wireless.work_freq;break;             
    case RWReg_433AssistFreq:           *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].wireless.assist_freq;break;           

    case RWReg_NominalCap:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.nominal_cap;break;       
    case RWReg_NominalVolt:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.nominal_volt;break;             
    case RWReg_FloatVolt:               *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.float_volt;break;               
    case RWReg_TempCompensate:          *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.temp_compensate;break;          
    case RWReg_DischargeMethod:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.discharge_method;break;         
    case RWReg_ChargeMethod:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.charge_method;break;            
    case RWReg_ChargeEfficiency:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.charge_efficiency;break;        
    case RWReg_MaxDischargeCap:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.max_discharge_cap;break;         
    case RWReg_CellMinVolt:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.cell_min_voltage;break;             
    case RWReg_CellMaxVolt:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.cell_max_voltage;break;             
    case RWReg_DischargeMaxCurrent:     *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.discharge_max_current;break;     
    case RWReg_DischargeMinCurrent:     *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.discharge_min_current;break;     
    case RWReg_MinCalculateTemp:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.min_calculate_temp;break;        
    case RWReg_MaxCalculateTemp:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.max_calculate_temp;break;        
    case RWReg_CapInitFlg:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].cap.cap_init;break; 
	
    case RWReg_TotalVoltageOverThreshold_Level_1:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_1; break;
	case RWReg_TotalVoltageOverThreshold_Level_2:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_2; break;
	case RWReg_TotalVoltageOverThreshold_Level_3:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_3; break;
	case RWReg_TotalVoltageOverThreshold_ResumeLevel_1:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_1_resume; break;
	case RWReg_TotalVoltageOverThreshold_ResumeLevel_2:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_2_resume; break;
	case RWReg_TotalVoltageOverThreshold_ResumeLevel_3:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_3_resume; break;
	case RWReg_TotalVoltageOverThreshold_DelayLevel_1:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_1_delay; break;
	case RWReg_TotalVoltageOverThreshold_DelayLevel_2:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_2_delay; break;
	case RWReg_TotalVoltageOverThreshold_DelayLevel_3:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.over.level_3_delay; break;
	case RWReg_TotalVoltageLowThreshold_Level_1:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_1; break;
	case RWReg_TotalVoltageLowThreshold_Level_2:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_2; break;
	case RWReg_TotalVoltageLowThreshold_Level_3:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_3; break;
	case RWReg_TotalVoltageLowThreshold_ResumeLevel_1:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_1_resume; break;
	case RWReg_TotalVoltageLowThreshold_ResumeLevel_2:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_2_resume; break;
	case RWReg_TotalVoltageLowThreshold_ResumeLevel_3:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_3_resume; break;
	case RWReg_TotalVoltageLowThreshold_DelayLevel_1:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_1_delay; break;
	case RWReg_TotalVoltageLowThreshold_DelayLevel_2:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_2_delay; break;
	case RWReg_TotalVoltageLowThreshold_DelayLevel_3:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_voltage.low.level_3_delay; break;
	case RWReg_ChargeOverCurrentThreshold_Level_1:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_1; break;
	case RWReg_ChargeOverCurrentThreshold_Level_2:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_2; break;
	case RWReg_ChargeOverCurrentThreshold_Level_3:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_3; break;
	case RWReg_ChargeOverCurrentThreshold_ResumeLevel_1:      *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_1_resume; break;
	case RWReg_ChargeOverCurrentThreshold_ResumeLevel_2:      *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_2_resume; break;
	case RWReg_ChargeOverCurrentThreshold_ResumeLevel_3:      *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_3_resume; break;
	case RWReg_ChargeOverCurrentThreshold_DelayLevel_1:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_1_delay; break;
	case RWReg_ChargeOverCurrentThreshold_DelayLevel_2:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_2_delay; break;
	case RWReg_ChargeOverCurrentThreshold_DelayLevel_3:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_charge.level_3_delay; break;
	case RWReg_DischargeOverCurrentThreshold_Level_1:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_1; break;
	case RWReg_DischargeOverCurrentThreshold_Level_2:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_2; break;
	case RWReg_DischargeOverCurrentThreshold_Level_3:         *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_3; break;
	case RWReg_DischargeOverCurrentThreshold_ResumeLevel_1:   *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_1_resume; break;
	case RWReg_DischargeOverCurrentThreshold_ResumeLevel_2:   *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_2_resume; break;
	case RWReg_DischargeOverCurrentThreshold_ResumeLevel_3:   *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_3_resume; break;
	case RWReg_DischargeOverCurrentThreshold_DelayLevel_1:    *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_1_delay; break;
	case RWReg_DischargeOverCurrentThreshold_DelayLevel_2:    *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_2_delay; break;
	case RWReg_DischargeOverCurrentThreshold_DelayLevel_3:    *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.total_current.over_discharge.level_3_delay; break;
	
	case RWReg_TempOverThreshold_Level_1:                     *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_1; break;
	case RWReg_TempOverThreshold_Level_2:                     *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_2; break;
	case RWReg_TempOverThreshold_Level_3:                     *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_3; break;
	case RWReg_TempOverThreshold_ResumeLevel_1:               *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_1_resume; break;
	case RWReg_TempOverThreshold_ResumeLevel_2:               *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_2_resume; break;
	case RWReg_TempOverThreshold_ResumeLevel_3:               *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_3_resume; break;
	case RWReg_TempOverThreshold_DelayLevel_1:                *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_1_delay; break;
	case RWReg_TempOverThreshold_DelayLevel_2:                *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_2_delay; break;
	case RWReg_TempOverThreshold_DelayLevel_3:                *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.temp.over.level_3_delay; break;
	
    case RWReg_SocLowThreshold_Level_1:                       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_1; break;
	case RWReg_SocLowThreshold_Level_2:                       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_2; break;
	case RWReg_SocLowThreshold_Level_3:                       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_3; break;
	case RWReg_SocLowThreshold_ResumeLevel_1:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_1_resume; break;
	case RWReg_SocLowThreshold_ResumeLevel_2:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_2_resume; break;
	case RWReg_SocLowThreshold_ResumeLevel_3:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_3_resume; break;
	case RWReg_SocLowThreshold_DelayLevel_1:                  *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_1_delay; break;
	case RWReg_SocLowThreshold_DelayLevel_2:                  *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_2_delay; break;
	case RWReg_SocLowThreshold_DelayLevel_3:                  *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc.low.level_3_delay; break;
	
    case RWReg_CellVoltOverThreshold_Level_1:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_1; break;
	case RWReg_CellVoltOverThreshold_Level_2:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_2; break;
	case RWReg_CellVoltOverThreshold_Level_3:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_3; break;
	case RWReg_CellVoltOverThreshold_ResumeLevel_1:           *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_1_resume; break;
	case RWReg_CellVoltOverThreshold_ResumeLevel_2:           *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_2_resume; break;
	case RWReg_CellVoltOverThreshold_ResumeLevel_3:           *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_3_resume; break;
	case RWReg_CellVoltOverThreshold_DelayLevel_1:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_1_delay; break;
	case RWReg_CellVoltOverThreshold_DelayLevel_2:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_2_delay; break;
	case RWReg_CellVoltOverThreshold_DelayLevel_3:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.over.level_3_delay; break;
	
    case RWReg_CellVoltLowThreshold_Level_1:                  *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_1; break;
	case RWReg_CellVoltLowThreshold_Level_2:                  *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_2; break;
	case RWReg_CellVoltLowThreshold_Level_3:                  *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_3; break;
	case RWReg_CellVoltLowThreshold_ResumeLevel_1:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_1_resume; break;
	case RWReg_CellVoltLowThreshold_ResumeLevel_2:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_2_resume; break;
	case RWReg_CellVoltLowThreshold_ResumeLevel_3:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_3_resume; break;
	case RWReg_CellVoltLowThreshold_DelayLevel_1:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_1_delay; break;
	case RWReg_CellVoltLowThreshold_DelayLevel_2:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_2_delay; break;
	case RWReg_CellVoltLowThreshold_DelayLevel_3:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.voltage.low.level_3_delay; break;
	
    case RWReg_CellTempOverThreshold_Level_1:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_1; break;
	case RWReg_CellTempOverThreshold_Level_2:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_2; break;
	case RWReg_CellTempOverThreshold_Level_3:                 *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_3; break;
	case RWReg_CellTempOverThreshold_ResumeLevel_1:           *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_1_resume; break;
	case RWReg_CellTempOverThreshold_ResumeLevel_2:           *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_2_resume; break;
	case RWReg_CellTempOverThreshold_ResumeLevel_3:           *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_3_resume; break;
	case RWReg_CellTempOverThreshold_DelayLevel_1:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_1_delay; break;
	case RWReg_CellTempOverThreshold_DelayLevel_2:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_2_delay; break;
	case RWReg_CellTempOverThreshold_DelayLevel_3:            *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.temp.over.level_3_delay; break;
	
    case RWReg_CellInterResOverThreshold_Level_1:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_1; break;
	case RWReg_CellInterResOverThreshold_Level_2:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_2; break;
	case RWReg_CellInterResOverThreshold_Level_3:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_3; break;
	case RWReg_CellInterResOverThreshold_ResumeLevel_1:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_1_resume; break;
	case RWReg_CellInterResOverThreshold_ResumeLevel_2:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_2_resume; break;
	case RWReg_CellInterResOverThreshold_ResumeLevel_3:       *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_3_resume; break;
	case RWReg_CellInterResOverThreshold_DelayLevel_1:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_1_delay; break;
	case RWReg_CellInterResOverThreshold_DelayLevel_2:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_2_delay; break;
	case RWReg_CellInterResOverThreshold_DelayLevel_3:        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.cell.res.over.level_3_delay; break;

	case RWRegExtCfg_LeakThreshold_Level_1:                   *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_1; break;
	case RWRegExtCfg_LeakThreshold_Level_2:                   *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_2; break;
	case RWRegExtCfg_LeakThreshold_Level_3:                   *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_3; break;
	case RWRegExtCfg_LeakThreshold_ResumeLevel_1:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_1_resume; break;
	case RWRegExtCfg_LeakThreshold_ResumeLevel_2:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_2_resume; break;
	case RWRegExtCfg_LeakThreshold_ResumeLevel_3:             *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_3_resume; break;
	case RWRegExtCfg_LeakThreshold_DelayLevel_1:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_1_delay; break;
	case RWRegExtCfg_LeakThreshold_DelayLevel_2:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_2_delay; break;
	case RWRegExtCfg_LeakThreshold_DelayLevel_3:              *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.leak.leak.level_3_delay; break;
 
    case RWReg_AlarmCorrelation:
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].dry_node.alarm_correlation.alarm_correlation;
        break;
    case RWReg_AlarmCorrelationGroup:
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].dry_node.alarm_correlation.alarm_correlation_group;
        break;
    case RWReg_AlarmCorrelationCells:
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].dry_node.alarm_correlation.alarm_correlation_cells;
        break;
    
    case RWReg_VoltageLevel: 
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sys_para.voltage_level;
        break;
    case RWReg_CurrentLevel: 
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sys_para.current_level;
        break;
    case RWReg_CellSNLength: 
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sys_para.sn_length;
        break;
    case RWReg_AlarmMaxLevel:
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sys_para.alarm_max_level;
        break;
    case RWReg_CellWorkMode:
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_mode;
        break;
    case RWReg_VoltageCalibZeroOffset:
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].calib.total_voltage.zero_offset;
        break;
    case RWReg_VoltageCalibKH:
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.total_voltage.kb.k;
        *reg_value = conv.u >> 16;
        break;
    case RWReg_VoltageCalibKL:
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.total_voltage.kb.k;
        *reg_value = conv.u;
        break;
    case RWReg_VoltageCalibBH:                     
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.total_voltage.kb.b;
        *reg_value = conv.u >> 16;
        break;
    case RWReg_VoltageCalibBL:
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.total_voltage.kb.b;
        *reg_value = conv.u;
        break;
    case RWReg_Current1CalibZeroOffset:        
        *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].calib.current[0].zero_offset;        
        break;
    case RWReg_Current1CalibKH:  
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.current[0].kb.k;
        *reg_value = conv.u >> 16;        
        break;
    case RWReg_Current1CalibKL:   
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.current[0].kb.k;
        *reg_value = conv.u;              
        break;
    case RWReg_Current1CalibBH: 
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.current[0].kb.b;
        *reg_value = conv.u >> 16;         
        break;
    case RWReg_Current1CalibBL: 
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.current[0].kb.b;
        *reg_value = conv.u;          
        break;
    
    case RWReg_TempCalibKH:  
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.temp.kb.k;
        *reg_value = conv.u >> 16;          
        break;
    case RWReg_TempCalibKL:   
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.temp.kb.k;
        *reg_value = conv.u;          
        break;
    case RWReg_TempCalibBH:  
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.temp.kb.b;
        *reg_value = conv.u >> 16;          
        break;
    case RWReg_TempCalibBL: 
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.temp.kb.b;
        *reg_value = conv.u;          
        break;
    case RWReg_5VCalibKH: 
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.voltage_5v.kb.k;
        *reg_value = conv.u >> 16;           
        break;
    case RWReg_5VCalibKL:   
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.voltage_5v.kb.k;
        *reg_value = conv.u;           
        break;
    case RWReg_5VCalibBH:  
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.voltage_5v.kb.b;
        *reg_value = conv.u >> 16;           
        break;
    case RWReg_5VCalibBL:
        conv.f = pDEApp->device_config.i03t_nodes[i03t_index].calib.voltage_5v.kb.b;
        *reg_value = conv.u;           
        break;

	default:
        *reg_value = 0;
		break;
	}  

    return errOK;

}


errStatus_t modbus_get_i03t_ro_reg_value(uint8_t i03t_addr,uint16_t reg,uint16_t *reg_value) {
    *reg_value = 0;
    I03T_Info_t *i03t_node = (I03T_Info_t *)i03t_node_find(i03t_addr);
    if(i03t_node == NULL) {
        return errErr;
    }
    
    uint8_t i03t_index = i03t_addr - 1;
        
    if(reg >= ROReg_BatGroupStatus && reg <= ROReg_BatGroupRegMax) {
        switch (reg){
            case ROReg_BatGroupStatus:
                *reg_value = i03t_node->discharge.status;
                return errOK;                                                   
            case ROReg_BatGroupVoltage:
                *reg_value = multiple(i03t_node->discharge.voltage,MODBUS_VOLT_CONV);
                return errOK;
            case ROReg_BatGroupCurrent_1:
                *reg_value = multiple(i03t_node->discharge.current[0],MODBUS_CURRENT_CONV);
                return errOK; 
            case ROReg_BatSOC:
                *reg_value = multiple(i03t_node->discharge.soc,1);
                return errOK;  
            case ROReg_BatAvailableTime:
                *reg_value = multiple(i03t_node->discharge.available_time,1);
                return errOK;
            
            case ROReg_SOH:
                *reg_value = i03t_node->discharge.soh;
                return errOK;                 
            case ROReg_DryNodeStatus:
                *reg_value = i03t_node->hist.total_info.dry_node_status;
                return errOK;                                     
            case ROReg_CellNumber:
                *reg_value = pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number;
                return errOK;         
            case ROReg_CellMaxVoltage:
                *reg_value = i03t_node->cell_volt_peak.max;
                return errOK;
            case ROReg_CellMinVoltage:
                *reg_value = i03t_node->cell_volt_peak.min;
                return errOK;
            case ROReg_CellAverageVoltage:
                *reg_value = i03t_node->cell_volt_peak.average;
                return errOK;
            case ROReg_CellMaxVoltageId:
                *reg_value = i03t_node->cell_volt_peak.max_id;
                return errOK;
            case ROReg_CellMinVoltageId:
                *reg_value = i03t_node->cell_volt_peak.min_id;
                return errOK;
            case ROReg_CellMaxTemp:
                *reg_value = i03t_node->cell_temp_peak.max;
                return errOK;
            case ROReg_CellMinTemp:
                *reg_value = i03t_node->cell_temp_peak.min;
                return errOK;
            case ROReg_CellAverageTemp:
                *reg_value = i03t_node->cell_temp_peak.average;
                return errOK;
            case ROReg_CellMaxTempId:
                *reg_value = i03t_node->cell_temp_peak.max_id;
                return errOK;
            case ROReg_CellMinTempId:
                *reg_value = i03t_node->cell_temp_peak.min_id;
                return errOK;
            case ROReg_CellMaxRes:
                *reg_value = i03t_node->cell_res_peak.max;
                return errOK;
            case ROReg_CellMinRes:
                *reg_value = i03t_node->cell_res_peak.min;
                return errOK;
            case ROReg_CellMaxResId:
                *reg_value = i03t_node->cell_res_peak.max_id;
                return errOK;
            case ROReg_CellMinResId:
                *reg_value = i03t_node->cell_res_peak.min_id;
                return errOK;
            case ROReg_BatGroupCurrent_2:
                *reg_value = multiple(i03t_node->discharge.current[1],MODBUS_CURRENT_CONV);
                return errOK;
            case ROReg_BatGroupCurrent_3:
                *reg_value = multiple(i03t_node->discharge.current[2],MODBUS_CURRENT_CONV);
                return errOK;
            case ROReg_BatGroupCurrent_4:
                *reg_value = multiple(i03t_node->discharge.current[3],MODBUS_CURRENT_CONV);
                return errOK;
            case ROReg_CellDiffTemp:
                *reg_value = i03t_node->cell_temp_peak.diff;
                return errOK;
            case ROReg_CellDiffVolt:
                *reg_value = i03t_node->cell_volt_peak.diff;
                return errOK;
            case ROReg_CellDiffRes:
                *reg_value = i03t_node->cell_volt_peak.diff;
                return errOK;

        
            case ROReg_SoftVersion:
                *reg_value = i03t_node->soft_version;
                return errOK;
            case ROReg_HardVersion:
                *reg_value = i03t_node->hard_version;
                return errOK;
            
         
            default:
                *reg_value = 0;
                return errOK;
        }
        
    }    
    
    if(reg >= ROReg_AlarmCell_1 && reg <= ROReg_AlarmCell_MAX) {
        uint16_t index = reg-ROReg_AlarmCell_1;
   
        if(index >= CONFIG_MAX_CELL) {
            return errErr;
        }
        *reg_value = i03t_node->alarm.alarm.cell_alarm[index];
        return errOK;
    }
    
    switch(reg) {
       case ROReg_AlarmBatGroup1:
            *reg_value = i03t_node->alarm.alarm.bat_group_alarm1;
            return errOK;
        case ROReg_AlarmBatGroup2:
            *reg_value = i03t_node->alarm.alarm.bat_group_alarm2;
            return errOK;
        case ROReg_AlarmBatGroup3:
            *reg_value = i03t_node->alarm.alarm.bat_group_alarm3;
            return errOK;
    }

    
    if(reg >= ROReg_CellVoltage_1 && reg <= ROReg_CellSpare_MAX_2) {
        #define _VOLT    0x00
        #define _RES     0x01
        #define _TEMP    0x02
        #define _SPARE1  0x03
        #define _SPARE2  0x04
        uint16_t cell_index = (reg - ROReg_CellVoltage_1) / 5;
        if(cell_index >= CONFIG_MAX_CELL) {
            return errErr;
        }
        
        for(uint16_t index = 0;index < 5;index ++) {
            if(ROReg_CellVoltage_1 + index + cell_index * 5 ==  reg) {
                switch(index) {
                    case _VOLT:
                        *reg_value = i03t_node->hist.cells[cell_index].voltage;
                        break;
                    case _RES:
                        *reg_value = i03t_node->hist.cells[cell_index].inter_res;
                        break;
                    case _TEMP:
                         *reg_value = i03t_node->hist.cells[cell_index].temperature;
                        break;
                    case _SPARE1:
                         *reg_value = 0;
                        break;
                    case _SPARE2:
                         *reg_value = 0;
                        break;
                    
                }
                return errOK;
            }
        }
        return errErr;
    }
    
    *reg_value = 0;
    return errOK;
}

uint16_t modbus_read_rw_regs_handler(modbus_request_t *request,uint8_t *buffer) {
	uint16_t length = 0;
	uint16_t reg = request->reg_addr_hi << 8 | request->reg_addr_lo;
    uint16_t reg_no = request->regs_num_hi << 8 | request->regs_num_lo;

    if(!(reg & 0x8000) || reg_no > FUNCTION_CONTINUES_R_MAX_REGS || reg_no == 0) {
        return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
    }
    
    uint8_t addr = (reg & 0x7FFF) >> 12;
    reg = reg & 0x0FFF;

    if(!addr) {

        uint16_t *regs_value = sys_malloc(sizeof(uint16_t) * (reg_no));
        if(regs_value == NULL) {
            return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
        }

        for(uint16_t reg_cnt = 0;reg_cnt < reg_no;reg_cnt++) {
            if(modbus_get_i03m_rw_reg_value(reg++,(uint16_t *)regs_value + (reg_cnt)) != errOK) {
                sys_free(regs_value);
                return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
            } else {
                regs_value[reg_cnt] = swap16(regs_value[reg_cnt]);
            }
        }
        
        length = modbus_response_package_fun_03_04(0,request,buffer,(uint8_t *)regs_value,2 * (reg_no));
        sys_free(regs_value);     
        
    } else {
        
        uint16_t *regs_value = sys_malloc(sizeof(uint16_t) * (reg_no));
        if(regs_value == NULL) {
            return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
        }
         
        for(uint16_t reg_cnt = 0;reg_cnt < reg_no;reg_cnt++) {
            if(modbus_get_i03t_rw_reg_value(addr,reg++,(uint16_t *)regs_value + (reg_cnt)) != errOK) {
                sys_free(regs_value);
                return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
            } else {
                regs_value[reg_cnt] = swap16(regs_value[reg_cnt]);
            }
        }
        
        length = modbus_response_package_fun_03_04(0,request,buffer,(uint8_t *)regs_value,2 * (reg_no));
        sys_free(regs_value);     
         
    }

	return length;
}

uint16_t modbus_response_err(modbus_request_t *request,uint8_t *buffer,ModbusErrCode_t err_code) {
    uint8_t data;
    data = err_code;
	uint16_t length = modbus_response_package(1,request,buffer,&data,1);
	return length;
}

uint16_t modbus_read_ro_regs_handler(modbus_request_t *request,uint8_t *buffer) {
	uint16_t length = 0;
	uint16_t reg = request->reg_addr_hi << 8 | request->reg_addr_lo;
    uint16_t reg_no = request->regs_num_hi << 8 | request->regs_num_lo;
    
    if((reg & 0x8000) || reg_no > FUNCTION_CONTINUES_R_MAX_REGS || reg_no == 0) {
        return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
    }
    
    uint8_t addr = reg >> 12;
    reg = reg & 0x0FFF;

    if(!addr) {
        
        uint16_t *regs_value = sys_malloc(sizeof(uint16_t) * (reg_no));
        if(regs_value == NULL) {
            return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
        }

        for(uint16_t reg_cnt = 0;reg_cnt < reg_no;reg_cnt++) {
            if(modbus_get_i03m_ro_reg_value(reg++,(uint16_t *)regs_value + (reg_cnt)) != errOK) {
                sys_free(regs_value);
                return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
            } else {
                 regs_value[reg_cnt] = swap16(regs_value[reg_cnt]);
            }
        }
        
        length = modbus_response_package_fun_03_04(0,request,buffer,(uint8_t *)regs_value,2 * (reg_no));
        
        sys_free(regs_value);
                
       
    } else {
         
        uint16_t *regs_value = sys_malloc(sizeof(uint16_t) * (reg_no));
        if(regs_value == NULL) {
            return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
        }
        for(uint16_t reg_cnt = 0;reg_cnt < reg_no;reg_cnt++) {
            if(modbus_get_i03t_ro_reg_value(addr,reg++,(uint16_t *)regs_value + (reg_cnt)) != errOK) {
                sys_free(regs_value);
                return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
            } else {
                 regs_value[reg_cnt] = swap16(regs_value[reg_cnt]);
            }
        }
        
        length = modbus_response_package_fun_03_04(0,request,buffer,(uint8_t *)regs_value,2 * (reg_no));
        
        sys_free(regs_value);

    }
	return length;
}
uint16_t modbus_write_regs_handler(modbus_request_t *req,uint8_t *buffer) {
    
    modbus_request_conti_write_t *request = (modbus_request_conti_write_t *)req;
    uint16_t length = 0;
	uint16_t reg = request->reg_addr_hi << 8 | request->reg_addr_lo;
    uint16_t reg_no = request->regs_num_hi << 8 | request->regs_num_lo;

    if(!(reg & 0x8000) || reg_no > FUNCTION_CONTINUES_W_MAX_REGS) {
        return modbus_response_err((modbus_request_t *)req,buffer,MODBUS_ERR_illegal_Data);
    }
    
    uint8_t addr = (reg & 0x7FFF) >> 12;
    reg = reg & 0x0FFF;
    uint16_t reg_value; 
    uint8_t index = 0;
    if(!addr) {
        for(uint8_t i = 0;i<reg_no;i++) {
            reg_value = (uint16_t)request->data[index] << 8 | request->data[index+1];
            index += 2;
            if(modbus_set_i03m_rw_reg_value(reg++,reg_value) != errOK) {
                return modbus_response_err((modbus_request_t *)req,buffer,MODBUS_ERR_illegal_Data);
            }
        }

    } else {
        for(uint8_t i = 0;i<reg_no;i++) {
            reg_value = (uint16_t)request->data[index] << 8 | request->data[index+1];
            index += 2;
            if(modbus_set_i03t_rw_reg_value(addr,reg++,reg_value) != errOK) {
                return modbus_response_err((modbus_request_t *)req,buffer,MODBUS_ERR_illegal_Data);
            }
        }  
        
        if(addr <= CONFIG_MAX_I03T) {
            I03T_Info_t *i03t_node = i03t_node_find(addr);
            if(i03t_node != NULL) {
                i03t_node->flag.bits.synch_config = 0;
            }
        }
    }
    
    storage_msg_put(0,NULL,0,StorageParaInfor);
    uint8_t data[4];
	data[0] = request->reg_addr_hi;
	data[1] = request->reg_addr_lo;
	data[2] = request->regs_num_hi;
	data[3] = request->regs_num_lo;
	length = modbus_response_package(0,req,buffer,data,4);
	return length;
}

uint16_t modbus_write_reg_handler(modbus_request_t *request,uint8_t *buffer) {

    uint16_t length = 0;
	uint16_t reg = request->reg_addr_hi << 8 | request->reg_addr_lo;
    uint16_t reg_value = request->regs_num_hi << 8 | request->regs_num_lo;

    if(!(reg & 0x8000)) {
        return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
    }
    
    uint8_t addr = (reg & 0x7000) >> 12;
    reg = reg & 0x0FFF;
    
     if(!addr) {
        if(modbus_set_i03m_rw_reg_value(reg,reg_value) != errOK) {
            return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
        } 
    } else {
        if(modbus_set_i03t_rw_reg_value(addr,reg,reg_value) != errOK) {
            return modbus_response_err(request,buffer,MODBUS_ERR_illegal_Data);
        } 
        
        if(addr <= CONFIG_MAX_I03T) {
            I03T_Info_t *i03t_node = i03t_node_find(addr);
            if(i03t_node != NULL) {
                i03t_node->flag.bits.synch_config = 0;
            }
        }
    }
    storage_msg_put(0,NULL,0,StorageParaInfor);
    
	uint8_t data[4];
	data[0] = request->reg_addr_hi;
	data[1] = request->reg_addr_lo;
	data[2] = request->regs_num_hi;
	data[3] = request->regs_num_lo;
	length = modbus_response_package(0,request,buffer,data,4);
	return length;
}

const modbus_requests_t requests[] = {
    { FUNCTION_READ_RO_REGS,      modbus_read_ro_regs_handler},
	{ FUNCTION_READ_RW_REGS,      modbus_read_rw_regs_handler},
	{ FUNCTION_WRTIE_REG,         modbus_write_reg_handler},
    { FUNCTION_WRTIE_REGS,        modbus_write_regs_handler},
};


#define MAX_REQUESTS sizeof(requests)/sizeof(requests[0])


void modbus_handler(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length) {

    if(length > 10) {
        if(!modbus_crc16_check_conti_write(pdata,length)) {
            return;
        }
    } else {
        if(!modbus_crc16_check(pdata,length)) {
            return;
        }
    } 
	
	modbus_request_t *request = (modbus_request_t *)pdata;
    
    if(request->address != pDEApp->device_config.i03m.i03m_addr &&\
       request->address != 0x00 &&\
       request->address != 0xFF) {
        return;
    }

	uint16_t tx_length = 0;
    bool done = false;
    uint8_t *modbus_tx_buffer = sys_malloc(256);
    
    if(modbus_tx_buffer == NULL) {
        return;
    }
    
	for(uint8_t index = 0; index < MAX_REQUESTS; index++) {
		if(request->function == requests[index].funtion) {
			tx_length = requests[index].modbus_fun(request,modbus_tx_buffer);
            modbus_protocol_send_bytes(commType,modbus_tx_buffer,tx_length);
            done = true;
            break;
		}
	}
    
    if(!done) {
        tx_length = modbus_response_err(request,modbus_tx_buffer,MODBUS_ERR_illegal_Fun);
        modbus_protocol_send_bytes(commType,modbus_tx_buffer,tx_length);
    }
    
    sys_free(modbus_tx_buffer);

}

void debug_sendbytes(uint8_t *pdata,uint16_t bytes);
void modbus_protocol_send_bytes(COMM_TYPE_t type,uint8_t *pdata,uint16_t length) {
    switch(type) {
        case COMM_TYPE_ETH:
            break;
        case COMM_TYPE_USB:
            break;

        case COMM_TYPE_COM:
            debug_sendbytes(pdata,length);
            break;
        
        case COMM_TYPE_485_1:
            Bsp_Rs485_1_SendBytes(pdata,length);
            break;
        case COMM_TYPE_485_2:
            Bsp_Rs485_2_SendBytes(pdata,length);
            break;
        case COMM_TYPE_485_3:
            Bsp_Rs485_3_SendBytes(pdata,length);
            break;
        case COMM_TYPE_485_4:
            Bsp_Rs485_4_SendBytes(pdata,length);
            break;

        default:
            break;
    }
}






