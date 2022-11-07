#include <time.h>

#include "main.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "module_soc.h"
#include "App_soc.h"

#define SOC_ENABLE  0
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_thread_soc;                        // thread id
 
void thread_soc_process(void *argument);                   // thread function
 
errStatus_t thread_soc_init (void) {
     const osThreadAttr_t attr_thread = {
        .name = "thread_soc",
        .stack_size = 2048,
        .priority = osPriorityHigh, 
    };
     

    tid_thread_soc = osThreadNew(thread_soc_process, NULL, &attr_thread);
    if (tid_thread_soc == NULL) {
        return (errErr);
    }
    

    return (errOK);
}


void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);

void discharge_store_process(void) {    
    
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL && !i03t_node->mount) {
            if(i03t_node->discharge.status != DISCHARGE_STATE)   {              
                i03t_node->discharge_store_time = osKernelGetTickCount();
            } else {
                int32_t tick_diff = (int32_t)(osKernelGetTickCount() - i03t_node->discharge_store_time);
                 if(abs(tick_diff) > FILE_DISCHARGE_STORE_TIME ) {
                    i03t_node->discharge_store_time = osKernelGetTickCount();
                    storage_msg_put(i03t_addr,NULL,0,StorageDischarge);    
                }
            }
        }
    }
}

void charge_store_process(void) {    
    
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL && !i03t_node->mount) {
            if(i03t_node->discharge.status != CHARGE_STATE)   {              
                i03t_node->charge_store_time = osKernelGetTickCount();
            } else {
                
                int32_t tick_diff = (int32_t)(osKernelGetTickCount() - i03t_node->charge_store_time);
                if(abs(tick_diff) > FILE_DISCHARGE_STORE_TIME ) {
                    i03t_node->charge_store_time = osKernelGetTickCount();
                    i03t_node->charge.voltage = i03t_node->discharge.voltage;
                    i03t_node->charge.current = i03t_node->discharge.current[0];
                    i03t_node->charge.temp = i03t_node->discharge.temp;
                    i03t_node->charge.soc = i03t_node->discharge.soc;
                    i03t_node->charge.soh = i03t_node->discharge.soh;
                    i03t_node->charge.available_time = i03t_node->discharge.available_time;

                    storage_msg_put(i03t_addr,NULL,0,StorageCharge);    
                }
            }
        }
    }
}



#define MAX_CACHE_CNT  10
typedef struct {
    uint16_t soc;
    float  current;
    float  voltage;
    int16_t  _current[MAX_CACHE_CNT];
    int16_t  _voltage[MAX_CACHE_CNT];
    uint8_t  index;
    uint8_t  cnt;
    int32_t  sum_current;
    int32_t  sum_voltage;
    uint32_t last_time;
}SocCalc_t;

osMessageQueueId_t socMsgId = NULL;

typedef struct {
    uint32_t i03t_addr : 8;
    uint32_t command   : 8;
    uint32_t spare     : 16;
}SOCMsgBit_t;

typedef union {
    SOCMsgBit_t bit;
    uint32_t    all;
}SOCMsg_t;

void soc_msg_put(uint8_t i03t_addr,uint8_t command) {
    
    if(socMsgId == NULL) return;
    
    SOCMsg_t msg = {0};
    
    msg.bit.i03t_addr = i03t_addr;
    msg.bit.command = command;
    osMessageQueuePut(socMsgId,&msg,osPriorityNormal,0);
}

void soh_dec_handle(void) {
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {    
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL && i03t_node->flag.bits.i03t_soc_init) {
            uint16_t temp = multiple(App_soh_process(i03t_addr),1);
            if(temp > 1000) {
                temp = 1000;
            }
            i03t_node->discharge.soh = temp;
        }
    }
    
    storage_msg_put(0,NULL,0,StorageFileIndex);
}


#define soc_printf(...)    if(pDEApp->Flag.bits.soc_trace)  {debug_printf(__VA_ARGS__);}



