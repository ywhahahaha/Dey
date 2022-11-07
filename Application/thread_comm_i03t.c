#include <stdbool.h>

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "appl_comm_i03t.h"
#include "i03t_list.h"
#include "de_protocol.h"
#include "main.h"
#include "logger.h"
#include "strTools.h"
#include "de_protocol_frame.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 osMessageQueueId_t protocol_master_handle = NULL;
#define PROTOCOL_MASTER_MSG_OBJECTS    16

osThreadId_t tid_thread_comm_i03t;                     
osThreadId_t tid_thread_comm_i03t_rx;  
osThreadId_t tid_thread_comm_i03t_protocol_master_process;  

void thread_comm_i03t_process (void *argument);  
void thread_comm_i03t_rx_process (void *argument); 
void thread_protocol_master_process (void *argument);


errStatus_t thread_comm_i03t_init (void) {
    
    const osThreadAttr_t attr_thread = {
        .name = "thread_commi03t",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_comm_i03t = osThreadNew(thread_comm_i03t_process, NULL, &attr_thread);
    if (tid_thread_comm_i03t == NULL) {
        return (errErr);
    }
    
    const osThreadAttr_t attr_thread_rx = {
        .name = "thread_i03t_rx",
        .stack_size = 2048,
        .priority = osPriorityHigh, 
    };
    
    tid_thread_comm_i03t_rx = osThreadNew(thread_comm_i03t_rx_process, NULL, &attr_thread_rx);
    if (tid_thread_comm_i03t_rx == NULL) {
        return (errErr);
    }
    
    const osThreadAttr_t attr_thread_protocol_master_process = {
        .name = "thread_master",
        .stack_size = 2048,
        .priority = osPriorityNormal1, 
    };
    
    tid_thread_comm_i03t_protocol_master_process = osThreadNew(thread_protocol_master_process, NULL, &attr_thread_protocol_master_process);
    if (tid_thread_comm_i03t_protocol_master_process == NULL) {
        return (errErr);
    }
    
    protocol_master_handle = osMessageQueueNew(PROTOCOL_MASTER_MSG_OBJECTS, sizeof(void *),NULL);

    return (errOK);
}



errStatus_t protocol_master_msg_put(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length) {
    
    if(protocol_master_handle == NULL) return errErr;
    
    ApplProto_t *msg = sys_malloc(sizeof(ApplProto_t));
    
    if(msg != NULL) {
        
        memset(msg,0,sizeof(ApplProto_t));
       
        msg->commType = commType;
        if(length > 0 && pdata != NULL) {
            msg->pdata = sys_malloc(length);
            if(msg->pdata != NULL) {
                memcpy(msg->pdata,pdata,length);
                msg->length = length;
            } else {
                sys_free(msg);
                return errErr;
            }
        } 
        
        osStatus_t status;
        status = osMessageQueuePut(protocol_master_handle,&msg,osPriorityNormal,0);
        if(status != osOK) {
            if(msg->pdata != NULL) {
                sys_free(msg->pdata);
            }
            
            sys_free(msg);
            return errErr;
        }
    }
    
    return errOK;
}

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
void de_protocol_master_process(COMM_TYPE_t type,uint8_t *pdate,uint16_t length);
de_protocol_t *de_protocol_check(uint8_t *p,uint16_t length);
void thread_comm_i03t_rx_process (void *argument) {
    
    uint8_t cnt = 0;
    
    while(1) {
        
        osDelay(PROTOCOL_RX_PULL_TIME);

        if(protocolRxMsg.length > 3 && osKernelGetTickCount() >= protocolRxMsg.tick) {
          /*  
            if(protocolRxMsg.buffer[protocolRxMsg.length - 1] != 0x16) {
                cnt++;
                if(cnt > 10) {
                    memset((void *)&protocolRxMsg,0,sizeof(protocolRxMsg)); 
                    cnt = 0;
                }
                continue;
            }
            cnt = 0;
           */ 
            
            de_protocol_t *pMsg = de_protocol_check(protocolRxMsg.buffer, protocolRxMsg.length);
            if(pMsg == NULL) {
                cnt++;
                if(cnt > 10) {
                    //pMsg = de_protocol_check(protocolRxMsg.buffer, protocolRxMsg.length);
                    memset((void *)&protocolRxMsg,0,sizeof(protocolRxMsg)); 
                    cnt = 0;
                }
                continue;
            }
            cnt = 0;

#if 0
            //void debug_sendbytes(uint8_t *pdata,uint16_t bytes);
            //char *dest = sys_malloc(2 * protocolRxMsg.length + 1);
            //STR_Hex2Str((char *)protocolRxMsg.buffer,protocolRxMsg.length,dest);
            //debug_sendbytes("==>%s\r\n",dest);
            //debug_sendbytes((char *)dest,strlen(dest));
            //sys_free(dest);
            
            debug_printf("====>22rx byte:%d\r\n",protocolRxMsg.length);
#endif          
            protocol_master_msg_put(COMM_I03T_PORT,protocolRxMsg.buffer,protocolRxMsg.length);
            memset((void *)&protocolRxMsg,0,sizeof(protocolRxMsg));  
        }
    }
}



void thread_protocol_master_process (void *argument) {

    while (1) {
        uint32_t msg;
        osStatus_t status;
       
        status = osMessageQueueGet(protocol_master_handle,&msg,NULL,osWaitForever);
        
        if(status == osOK) {
            ApplProto_t *pMsg = (ApplProto_t *)msg;
            if(pMsg != NULL && pMsg->pdata != NULL) {
                
#if 0
            char *dest = sys_malloc(2 * pMsg->length + 1);
            STR_Hex2Str((char *)pMsg->pdata,pMsg->length,dest);
            debug_printf("==>%s\r\n",dest);
            sys_free(dest);
#endif 
                de_protocol_master_process(pMsg->commType,pMsg->pdata,pMsg->length);
            }
            
            if(pMsg != NULL) {
                if(pMsg->pdata != NULL) {
                    sys_free(pMsg->pdata);
                }
                
                sys_free(pMsg);
            }

        } 
    }
}

void appl_i03t_sn_synch_process(I03T_Info_t *i03t_node);
void appl_i03t_intres_sample(I03T_Info_t *i03t_node);
void appl_i03t_reset(I03T_Info_t *i03t_node);
void appl_i03t_config_time(I03T_Info_t *i03t_node);

void thread_comm_i03t_process (void *argument) {
    
    uint16_t timeout = 1000;
    errStatus_t status;
    
    osDelay(5000);
    
    const uint32_t real_data_pull_time = 1000;
    uint32_t cell_data_pull_time = 20000;

    while (1) {
        
        cell_data_pull_time = (uint32_t)pDEApp->device_config.i03t_nodes[0].sample_period.period_cell_poll * 1000u;
        if(cell_data_pull_time == 0) {
            cell_data_pull_time = 20000;
        } else {
            if(cell_data_pull_time > 10000) {
                cell_data_pull_time = cell_data_pull_time / 2;
            }
        }
        
        osDelay(real_data_pull_time); 

        Bsp_RtcGetTime(&pDEApp->now);        

        for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {
            
            I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
            
            if(i03t_node != NULL) {
                //  para synch.
                if(i03t_node->flag.bits.synch_config && !i03t_node->flag.bits.i03t_comm_err) {
                    
                    status = appl_comm_i03t_request_status_data(COMM_I03T_PORT,i03t_addr,timeout);
                    
                    if(status == osOK) {

                        memcpy(&i03t_node->hist.total_info.discharge,&i03t_node->discharge,sizeof(Discharge_t));

                        i03t_node->alarm.alarm.bat_group_alarm2 &= ~(1u << I03T_GROUP2_ALARM_COMM_ERR_POS);

                        int32_t tick_diff = (int32_t)(osKernelGetTickCount() - i03t_node->cell_data_pull_time_tick);
                        
                        if(abs(tick_diff) > cell_data_pull_time \
                            || !i03t_node->flag.bits.i03t_comm_once) {
                                
                            for(uint8_t trytimes = 0;trytimes < 3;trytimes++) {
                                i03t_node->hist_cell_cnt = 0;
                                status = appl_comm_i03t_request_cell_data(COMM_I03T_PORT,i03t_addr,timeout);
                                if(status != errOK || i03t_node->hist_cell_cnt < pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number) {
                                    continue;
                                } else {
                                    break;
                                }
                            }
   
                            status = appl_comm_i03t_request_cell_alarmdata(COMM_I03T_PORT,i03t_addr,timeout);
                                
                            if(status == errOK) {
                                bool cell_alarm = false;
                                for(uint16_t i=0; i < pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number;i++) {
                                    if(i03t_node->alarm.alarm.cell_alarm[i]) {
                                        cell_alarm = true;
                                        break;
                                    }
                                }
                                
                                i03t_node->flag.bits.cell_alarm = cell_alarm;   
                            }
                            
                            i03t_node->cell_data_pull_time_tick = osKernelGetTickCount();

                        }
                            
                         if(memcmp(&i03t_node->alarm.alarm,&pDEApp->alarm[i03t_addr-1],sizeof(AlarmInfo_t))) {
                             memcpy(&i03t_node->alarm.total_info,&i03t_node->hist.total_info,sizeof(TotalInfo_t));
                             
                             storage_msg_put(i03t_addr,
                                            NULL,
                                            0,
                                            StorageAlarmData);
                            
                             memcpy(&pDEApp->alarm[i03t_addr-1],&i03t_node->alarm.alarm,sizeof(AlarmInfo_t));
                        }
                            
                        appl_i03t_sn_synch_process(i03t_node);
                        
                        appl_i03t_intres_sample(i03t_node);
                        
                        appl_i03t_reset(i03t_node);
                        
                        appl_i03t_config_time(i03t_node);
                        
                        i03t_node->flag.bits.i03t_comm_once = 1;
                        
                        i03t_node->comm_err_cnt = 0;
                        
                        i03t_node->sample_time.date = pDEApp->now.date;

                    } else {
                        
                        i03t_node->flag.bits.i03t_comm_err = 1;
                        
                        i03t_node->comm_err_cnt++;
                        
                        i03t_node->alarm.alarm.bat_group_alarm2 |= (1u << I03T_GROUP2_ALARM_COMM_ERR_POS);

                        if(memcmp(&i03t_node->alarm.alarm,&pDEApp->alarm[i03t_addr-1],sizeof(AlarmInfo_t))) {
                            memcpy(&i03t_node->alarm.total_info,&i03t_node->hist.total_info,sizeof(TotalInfo_t));
                            storage_msg_put(i03t_addr,
                                            NULL,
                                            0,
                                            StorageAlarmData);
                            
                            memcpy(&pDEApp->alarm[i03t_addr-1],&i03t_node->alarm.alarm,sizeof(AlarmInfo_t));
                        }  
                    }
 
                } else {
                    
                    i03t_node->flag.bits.synch_config = 0;
                    
                    status = appl_comm_i03t_request_paradata(COMM_I03T_PORT,i03t_addr,timeout);
                    
                    if(status == osOK) { 
                        
                        i03t_node->flag.bits.i03t_comm_err = 0;
                        
                        i03t_node->comm_err_cnt = 0;
                        
                        if(!i03t_node->flag.bits.synch_config) {
                            
                            debug_printf("appl_comm_i03t_request_config start @I03T%d\r\n",i03t_addr);
                            
                            status = appl_comm_i03t_request_config(COMM_I03T_PORT,i03t_addr,timeout);
                            
                            if(status == errOK) {
                                debug_printf("appl_comm_i03t_request_config@I03T%d OK\r\n",i03t_addr);
                                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_SYNCH_PARA_I03T,i03t_node->i03t_addr,NULL,__FILE__,__LINE__);
                            }
                        }
                        
                        if(!i03t_node->flag.bits.i03t_comm_once) {
                            
                            i03t_node->sn_request.sn_cnt = 0;
                            
                            i03t_node->sn_request.sn_err_cnt = 0;
                            
                            appl_comm_i03t_request_sn(COMM_I03T_PORT,i03t_addr,timeout,1);
                        }

                    } else {
                        
                        i03t_node->flag.bits.i03t_comm_err = 1;

                        i03t_node->alarm.alarm.bat_group_alarm2 |= (1u << I03T_GROUP2_ALARM_COMM_ERR_POS);

                        if(memcmp(&i03t_node->alarm.alarm,&pDEApp->alarm[i03t_addr-1],sizeof(AlarmInfo_t))) {
                            memcpy(&i03t_node->alarm.total_info,&i03t_node->hist.total_info,sizeof(TotalInfo_t));
                            storage_msg_put(i03t_addr,
                                            NULL,
                                            0,

                            StorageAlarmData);
                            
                            memcpy(&pDEApp->alarm[i03t_addr-1],&i03t_node->alarm.alarm,sizeof(AlarmInfo_t));
                        } 

                        i03t_node->comm_err_cnt++;
                        
                        if(i03t_node->comm_err_cnt > 10) {
                            
                            i03t_node->comm_err_cnt = 0;

                            errStatus_t result = appl_comm_i03t_request_reset(COMM_I03T_PORT,i03t_node->i03t_addr,0);
                            
                            i03t_node->sn_request.sn_cnt = 0;
                            i03t_node->sn_request.sn_err_cnt = 0;
                            appl_comm_i03t_request_sn(COMM_I03T_PORT,i03t_addr,timeout,1);
                        }
                    }
                }
            }
        }         
    }
}
