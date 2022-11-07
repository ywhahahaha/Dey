#include "main.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "module_4g.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
#define TRANSFER_CELLS_ONCE  8


#define MSG_AT_RESP    8          
osMessageQueueId_t mid_MsgAtResp = NULL;

void Module4G_RxMsgHandle(LinkerMsg_t *msg);
osThreadId_t tid_thread_4g_tx;                        // thread id
#define MSG_OBJECTS    8
osMessageQueueId_t msgQ_4g_tx = NULL; 

void debug_sendbytes(uint8_t *pdata,uint16_t bytes);
errStatus_t Module4G_Register(void);
void thread_4g_process (void *argument);                   // thread function
errStatus_t protocol_msg_put(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length);


void module_4g_msg_put(ACTION_TYPE_t action,uint8_t *pdata,uint16_t length) {
    if(msgQ_4g_tx == NULL) return;
    
    LinkerMsg_t *msg = sys_malloc(sizeof(LinkerMsg_t));
    if(msg != NULL) {
        
        memset(msg,0,sizeof(LinkerMsg_t));
       
        msg->action = action;
        
        if(length > 0 && pdata != NULL) {
            msg->pdata = sys_malloc(length);
            if(msg->pdata != NULL) {
                memcpy(msg->pdata,pdata,length);
                msg->length = length;
            } else {
                sys_free(msg);
                return;
            }
        } 
        
        osStatus_t status;
        status = osMessageQueuePut(msgQ_4g_tx,&msg,osPriorityNormal,0);
        if(status != osOK) {
            if(msg->pdata != NULL) {
                sys_free(msg->pdata);
            }
            
            sys_free(msg);
        }
    }
}


 
errStatus_t thread_4g_init (void) { 
       
    if(!pDEApp->device_config.i03m.mqtt_config.cloud_enable) {
        return errErr; 
    }
    
    mid_MsgAtResp = osMessageQueueNew(MSG_AT_RESP,sizeof(void *),NULL);
    
    msgQ_4g_tx = osMessageQueueNew(MSG_OBJECTS,sizeof(void *),NULL);
 
    const osThreadAttr_t attr_thread = {
        .name = "thread_4g",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_4g_tx = osThreadNew(thread_4g_process, NULL, &attr_thread);
    if (tid_thread_4g_tx == NULL) {
        return (errErr);
    }

    return (errOK);
}
char *module_payload_make_i03t(uint8_t i03t_addr);
void AT_SendBytes(uint8_t *sendbytes,uint16_t length);
#define do_fun_try_times(fun,times)  \
    {\
        for(uint8_t i=0;i<times;i++) {\
            ret = fun(0);\
            if(ret == errOK) {\
                break;\
            }\
            osDelay(100);\
        }\
    }
    
errStatus_t appl_cloud_send_by_tas(void) {
    errStatus_t ret;
    do_fun_try_times(ModuleTAS_EnterConfig,3);
    
    if(ret != errOK) {
        return errErr;
    }
    
    //ModuleTAS_DTUPACKET(0);
    
    do_fun_try_times(ModuleTAS_CSQ,3);
    
    do_fun_try_times(ModuleTAS_TIME,1);
    
    do_fun_try_times(ModuleTAS_ASKCONNECT,3);
    if(ret != errOK) {
        pModule4GInfor->connect = 0;
    }
    
    do_fun_try_times(ModuleTAS_ExitConfig,6);
    
    osDelay(100);
    
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_I03T; i03t_addr++) {
        char *json = (char *)module_payload_make_i03t(i03t_addr);
        if(json != NULL) {
            if(pDEApp->Flag.bits.g4_trace) {
                debug_printf("I03T[%d]:===>%s\r\n",i03t_addr,json);
            }
            AT_SendBytes((uint8_t *)json,strlen(json));
            osDelay(5000);
            sys_free(json);
        }
    } 

    return ret;
}

