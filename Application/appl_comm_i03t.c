#include "main.h"
#include "appl_comm_i03t.h"
#include "de_protocol.h"
#include "module_sn.h"
#include "logger.h"

errStatus_t i03t_module_request_upgrade(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length );
errStatus_t appl_comm_i03t_request_upgrade(COMM_TYPE_t commType,
                        uint8_t i03t_addr,
                        uint8_t dataId,
                        uint8_t *pdata,
                        uint16_t length,
                        uint16_t pack_no,
                        uint16_t timeout) {
    uint8_t *temp_buf = sys_malloc(1064);
    if(temp_buf == NULL) {
        return errErr;
    }
    uint16_t index = 0;

    temp_buf[index++] = dataId;
    if(pack_no > 0) {
        temp_buf[index++] = pack_no >> 8;
        temp_buf[index++] = pack_no;
        temp_buf[index++] = length >> 8;
        temp_buf[index++] = length;
    }

    memcpy(temp_buf + index,pdata,length);
    index += length;
                            

    i03t_module_request_upgrade(commType,
                                       i03t_addr,
                                       TAG_Upgrade,
                                       temp_buf,
                                       index);
                            
    sys_free(temp_buf);
                            
    return errOK;
}

errStatus_t appl_comm_i03t_request_intres_sample(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout) {
    uint8_t temp_buf[6] = {0};
    uint8_t index = 0;
    
    temp_buf[index++] = 0x00;
    temp_buf[index++] = i03t_addr;
    temp_buf[index++] = ControlDataId_SampleIntRes;
    temp_buf[index++] = 0;

    return i03t_module_control(commType,i03t_addr,TAG_Control,temp_buf,index,PROTOCOL_TRY_TIMES);
}

errStatus_t appl_comm_i03t_request_reset(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout) {
    uint8_t temp_buf[6] = {0};
    uint8_t index = 0;
    
    temp_buf[index++] = 0x00;
    temp_buf[index++] = i03t_addr;
    temp_buf[index++] = ControlDataId_Reset;
    temp_buf[index++] = 0;

    return i03t_module_control(commType,i03t_addr,TAG_Control,temp_buf,index,PROTOCOL_TRY_TIMES);
}

errStatus_t appl_comm_i03t_request_cell_data(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout) {
    
    uint8_t temp_buf[6] = {0};
    uint8_t index = 0;
    temp_buf[index++] = TAG_Status;
    temp_buf[index++] = i03t_addr;
    temp_buf[index++] = StatusDataId_CellInfo;
    return i03t_module_query(commType,i03t_addr,TAG_Status,temp_buf,index,1,PROTOCOL_COMMU_RESP_TIME_OUT_2);
}

errStatus_t appl_comm_i03t_request_status_data(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout) {
    uint8_t temp_buf[6] = {0};
    uint8_t index = 0;
    temp_buf[index++] = TAG_Status;
    temp_buf[index++] = i03t_addr;
    temp_buf[index++] = 0;
    return i03t_module_query(commType,i03t_addr,TAG_Status,temp_buf,index,PROTOCOL_TRY_TIMES,PROTOCOL_COMMU_RESP_TIME_OUT);
}


errStatus_t appl_comm_i03t_request_cell_alarmdata(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout) {
    uint8_t temp_buf[6] = {0};
    uint8_t index = 0;
    temp_buf[index++] = TAG_Status;
    temp_buf[index++] = i03t_addr;
    temp_buf[index++] = StatusDataId_CellAlarmInfo;
    return i03t_module_query(commType,i03t_addr,TAG_Status,temp_buf,index,PROTOCOL_TRY_TIMES,PROTOCOL_COMMU_RESP_TIME_OUT);
}

errStatus_t appl_comm_i03t_request_paradata(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout) {
    uint8_t temp_buf[6] = {0};
    uint8_t index = 0;
    temp_buf[index++] = TAG_Config;
    temp_buf[index++] = i03t_addr;
    temp_buf[index++] = 0;

    return i03t_module_query(commType,i03t_addr,TAG_Config,temp_buf,index,PROTOCOL_TRY_TIMES,PROTOCOL_COMMU_RESP_TIME_OUT);
}

