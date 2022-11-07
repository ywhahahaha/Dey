#include "main.h"
#include "appl_para.h"
#include "storage.h"
#include "crc_check.h"
#include "project_config.h"
#include "thread_debug.h"

osSemaphoreId_t flash_lock_sem = NULL;

/*
load para,if err use default para.
*/
errStatus_t appl_para_load(void) {
    
    storage_readbytes(ADDR_DEVICE_CONFIG,(uint8_t *)&pDEApp->device_config,sizeof(DeviceConfig_t));
    
    uint32_t crc_value = CRC_Get32((uint8_t *)(&pDEApp->device_config) + 4,sizeof(DeviceConfig_t) - 4);
    
    return crc_value == pDEApp->device_config.check_value ? errOK : errErr;

}

/*
save para
*/

errStatus_t appl_para_save(void) {
    
    osSemaphoreAcquire(flash_lock_sem,osWaitForever);
    
    DeviceConfig_t *config = (DeviceConfig_t *)sys_malloc(sizeof(DeviceConfig_t));
    if(config == NULL) {
        osSemaphoreRelease(flash_lock_sem);
        return errErr;
    }
    
    pDEApp->device_config.check_value = CRC_Get32((uint8_t *)(&pDEApp->device_config) + 4,sizeof(DeviceConfig_t) - 4);
    
    for(uint8_t i=0;i<6;i++) {
        
        storage_erasesector(ADDR_DEVICE_CONFIG);
        
        storage_writebytes(ADDR_DEVICE_CONFIG,(uint8_t *)&pDEApp->device_config,sizeof(DeviceConfig_t));

        storage_readbytes(ADDR_DEVICE_CONFIG,(uint8_t *)config,sizeof(DeviceConfig_t));
        
        if(!memcmp((void *)&pDEApp->device_config,(void *)config,sizeof(DeviceConfig_t))) {
            
            sys_free(config);

            osSemaphoreRelease(flash_lock_sem);
            
            return errOK;
        }
        
        osDelay(100);
    }
    
    sys_free(config);
    
    osSemaphoreRelease(flash_lock_sem);
    
    return errErr;
    
}

errStatus_t module_sn_clear(uint8_t i03t_addr);