errStatus_t appl_cloud_send_by_zlg(void) {
    errStatus_t ret;
    
    do_fun_try_times(ModuleZLG_GETDTUSTATE,3);
    if(ret != errOK) {
        pModule4GInfor->connect = 0;
        return errErr;
    }
    
    ModuleZLG_CSQ(0);
    
    ModuleZLG_TIME(0);
   
    osDelay(50);
    
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_I03T; i03t_addr++) {
        char *json = (char *)module_payload_make_i03t(i03t_addr);
        if(json != NULL) {
            if(pDEApp->Flag.bits.g4_trace) {
                debug_printf("I03T[%d]:===>%s\r\n",i03t_addr,json);
            }
            AT_SendBytes((uint8_t *)json,strlen(json));
            osDelay(5000);
            sys_free(json);
        }
    } 

    return ret;
}
    
errStatus_t appl_cloud_send_by_eth(void);    
errStatus_t appl_cloud_send_i03t(void) {

    switch(pDEApp->device_config.i03m.module_type) {
        case MODULE_4G_TAS:
            return appl_cloud_send_by_tas();
        case MODULE_4G_ZLG:
            return appl_cloud_send_by_zlg();
        case MODULE_ETH_TAS:
            return appl_cloud_send_by_eth();
    }
    
    return appl_cloud_send_by_zlg();
}

char *module_payload_make_cells(I03T_Info_t *i03t_node,uint16_t cell_id_start,uint16_t num);

errStatus_t app_cloud_send_cells_by_tas(void) {

    errStatus_t ret;
    do_fun_try_times(ModuleTAS_EnterConfig,3);
    
    if(ret != errOK) {
        return errErr;
    }
    
    do_fun_try_times(ModuleTAS_CSQ,3);

    do_fun_try_times(ModuleTAS_ASKCONNECT,3);
    if(ret != errOK) {
        pModule4GInfor->connect = 0;
    }
    
    do_fun_try_times(ModuleTAS_TIME,1);
    
    do_fun_try_times(ModuleTAS_ExitConfig,6);
    
    osDelay(100);
    
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_I03T; i03t_addr++) {
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL) {
            uint16_t cells = pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number;
            uint16_t cell_start = 1;
            do {
                char *json = (char *)module_payload_make_cells(i03t_node,cell_start,TRANSFER_CELLS_ONCE);
                if(json != NULL) {
                    AT_SendBytes((uint8_t *)json,strlen(json));
                    sys_free(json);
                    osDelay(5000);
                }
                cell_start += TRANSFER_CELLS_ONCE;
            } while(cell_start < cells);
        }
    }
    
    
    return ret;
}

errStatus_t appl_cloud_send_cells(void) {

    switch(pDEApp->device_config.i03m.module_type) {
        case MODULE_4G_TAS:
            return app_cloud_send_cells_by_tas();
        case MODULE_4G_ZLG:
            return errOK;
        case MODULE_ETH_TAS:
            //eth tas init.
            break;
    }
    
    return errOK;
}