void thread_soc_process(void *argument) {
    
    
    socMsgId = osMessageQueueNew(16,sizeof(SOCMsg_t),NULL);

    SocCalc_t _cache[CONFIG_MAX_I03T] = {0};
    
    bool updata;
    
    //uint32_t tick1;
    //uint32_t tick2;
    //uint32_t tick3;
    
    while(1) {

        uint32_t msg;
        osStatus_t status;
        status = osMessageQueueGet(socMsgId,&msg,NULL,1000);
        if(status == osOK) {
            SOCMsg_t socMsg;
            socMsg.all = msg;
            switch(socMsg.bit.command) {
                case I03T_CREATE://create 
                    break;
                case I03T_DELETE://
                    App_soc_terminate(socMsg.bit.i03t_addr);
                    break;
                
                case I03T_SOH_DEC:
                    break;
                
                case I03T_SOH_INIT:{
                        I03T_Info_t *i03t_node = i03t_node_find(socMsg.bit.i03t_addr);
                        if(i03t_node != NULL) {
                            App_soh_intial( pDEApp->device_config.i03t_nodes[socMsg.bit.i03t_addr-1].base_para.bat_life,
                                    multiple(i03t_node->discharge.soh,1),
                                    i03t_node->discharge.deep_discharge_cycle,
                                    socMsg.bit.i03t_addr);
                            storage_msg_put(0,NULL,0,StorageFileIndex);
                        }
                    }
                    break;
                
                default:
                    break;
            }
        } 
        
        updata = false;
                
        for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {
            
            I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
            
            if(i03t_node != NULL && !i03t_node->mount) {
                
                uint8_t i03t_index = i03t_addr - 1;

                if(i03t_node->discharge.voltage == 0 || !i03t_node->flag.bits.i03t_comm_once) {
                    continue;
                }

                _cache[i03t_index]._current[_cache[i03t_index].index] = i03t_node->discharge.current[0];
                _cache[i03t_index]._voltage[_cache[i03t_index].index] = i03t_node->discharge.voltage;
                _cache[i03t_index].index++;
                if(_cache[i03t_index].index >= MAX_CACHE_CNT) {
                    _cache[i03t_index].index = 0;
                }
                
                if(_cache[i03t_index].cnt < MAX_CACHE_CNT) {
                    _cache[i03t_index].cnt++;
                }
                
                _cache[i03t_index].sum_current = 0;
                _cache[i03t_index].sum_voltage = 0;
                for(uint8_t i=0;i<_cache[i03t_index].cnt;i++) {
                    _cache[i03t_index].sum_voltage += _cache[i03t_index]._voltage[i];
                    _cache[i03t_index].sum_current += _cache[i03t_index]._current[i];
                }
                
                _cache[i03t_index].current = (float)_cache[i03t_index].sum_current/_cache[i03t_index].cnt;
                _cache[i03t_index].voltage = (float)_cache[i03t_index].sum_voltage/_cache[i03t_index].cnt;
                
                float voltage;
                //uint16_t temp_voltage;
                if(pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number > 0) {
                    voltage = multiple((float)_cache[i03t_index].voltage / pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number,0.1f);
                } else {
                    voltage = multiple((float)_cache[i03t_index].voltage,0.1f);
                }

                //float current = multiple(i03t_node->discharge.current[0],0.1f);
                float current = multiple(_cache[i03t_index].current,0.1f);
                
                float temp = multiple(i03t_node->discharge.temp,0.1f);
                            
                uint8_t nominal_volt = pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.nominal_volt;
                if(nominal_volt == 0) {
                    nominal_volt = 1;
                }
                
                if(!i03t_node->flag.bits.i03t_soc_init) {
                    DATE_yymmddhhmmss_t now;
                    Bsp_RtcGetTime(&now);

                    struct tm time1;
                    uint32_t sec1;
                    uint32_t sec2;
                    
                    time1.tm_year = i03t_node->soc_store_time.bits.year + 2000 - 1900;
                    time1.tm_mon = i03t_node->soc_store_time.bits.month - 1;
                    time1.tm_mday = i03t_node->soc_store_time.bits.day;
                    time1.tm_hour = i03t_node->soc_store_time.bits.hour;
                    time1.tm_min = i03t_node->soc_store_time.bits.min;
                    time1.tm_sec = i03t_node->soc_store_time.bits.sec;
                    time1.tm_isdst = 0;
                    sec1 = mktime(&time1);
                    
                    time1.tm_year = now.bits.year + 2000 - 1900;
                    time1.tm_mon = now.bits.month - 1;
                    time1.tm_mday = now.bits.day;
                    time1.tm_hour = now.bits.hour;
                    time1.tm_min = now.bits.min;
                    time1.tm_sec = now.bits.sec;
                    time1.tm_isdst = 0;
                    sec2 = mktime(&time1);
                    
                    int32_t diff = sec2 - sec1;
                    uint8_t soc_valid = false;
                    if(abs(diff) < 360) {
                        soc_valid = true;
                    }
                    
#if 0
                    App_soc_intial(multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.nominal_cap,0.1f), 
                                   multiple(i03t_node->discharge.soc,1),
                                   voltage,
                                   multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.float_volt,0.001f),
                                   multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.cell_max_voltage,0.001f),
                                   multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.cell_min_voltage,0.001f),
                                   i03t_node->discharge.status,
                                   i03t_addr,
                                   soc_valid );
#else

                    
                    App_soc_intial(multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.nominal_cap,0.1f), 
                                   multiple(i03t_node->discharge.soc,1),
                                   voltage/nominal_volt,
                                   multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.float_volt,0.001f)/nominal_volt,
                                   multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.cell_max_voltage,0.001f)/nominal_volt,
                                   multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.cell_min_voltage,0.001f)/nominal_volt,
                                   i03t_node->discharge.status,
                                   i03t_addr,
                                   soc_valid );