void appl_para_factory(void) {
    
    memset(&pDEApp->device_config,0,sizeof(pDEApp->device_config));


    for(uint8_t i=0;i<CONFIG_MAX_485_PORT;i++) {
        pDEApp->device_config.i03m.rs485_cfg[i].baudrate.bits.baudrate = BAUDRATE_115200;
        pDEApp->device_config.i03m.rs485_cfg[i].protocol = PROTOCOL_TYPE_DE_MODBUS;
        pDEApp->device_config.i03m.rs485_cfg[i].port_type = PORT_485;
    }
        
    pDEApp->device_config.i03m.storage_period = 300;
    pDEApp->device_config.i03m.i03t_number = 0;
    pDEApp->device_config.i03m.cell_numbers = 0;
    
    pDEApp->device_config.i03m.i03m_addr = 1;
    
    pDEApp->device_config.i03m.i03t_time_synch = 1;
    
    strcpy((char *)pDEApp->device_config.i03m.mqtt_config.mqtt_addr,"47.108.193.148");
    pDEApp->device_config.i03m.mqtt_config.mqtt_port = 1883;
    pDEApp->device_config.i03m.mqtt_config.cloud_enable = 0;
    pDEApp->device_config.i03m.module_type = MODULE_4G_ZLG;
    pDEApp->device_config.i03m.mqtt_config.i03t_report_interval = 60;
    pDEApp->device_config.i03m.mqtt_config.cells_report_interval = 5;
    
    strcpy((char *)pDEApp->device_config.i03m.net_config.dns,"114.114.114.114");
    strcpy((char *)pDEApp->device_config.i03m.net_config.ipv4.ip,"192.168.1.100");
    strcpy((char *)pDEApp->device_config.i03m.net_config.ipv4.mask,"255.255.255.0");
    strcpy((char *)pDEApp->device_config.i03m.net_config.ipv4.gate,"192.168.1.1");
    
    for(int i=0;i<CONFIG_MAX_I03T;i++) {

       
        pDEApp->device_config.i03t_nodes[i].sample_period.period_cell_poll = 30;
        pDEApp->device_config.i03t_nodes[i].sample_period.period_total_volt = 1;
        pDEApp->device_config.i03t_nodes[i].sample_period.period_int_res = 7200;
        
        pDEApp->device_config.i03t_nodes[i].sys_para.cell_number = 0;
        pDEApp->device_config.i03t_nodes[i].base_para.bat_life = 10;
        
        pDEApp->device_config.i03t_nodes[i].charge_status_threshold.critical_current = 20;
        pDEApp->device_config.i03t_nodes[i].charge_status_threshold.fast_to_float = 50;
        pDEApp->device_config.i03t_nodes[i].charge_status_threshold.float_to_fast = 80;
        pDEApp->device_config.i03t_nodes[i].charge_status_threshold.charge_to_discharge = -20;
        
        pDEApp->device_config.i03t_nodes[i].dry_node.dry_switch = 0;
        pDEApp->device_config.i03t_nodes[i].dry_node.output_resume_threshold = 0;
        pDEApp->device_config.i03t_nodes[i].dry_node.out_threshold = 0;
        pDEApp->device_config.i03t_nodes[i].dry_node.delay = 0;
        pDEApp->device_config.i03t_nodes[i].dry_node.output_min_volt = 0;
        
        pDEApp->device_config.i03t_nodes[i].wireless.work_freq = 54;
        pDEApp->device_config.i03t_nodes[i].wireless.assist_freq = 99;
        
        pDEApp->device_config.i03t_nodes[i].cap.nominal_cap = 12000;
        pDEApp->device_config.i03t_nodes[i].cap.nominal_volt = 1;
        pDEApp->device_config.i03t_nodes[i].cap.float_volt = 2350;
        pDEApp->device_config.i03t_nodes[i].cap.temp_compensate = 2000;
        pDEApp->device_config.i03t_nodes[i].cap.charge_method = 0;
        pDEApp->device_config.i03t_nodes[i].cap.discharge_method = 0;
        pDEApp->device_config.i03t_nodes[i].cap.charge_efficiency = 1000;
        pDEApp->device_config.i03t_nodes[i].cap.max_discharge_cap = 13000;
        pDEApp->device_config.i03t_nodes[i].cap.cell_min_voltage = 1800;
        pDEApp->device_config.i03t_nodes[i].cap.cell_max_voltage = 2400;
        pDEApp->device_config.i03t_nodes[i].cap.discharge_max_current = -6000;
        pDEApp->device_config.i03t_nodes[i].cap.discharge_min_current = 0;
        pDEApp->device_config.i03t_nodes[i].cap.min_calculate_temp = -250;
        pDEApp->device_config.i03t_nodes[i].cap.max_calculate_temp = 750;
        pDEApp->device_config.i03t_nodes[i].cap.cap_init = 1;
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_1 = 4320;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_2 = 4464;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_3 = 4590;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_1_resume = 4230;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_2_resume = 4374;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_3_resume = 4500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.over.level_3_delay = 3;
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_1 = 3420;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_2 = 3240;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_3 = 3060;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_1_resume = 3600;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_2_resume = 3330;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_3_resume = 3150;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_voltage.low.level_3_delay = 3;  
        
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_1 = 1600;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_2 = 1800;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_3 = 2000;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_1_resume = 1500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_2_resume = 1700;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_3_resume = 1900;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_charge.level_3_delay = 3;
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_1 = -1600;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_2 = -1800;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_3 = -2000;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_1_resume = -1500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_2_resume = -1700;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_3_resume = -1900;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.total_current.over_discharge.level_3_delay = 3;
        
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_1 = 500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_2 = 550;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_3 = 600;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_1_resume = 470;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_2_resume = 520;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_3_resume = 570;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.temp.over.level_3_delay = 3;
        
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_1 = 300;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_2 = 200;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_3 = 100;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_1_resume = 350;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_2_resume = 250;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_3_resume = 150;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.soc.low.level_3_delay = 3;  
        
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_1 = 2400;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_2 = 2480;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_3 = 2550;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_1_resume = 2350;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_2_resume = 2430;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_3_resume = 2500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.over.level_3_delay = 3;
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_1 = 1900;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_2 = 1800;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_3 = 1700;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_1_resume = 1950;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_2_resume = 1850;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_3_resume = 1750;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.voltage.low.level_3_delay = 3;
        
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_1 = 500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_2 = 550;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_3 = 600;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_1_resume = 470;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_2_resume = 520;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_3_resume = 570;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.temp.over.level_3_delay = 3;
        
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_1 = 2000;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_2 = 3000;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_3 = 4000;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_1_resume = 1500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_2_resume = 2500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_3_resume = 3500;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_1_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_2_delay = 3;
        pDEApp->device_config.i03t_nodes[i].alarm_threshold.cell.res.over.level_3_delay = 3;
        
        pDEApp->device_config.i03t_nodes[i].sys_para.voltage_level = 3;
        pDEApp->device_config.i03t_nodes[i].sys_para.current_level = 0;
        pDEApp->device_config.i03t_nodes[i].sys_para.sn_length = 20;
            
        pDEApp->device_config.i03t_nodes[i].sys_para.alarm_max_level = 2;
        pDEApp->device_config.i03t_nodes[i].sys_para.cell_mode = 0;
        
        pDEApp->device_config.i03t_nodes[i].calib.total_voltage.zero_offset = 0;
        pDEApp->device_config.i03t_nodes[i].calib.total_voltage.kb.k = 1;
        pDEApp->device_config.i03t_nodes[i].calib.total_voltage.kb.b = 0;
        
        pDEApp->device_config.i03t_nodes[i].calib.current[0].zero_offset = 2050;
        pDEApp->device_config.i03t_nodes[i].calib.current[0].kb.k = 1;
        pDEApp->device_config.i03t_nodes[i].calib.current[0].kb.b = 0;
        
        pDEApp->device_config.i03t_nodes[i].calib.current[1].zero_offset = 0;
        pDEApp->device_config.i03t_nodes[i].calib.current[1].kb.k = 1;
        pDEApp->device_config.i03t_nodes[i].calib.current[1].kb.b = 0;
        
        pDEApp->device_config.i03t_nodes[i].calib.temp.zero_offset = 0;
        pDEApp->device_config.i03t_nodes[i].calib.temp.kb.k = 1;
        pDEApp->device_config.i03t_nodes[i].calib.temp.kb.b = 0;
        
        pDEApp->device_config.i03t_nodes[i].calib.voltage_5v.zero_offset = 0;
        pDEApp->device_config.i03t_nodes[i].calib.voltage_5v.kb.k = 1;
        pDEApp->device_config.i03t_nodes[i].calib.voltage_5v.kb.b = 0;
    } 
    
   
}

void appl_para_init(void) {
    
    flash_lock_sem = osSemaphoreNew(1,1,NULL);

    if(appl_para_load() != errOK) {
        appl_para_factory();  
    } 
//    pDEApp->device_config.i03m.mqtt_config.cloud_enable = 1;
//    pDEApp->device_config.i03m.rs485_cfg[0].port_type = PORT_4G;
}

bool appl_para_find_i03t_cloud_id(char *cloud_id) {
    for(uint8_t i=0;i<CONFIG_MAX_I03T;i++) {
        if(!memcmp(pDEApp->device_config.i03t_nodes[i].base_para.cloud_id,cloud_id,CONFIG_CLOUD_ID_LENGTH)) {
            return true;
        }
    }
    
    return false;
}