errStatus_t appl_comm_i03t_request_sn(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout,uint8_t trytimes) {
    uint8_t temp_buf[6] = {0};
    uint8_t index = 0;
    if(i03t_addr > CONFIG_MAX_I03T || i03t_addr == 0) {
        return errErr;
    }
    temp_buf[index++] = TAG_Config;
    temp_buf[index++] = i03t_addr;
    temp_buf[index++] = ConfigDataId_AddSN;

    return i03t_module_query(commType,i03t_addr,TAG_Config,temp_buf,index,trytimes,PROTOCOL_COMMU_RESP_TIME_OUT_2);
}

uint16_t fill_query_config_i03t(uint8_t addr,uint8_t *p,uint8_t query);
errStatus_t appl_comm_i03t_request_config(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout)  {
    uint8_t *temp_buf = sys_malloc(1024);
    if(temp_buf == NULL) {
        return errErr;
    }
    uint16_t index = 0;
    
    index = fill_query_config_i03t(i03t_addr,temp_buf,0);

    errStatus_t err =  i03t_module_config(commType,i03t_addr,TAG_Config,temp_buf,index);
    
    sys_free(temp_buf);
    
    return err;
    
}

void appl_i03t_config_time(I03T_Info_t *i03t_node) {
    if(!i03t_node->flag.bits.i03t_synch_time) {
        return;
    }
    
    if(!pDEApp->device_config.i03m.i03t_time_synch) {
        return;
    }

    i03t_node->flag.bits.i03t_synch_time = 0;
    
    uint8_t *temp_buf = sys_malloc(128);
    if(temp_buf == NULL) {
        return;
    }

    uint16_t index = 0;

    temp_buf[index++] = 0;        //dataid
    temp_buf[index++] = i03t_node->i03t_addr;//
    
    temp_buf[index++] = ConfigDataId_Time;
    
    DATE_yymmddhhmmss_t now;
    Bsp_RtcGetTime(&now);
    
    temp_buf[index++] = now.bits.year;
    temp_buf[index++] = now.bits.month;
    temp_buf[index++] = now.bits.day;
    temp_buf[index++] = now.bits.hour;
    temp_buf[index++] = now.bits.min;
    temp_buf[index++] = now.bits.sec;


    i03t_module_config(COMM_I03T_PORT,i03t_node->i03t_addr,TAG_Config,temp_buf,index);
 
    sys_free(temp_buf);
    
    return;
    
}

errStatus_t appl_comm_i03t_request_synch_sn(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout)  {
    
    if(i03t_addr < 1 || i03t_addr > CONFIG_MAX_IO3T_MODBUS_ADDR) {
        return errErr;
    }
        
    uint8_t *temp_buf = sys_malloc(128);
    if(temp_buf == NULL) {
        return errErr;
    }
    errStatus_t err;
    uint16_t index = 0;

    temp_buf[index++] = 0;        //dataid
    temp_buf[index++] = i03t_addr;//i03M addr
    
    temp_buf[index++] = ConfigDataId_DelAllSN;
    memset(temp_buf + index,0xff,20);
    index += 20;

    err =  i03t_module_config(commType,i03t_addr,TAG_Config,temp_buf,index);
    if(err != errOK) {
        sys_free(temp_buf);
        return errErr;
    }
        
    for(int cell_id=1; cell_id <= CONFIG_MAX_CELL; cell_id++) {
        index = 0;
        
        SNStore_t * sn = (SNStore_t *)module_sn_get_by_index(i03t_addr,cell_id);
        if(sn == NULL) {
            break;
        }
        temp_buf[index++] = 0;//dataid
        temp_buf[index++] = i03t_addr;//i03M addr
        
        temp_buf[index++] = ConfigDataId_AddSN;
        temp_buf[index++] = cell_id >> 8;
        temp_buf[index++] = cell_id;
        
        temp_buf[index++] = sn->cell_on_current_group;
        
        memcpy(temp_buf + index,sn->sn,CONFIG_SN_LENGTH);
        index += 20;
        
        if(pDEApp->Flag.bits.print_flag) {
            char sn_temp[CONFIG_SN_LENGTH + 1] = {0};
            memcpy(sn_temp,sn->sn,CONFIG_SN_LENGTH);
            debug_printf("config I03T%d, cell:%d,SN:%s\r\n",i03t_addr,cell_id,sn_temp);
        }
        
        
        sys_free(sn);
    
        err = i03t_module_config(commType,i03t_addr,TAG_Config,temp_buf,index);
        if(err != errOK) {
            break;
        }
    }

    sys_free(temp_buf);
    
    return err;
    
}