void thread_4g_process (void *argument) {

    uint32_t module_4g_reset_tick;
    int32_t diff_value;
    uint8_t connect_status = 0;

    uint32_t i03t_tx_tick = 0;
    uint32_t cells_tx_tick = 0;
    
    uint8_t i03t_addr_poll = 1;
    uint16_t cell_id_poll = 1;
    
    uint16_t module_4g_cell_tx_cycle = MODULE_4G_CELL_TX_CYCLE;
    uint16_t module_4g_i03t_tx_cycle = MODULE_4G_I03T_TX_CYCLE;

    module_4g_cell_tx_cycle = (uint32_t)pDEApp->device_config.i03m.mqtt_config.cells_report_interval * 1000ul;
    if(module_4g_cell_tx_cycle < MODULE_4G_CELL_TX_CYCLE) {
        module_4g_cell_tx_cycle = MODULE_4G_CELL_TX_CYCLE;
    }
    
    module_4g_i03t_tx_cycle = (uint32_t)pDEApp->device_config.i03m.mqtt_config.i03t_report_interval * 1000ul;
    if(module_4g_i03t_tx_cycle < MODULE_4G_I03T_TX_CYCLE) {
        module_4g_i03t_tx_cycle = MODULE_4G_I03T_TX_CYCLE;
    }
    
    osDelay(5000);
    
    Module4G_Init();
    
    osDelay(2000);
    
    Module4G_Register();
    
    module_4g_reset_tick = osKernelGetTickCount();


    while(1) {
        uint32_t msg;
        osStatus_t status;
        
        status = osMessageQueueGet(msgQ_4g_tx,&msg,NULL,1000);
        
        if(status == osOK) {
            
            LinkerMsg_t *pMsg = (LinkerMsg_t *)msg;
           
            if(pMsg != NULL) {
                if(pModule4GInfor->connect) {
                    switch(pMsg->action) {
                        case ACTION_TYPE_REGISTER:
                            Module4G_Register();
                            break;
                        case ACTION_TYPE_TX_MSG1:    //real data.
                            appl_cloud_send_i03t();
                            break;
                        case ACTION_TYPE_TX_MSG2:    //cell data.
                            //appl_cloud_send_cells();
                            break;
                        case ACTION_TYPE_TX_MSG3:
                            break;
                        
                        default:
                            break;
                    }
                } 
                if(pMsg->pdata != NULL) {
                    sys_free(pMsg->pdata);
                }
                sys_free(pMsg);
            }
        } else {
            
            if(!pModule4GInfor->connect) {
                diff_value = (int32_t)(osKernelGetTickCount() - module_4g_reset_tick); 
                if(abs(diff_value) > MODULE_4G_RESET_TIME) {
                    Module4G_Register();
                    module_4g_reset_tick = osKernelGetTickCount();
                } else {
                    if(pDEApp->device_config.i03m.module_type == MODULE_4G_ZLG) {
                        ModuleZLG_GETDTUSTATE(0);
                    }
                }
            } else {
                
                module_4g_reset_tick = osKernelGetTickCount();
                
                //tx i0t 
                diff_value = (int32_t)(osKernelGetTickCount() - i03t_tx_tick); 
                if(abs(diff_value) >= module_4g_i03t_tx_cycle) {
                    i03t_tx_tick = osKernelGetTickCount();
                    
                    appl_cloud_send_i03t();
                    
                    module_4g_i03t_tx_cycle = (uint32_t)pDEApp->device_config.i03m.mqtt_config.i03t_report_interval * 1000ul;
                    if(module_4g_i03t_tx_cycle < MODULE_4G_I03T_TX_CYCLE) {
                        module_4g_i03t_tx_cycle = MODULE_4G_I03T_TX_CYCLE;
                    }
                    
                   
                    module_4g_cell_tx_cycle = (uint32_t)pDEApp->device_config.i03m.mqtt_config.cells_report_interval * 1000ul;
                    if(module_4g_cell_tx_cycle < MODULE_4G_CELL_TX_CYCLE) {
                        module_4g_cell_tx_cycle = MODULE_4G_CELL_TX_CYCLE;
                    }
   
                }
                
                
                diff_value = (int32_t)(osKernelGetTickCount() - cells_tx_tick); 
                if(abs(diff_value) >= module_4g_cell_tx_cycle) {
                    cells_tx_tick = osKernelGetTickCount();
                    I03T_Info_t *i03t_node = i03t_node_find(i03t_addr_poll);
                    if(i03t_node == NULL) {
                        i03t_addr_poll++;
                        if(i03t_addr_poll > CONFIG_MAX_I03T) {
                            i03t_addr_poll = 1;
                        }
                        cell_id_poll = 1;
                        continue;
                    } 
                    
                    char *json = (char *)module_payload_make_cells(i03t_node,cell_id_poll,TRANSFER_CELLS_ONCE);
                    if(json != NULL) {
                        if(pDEApp->Flag.bits.g4_trace) {
                            debug_printf("cells[%d,%d]==>%s\r\n",i03t_node->i03t_addr,cell_id_poll,json);
                        }
                        AT_SendBytes((uint8_t *)json,strlen(json));
                        sys_free(json);
                    }
                    
                    cell_id_poll += TRANSFER_CELLS_ONCE;
                    if(cell_id_poll > pDEApp->device_config.i03t_nodes[i03t_addr_poll-1].sys_para.cell_number) {
                        cell_id_poll = 1;
                        i03t_addr_poll++;
                        if(i03t_addr_poll > CONFIG_MAX_I03T) {
                            i03t_addr_poll = 1;
                        } 
                    }
                }
                 
            }
            
            if(pModule4GInfor->connect != connect_status) {
                connect_status = pModule4GInfor->connect;
                //logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_MQTT,connect_status,NULL,__FILE__,__LINE__);
            }
        }
    }
}