#endif
                    char infoStr[40] = {0};
                    sprintf(infoStr,"I03T%d|SOC:%d |Flag:%d |V:%.3f|S:%d",i03t_addr,\
                                                                i03t_node->discharge.soc,\
                                                                soc_valid,\
                                                                voltage,\
                                                                i03t_node->discharge.status);
                    
                    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_SOC_INIT,0,infoStr,__FILE__,__LINE__);

                    
                    soc_printf("App_soc_intial(Cap:%5.3f,(SOC:%4d,%d),V:%2.3f,F:%2.3f,Max:%2.3f,Min:%2.3f,S:%d,Id:%d)\r\n",multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.nominal_cap,0.1f), 
                                                                               multiple(i03t_node->discharge.soc,1),soc_valid,
                                                                               voltage,
                                                                               multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.float_volt,0.001f),
                                                                               multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.cell_max_voltage,0.001f),
                                                                               multiple(pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.cell_min_voltage,0.001f),
                                                                               i03t_node->discharge.status,
                                                                               i03t_addr);
                    
                    App_soh_intial( pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.bat_life,
                                    multiple(i03t_node->discharge.soh,1),
                                    i03t_node->discharge.deep_discharge_cycle,
                                    i03t_addr);
                                    
                    updata = true;
                }
                
                i03t_node->discharge.available_time = multiple(App_standby_time(i03t_addr,current,i03t_node->discharge.status),1);  
                
                if(App_deep_cycle_time(&i03t_node->discharge.deep_discharge_cycle,
                                    _cache[i03t_index].current,
                                    i03t_node->discharge.temp,
                                    i03t_node->discharge.status,
                                    i03t_addr)) {
                    updata = true;
                }
                
                i03t_node->discharge.soc = App_soc_process(voltage/nominal_volt,current,temp,1,i03t_node->discharge.status,i03t_addr); 

                soc_printf("App_soc_process(V:%6.3f,I:%9.3f,T:%7.3f,%d,S:%d,Id:%d,H:%5d,soc:%4d,soh:%d,cycle:%.3f)\r\n",voltage,
                                                                                              current,
                                                                                              temp,
                                                                                              1,
                                                                                              i03t_node->discharge.status,
                                                                                              i03t_addr,
                                                                                              i03t_node->discharge.available_time,  
                                                                                              i03t_node->discharge.soc,
                                                                                              i03t_node->discharge.soh,
                                                                                              i03t_node->discharge.deep_discharge_cycle);

                int16_t soc_temp = (int16_t)(i03t_node->discharge.soc - _cache[i03t_index].soc);
                int32_t dif_time = (pDEApp->now.date >> 6) - (_cache[i03t_index].last_time >> 6);
                if(abs(soc_temp) > 9 || abs(dif_time) >= 5) {
                    _cache[i03t_index].soc = i03t_node->discharge.soc;
                    _cache[i03t_index].last_time = pDEApp->now.date;
                    Bsp_RtcGetTime(&i03t_node->soc_store_time);
                    updata = true;
                }  
                
                i03t_node->flag.bits.i03t_soc_init = 1;
            }
        }
        
        if(updata) {
            storage_msg_put(0,NULL,0,StorageFileIndex);
        }
        
        discharge_store_process();
        
        charge_store_process();
        
        //tick2 = osKernelGetTickCount();
        //tick3 = tick2 - tick1;
        //debug_printf("tick3:%d\r\n",tick3);
    }
}