void appl_i03t_intres_sample(I03T_Info_t *i03t_node) {
    if(!i03t_node->flag.bits.i03t_sample_res) {
        return;
    }
    if(i03t_node->i03t_addr > CONFIG_MAX_I03T || i03t_node->i03t_addr == 0) {
        return;
    }

    i03t_node->flag.bits.i03t_sample_res = 0;
    
    errStatus_t result = appl_comm_i03t_request_intres_sample(COMM_I03T_PORT,i03t_node->i03t_addr,0);
    
    debug_printf("appl_comm_i03t_request_intres_sample @I03T%d,result:%d\r\n",i03t_node->i03t_addr,result);
}

void appl_i03t_reset(I03T_Info_t *i03t_node) {
    if(!i03t_node->flag.bits.i03t_reset) {
        return;
    }
    if(i03t_node->i03t_addr > CONFIG_MAX_I03T || i03t_node->i03t_addr == 0) {
        return;
    }

    i03t_node->flag.bits.i03t_reset = 0;
    
    errStatus_t result = appl_comm_i03t_request_reset(COMM_I03T_PORT,i03t_node->i03t_addr,0);
    
    //debug_printf("appl_comm_i03t_request_reset @I03T%d,result:%d\r\n",i03t_node->i03t_addr,result);
}

void appl_i03t_sn_synch_process(I03T_Info_t *i03t_node) {
    errStatus_t status;
    if(i03t_node->i03t_addr > CONFIG_MAX_I03T || i03t_node->i03t_addr == 0) {
        return;
    }
    
    uint8_t i03t_index = i03t_node->i03t_addr - 1;
    
    if(i03t_node->flag.bits.synch_sn_need \
        || i03t_node->sn_request.sn_cnt != pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number \
        || i03t_node->flag.bits.synch_sn_force ) {
            

        int32_t tick_diff = (int32_t)(osKernelGetTickCount() - i03t_node->sn_request.sn_synch_time);
            
        if(abs(tick_diff) > SN_SYNCH_DELAY) {
            
            if(!i03t_node->flag.bits.synch_sn_force) {
                for(uint8_t i=0;i<6;i++) {
                    i03t_node->sn_request.sn_cnt = 0;
                    i03t_node->sn_request.sn_err_cnt = 0;
                    
                    status = appl_comm_i03t_request_sn(COMM_I03T_PORT,i03t_node->i03t_addr,0,1);
                    
                    if(i03t_node->sn_request.sn_err_cnt == 0 && i03t_node->sn_request.sn_cnt == pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number) {
                        i03t_node->flag.bits.synch_sn_need = 0;
                        
                        debug_printf("appl_comm_i03t_request_sn@I03T%d OK.\r\n",i03t_node->i03t_addr);
                        return;
                    } 
                    
                    osDelay(500);
                }
            }
            
            i03t_node->flag.bits.synch_sn_force = 0;
            i03t_node->sn_request.sn_cnt = 0;
            i03t_node->sn_request.sn_err_cnt = 0;

            debug_printf("Start synch SN @I03T:%d,SN count:%d\r\n",i03t_node->i03t_addr,pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number);
            
            status = appl_comm_i03t_request_synch_sn(COMM_I03T_PORT,i03t_node->i03t_addr,0);
            
            if(status == osOK) {
                //i03t_node->flag.bits.synch_sn_need = 0;
                debug_printf("Synch SN @I03T:%d Finish.\r\n",i03t_node->i03t_addr);
                
                logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_SYNCH_SN_I03T,i03t_node->i03t_addr,NULL,__FILE__,__LINE__);

            } else {
                i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
            }
        }
    } 
}
