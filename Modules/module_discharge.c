#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "cmsis_os2.h"
#include "rl_fs.h"
#include "main.h"
#include "crc_check.h"
#include "i03t_list.h"
#include "module_discharge.h"
#include "de_protocol.h"
#include "thread_debug.h"

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
//---------------------------charge---------------------------------------------------------------
errStatus_t module_charge_store(I03T_Info_t *i03t) {
    FILE *fp = NULL;
    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }

    Charge_t *charge = sys_malloc(sizeof(Charge_t));
    if(charge == NULL) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));
    
    uint16_t write_index = (i03t->charge.index % FILE_DISCHARGE_DATA_MAX_CNT);
    uint32_t write_addr = write_index * sizeof(Charge_t);
    
    sprintf(path,"charge%d.txt",i03t->i03t_addr);
    fp = fopen(path,"r+");
    if(fp == NULL) {
        sys_free(charge);
        storage_msg_put(i03t->i03t_addr,NULL,0,StorageFileCheck);
        return errErr;
    }
 
    fseek(fp,write_addr,SEEK_SET);  
    
    memcpy(charge,&i03t->charge,sizeof(Charge_t));
    
    Bsp_RtcGetTime((DATE_yymmddhhmmss_t *)&charge->time);
  
    charge->crc_chk = CRC_Get32((uint8_t *)(charge) + 4,sizeof(Charge_t) - 4);
    
    size_t size = fwrite(charge,sizeof(Charge_t),1,fp);
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);
    
    sys_free(charge);
    
    i03t->charge.index++;

    return (size == 1) ? errOK : errErr;

}


errStatus_t module_charge_load(I03T_Info_t *i03t) {
    FILE *fp = NULL;
    
    bool flag = false;
    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"charge%d.txt",i03t->i03t_addr);
    
    Charge_t *charge = sys_malloc(sizeof(Charge_t));
    if(charge == NULL) {
        return errErr;
    }
    memset(charge,0,sizeof(Charge_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(charge);
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 
    
    uint32_t tick = osKernelGetTickCount();
#if FILL_FILE
    uint8_t ff_cnt = 0;
#endif
    
    while(1) {
        feed_watchdog(); 
        memset(charge,0,sizeof(Charge_t));
        size_t size = fread(charge,sizeof(Charge_t),1,fp);
         
        if(size == 1 ) {
            uint32_t check = CRC_Get32(((uint8_t *)(charge) + 4),sizeof(Charge_t) - 4);
            if(check == charge->crc_chk ) {
                if(charge->index > i03t->charge.index) {
                    memcpy(&i03t->charge,charge,sizeof(Charge_t));
                }
#if FILL_FILE
                ff_cnt = 0;
#endif
                flag = true;
            } else {
#if FILL_FILE 
                if(charge->crc_chk == 0xffffffff) {
                    ff_cnt++;
                    if(ff_cnt > 3) {
                        break;
                    }
                }
#endif
            }
        } else {
            break;
        }
        
        int32_t tick_diff = (int32_t)(osKernelGetTickCount() - tick);
        
        if(abs(tick_diff) > FILE_DELAY) {
            tick = osKernelGetTickCount();
            osDelay(5);
        }
    }
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);   

    sys_free(charge);
    
    if(flag) {
        i03t->charge.index++;
    }

    return (flag) ? errOK : errErr;
}

//---------------------------discharge------------------------------------------------------------
errStatus_t module_discharge_store(I03T_Info_t *i03t) {
    FILE *fp = NULL;
    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    
    Discharge_t *discharge = sys_malloc(sizeof(Discharge_t));
    if(discharge == NULL) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));
    
    uint16_t write_index = (i03t->discharge.index % FILE_DISCHARGE_DATA_MAX_CNT);
    uint32_t write_addr = write_index * sizeof(Discharge_t);
    
    sprintf(path,"dis%d.txt",i03t->i03t_addr);
    fp = fopen(path,"r+");
    if(fp == NULL) {
        sys_free(discharge);
        storage_msg_put(i03t->i03t_addr,NULL,0,StorageFileCheck);
        return errErr;
    }
 
    fseek(fp,write_addr,SEEK_SET);  
    
    memcpy(discharge,&i03t->discharge,sizeof(Discharge_t));
    
    Bsp_RtcGetTime((DATE_yymmddhhmmss_t *)&discharge->time);
  
    discharge->crc_chk = CRC_Get32((uint8_t *)(discharge) + 4,sizeof(Discharge_t) - 4);
    
    size_t size = fwrite(discharge,sizeof(Discharge_t),1,fp);
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);
    
    sys_free(discharge);
    
    i03t->discharge.index++;

    return (size == 1) ? errOK : errErr;

}

errStatus_t module_discharge_load(I03T_Info_t *i03t) {
    FILE *fp = NULL;
    
    bool flag = false;
    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"dis%d.txt",i03t->i03t_addr);
    
    Discharge_t *discharge = sys_malloc(sizeof(Discharge_t));
    if(discharge == NULL) {
        return errErr;
    }
    memset(discharge,0,sizeof(Discharge_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(discharge);
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 
    
    uint32_t tick = osKernelGetTickCount();
#if FILL_FILE
    uint8_t ff_cnt = 0;
#endif
    
    while(1) {
        feed_watchdog(); 
        memset(discharge,0,sizeof(Discharge_t));
        size_t size = fread(discharge,sizeof(Discharge_t),1,fp);
         
        if(size == 1 ) {
            uint32_t check = CRC_Get32(((uint8_t *)(discharge) + 4),sizeof(Discharge_t) - 4);
            if(check == discharge->crc_chk ) {
                if(discharge->index > i03t->discharge.index) {
                    memcpy(&i03t->discharge,discharge,sizeof(Discharge_t));
                }
#if FILL_FILE
                ff_cnt = 0;
#endif
                flag = true;
            } else {
#if FILL_FILE 
                if(discharge->crc_chk == 0xffffffff) {
                    ff_cnt++;
                    if(ff_cnt > 3) {
                        break;
                    }
                }
#endif
            }
        } else {
            break;
        }
        
        int32_t tick_diff = (int32_t)(osKernelGetTickCount() - tick);
        
        if(abs(tick_diff) > FILE_DELAY) {
            tick = osKernelGetTickCount();
            osDelay(5);
        }
    }
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);   

    sys_free(discharge);
    
    if(flag) {
        i03t->discharge.index++;
    }

    return (flag) ? errOK : errErr;
}



int32_t module_discharge_get_index(FILE *fp,uint32_t file_pos,Discharge_t *discharge) {
    
    memset(discharge,0,sizeof(Discharge_t));

	if(fseek(fp,file_pos,SEEK_SET)) {
		return -1;
	}
        
    size_t size = fread(discharge,sizeof(Discharge_t),1,fp);
    
    if(size == 1 ) {
        uint32_t check = CRC_Get32(((uint8_t *)(discharge) + 4),sizeof(Discharge_t) - 4);
        if(check == discharge->crc_chk ) {
            return discharge->index;
        } 
    }
    
    return -1;
}


errStatus_t module_discharge_index_check(I03T_Info_t *i03t) {
    FILE *fp = NULL;

    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    if(i03t->discharge.index == 0) {
        return errErr;
    }

    uint16_t _index = ((i03t->discharge.index-1) % FILE_DISCHARGE_DATA_MAX_CNT);
    int32_t file_pos = _index * sizeof(Discharge_t);
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"dis%d.txt",i03t->i03t_addr);
    
    Discharge_t *discharge = sys_malloc(sizeof(Discharge_t));
    if(discharge == NULL) {
        return errErr;
    }
    memset(discharge,0,sizeof(Discharge_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(discharge);
        return errErr;
    }
    
    int32_t index = module_discharge_get_index(fp,file_pos,discharge);
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);   

    sys_free(discharge);
    
    if(index < 0) {
        return errErr;
    }

    return i03t->discharge.index >= index ? errOK : errErr;

}

errStatus_t module_discharge_load_protocol(uint8_t i03t_addr,LoggerQuery_t *query) {
    FILE *fp = NULL;
    
    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"dis%d.txt",i03t_addr);
    
    Discharge_t *discharge = sys_malloc(sizeof(Discharge_t));
    if(discharge == NULL) {
        return errErr;
    }
    memset(discharge,0,sizeof(Discharge_t));
    
    uint8_t *proto_tx_buf = sys_malloc(64);
    if(proto_tx_buf == NULL) {
        sys_free(discharge);
        return errErr;
    }
    
    uint8_t *p = sys_malloc(64);
    if(p == NULL) {
        sys_free(discharge);
        sys_free(proto_tx_buf);
        return errErr;
    }
    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(discharge);
        sys_free(p);
        sys_free(proto_tx_buf);
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 
    
    uint32_t tick = osKernelGetTickCount();
    
    uint8_t find = false;
    uint16_t index = 0;
#if FILL_FILE
    uint8_t ff_cnt = 0;
#endif
    while(1) {
        
        feed_watchdog(); 
        
        memset(discharge,0,sizeof(Discharge_t));
        
        size_t size = fread(discharge,sizeof(Discharge_t),1,fp);
        
        if(size == 1 ) {
            uint32_t check = CRC_Get32(((uint8_t *)(discharge) + 4),sizeof(Discharge_t) - 4);
            if(check == discharge->crc_chk ) {
                if(query->print){
                    DATE_yymmddhhmmss_t date;
                    date.date = discharge->time;
                    debug_printf("%04d-%02d-%02d %02d:%02d:%02d Volt:%05d,Current:%05d,Temp:%04d,SOC:%04d,SOH:%04d,Status:%d\r\n",\
                                                        date.bits.year + 2000,date.bits.month,date.bits.day,\
                                                        date.bits.hour,date.bits.min,date.bits.sec,\
                                                        discharge->voltage,\
                                                        discharge->current,\
                                                        discharge->temp,\
                                                        discharge->soc,\
                                                        discharge->soh,\
                                                        discharge->status);
                } else {
                    DATE_yymmddhhmmss_t date;
                    date.date = discharge->time;
                    index = 0;
                    p[index++] = 0x00;
                    p[index++] = i03t_addr;
                
                    p[index++] = DataId_Hist_Data;
                
                    p[index++] = date.bits.year;
                    p[index++] = date.bits.month;
                    p[index++] = date.bits.day;
                    p[index++] = date.bits.hour;
                    p[index++] = date.bits.min;
                    p[index++] = date.bits.sec;
                
                    p[index++] = discharge->voltage >> 8;
                    p[index++] = discharge->voltage;
                
                    p[index++] = discharge->current[0] >> 8;
                    p[index++] = discharge->current[0];
                
                    p[index++] = discharge->temp >> 8;
                    p[index++] = discharge->temp;
                    
                    p[index++] = discharge->soc >> 8;
                    p[index++] = discharge->soc;
                    
                    p[index++] = discharge->soh >> 8;
                    p[index++] = discharge->soh;
                    
                    p[index++] = discharge->available_time >> 8;
                    p[index++] = discharge->available_time;
                    
                    p[index++] = discharge->status;
                    
                    uint8_t ser_flg = find ? 0 : SER_SOF;
                    
                    uint16_t send_length = de_protocol_slave_package_fill(proto_tx_buf,
                                            CTRL_Response,
                                            pDEApp->device_config.i03m.i03m_addr,
                                            query->dest,
                                            ser_flg |(query->requestMsgId & 0x3F),
                                            TAG_MASK_Logger,
                                            p,
                                            index);
                    
                    protocol_send_bytes(query->commType,proto_tx_buf,send_length); 
                    osDelay(10);
                }
                find = true;
#if FILL_FILE
                ff_cnt = 0;
#endif
            }  else {
#if FILL_FILE 
                if(discharge->crc_chk == 0xffffffff) {
                    ff_cnt++;
                    if(ff_cnt > 5) {
                        break;
                    }
                }
#endif
            }
        } else {
            break;
        }
    }
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp); 


    if(!query->print) {
        uint16_t send_length = de_protocol_slave_package_fill(proto_tx_buf,
                                                    CTRL_Response,
                                                    pDEApp->device_config.i03m.i03m_addr,
                                                    query->dest,
                                                    SER_EOF |(query->requestMsgId & 0x3F),
                                                    TAG_MASK_Logger,
                                                    NULL,
                                                    0);
                            
        protocol_send_bytes(query->commType,proto_tx_buf,send_length);
    }

    sys_free(discharge);
    sys_free(p);
    sys_free(proto_tx_buf);
    
    return errOK;
}




const char *discharge_head_str = "时间戳,总电压,总电流,温度,SOC,SOH,状态,可用时间,\r\n";


errStatus_t module_discharge_copy(void) {
    
    #define DISCHARGE_LOG_SIZE 256
   
    FILE *f_dest = NULL;
    FILE *f_src = NULL;
    
    char path_dest[64];
    char path_src[24];
    
    I03T_Info_t *i03t = NULL;
    Discharge_t *discharge = NULL;
    
    char *write_buf = sys_malloc(DISCHARGE_LOG_SIZE);
    discharge = sys_malloc(sizeof(Discharge_t));
    if(discharge == NULL) {
        goto EXIT;
    }
    memset(discharge,0,sizeof(Discharge_t));    
    
    if(write_buf == NULL) {
        goto EXIT;
    }

    for(uint8_t i03t_addr=1;i03t_addr<=CONFIG_MAX_I03T;i03t_addr++) {
        
        i03t = i03t_node_find(i03t_addr);
        
        uint32_t tick = osKernelGetTickCount();
#if FILL_FILE
        uint8_t ff_cnt = 0;
#endif
        uint16_t index = 0;
        
        if(i03t != NULL) {
            DATE_yymmddhhmmss_t time;
            memset(path_src,0,sizeof(path_src));
            sprintf(path_src,"dis%d.txt",i03t->i03t_addr);
            
            memset(path_dest,0,sizeof(path_dest));
            Bsp_RtcGetTime(&time);

            //sprintf(path_dest,DRVIER_LOGGER_PATH"dis%d_%d_%04d-%02d-%02d %02d.%02d.%02d.csv",i03t->i03t_addr,\
                                                            pDEApp->device_config.i03m.i03m_addr,\
                                                            time.bits.year + 2000,\
                                                            time.bits.month,\
                                                            time.bits.day,\
                                                            time.bits.hour,\
                                                            time.bits.min,\
                                                            time.bits.sec);
            
            sprintf(path_dest,DRVIER_LOGGER_PATH"%d_dis%d.csv",pDEApp->device_config.i03m.i03m_addr,i03t->i03t_addr);

            usbh_printf("open %s\r\n",path_dest);
            f_dest = fopen(path_dest,"w");
            if(f_dest == NULL) {
                goto EXIT;
            }
            usbh_printf("open %s\r\n",path_src);
            f_src = fopen(path_src,"r");
            if(f_src == NULL) {
                goto EXIT;
            }
            fseek(f_src,0,SEEK_SET); 
            
            //usbh_printf("write discharge_head_str");
            //write header.
            fwrite(discharge_head_str,strlen(discharge_head_str),1,f_dest);
            index = 0;
            memset(write_buf,0,DISCHARGE_LOG_SIZE);
#if FILE_CHECKERROR
            if(ferror(f_dest)) {
                clearerr(f_dest);
                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            }
#endif
            
            if(ferror(f_dest)) {
                clearerr(f_dest);
                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            }

			if(ferror(f_dest)) {
                clearerr(f_dest);
                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            }

            while(1) {
        
                feed_watchdog(); 
                
                memset(discharge,0,sizeof(Discharge_t));
                
                size_t size = fread(discharge,sizeof(Discharge_t),1,f_src);
                
                if(size == 1 ) {
                    uint32_t check = CRC_Get32(((uint8_t *)(discharge) + 4),sizeof(Discharge_t) - 4);
                    if(check == discharge->crc_chk ) {
                        index = 0;
                        memset(write_buf,0,DISCHARGE_LOG_SIZE);

                        time.date = discharge->time;
                        sprintf(write_buf + index,"%04d-%02d-%02d %02d:%02d:%02d,",time.bits.year + 2000,\
                                                                                time.bits.month,\
                                                                                time.bits.day,\
                                                                                time.bits.hour,\
                                                                                time.bits.min,\
                                                                                time.bits.sec);
                        index = strlen(write_buf);
                        
                        //volt,current,temp,soc,soh,status
                        sprintf(write_buf + index,"%d,%d,%d,%d,%d,%d,%d,\r\n",discharge->voltage,\
                                                                 discharge->current,\
                                                                 discharge->temp,\
                                                                 discharge->soc,\
                                                                 discharge->soh,\
                                                                 discharge->status,
                                                                 discharge->available_time);
                        index = strlen(write_buf);
                        fwrite(write_buf,index,1,f_dest);
#if FILE_CHECKERROR
                        if(ferror(f_dest)) {
                            clearerr(f_dest);
                            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                        }
#endif
#if FILL_FILE
                        ff_cnt = 0;
#endif
                        
                    } else {
#if FILL_FILE 
                        if(discharge->crc_chk == 0xffffffff) {
                            ff_cnt++;
                            if(ff_cnt > 3) {
                                break;
                            }
                        }
#endif
                    }
                } else {
                    
                    break;
                }
                
                int32_t tick_diff = (int32_t)(osKernelGetTickCount() - tick);
                
                if(abs(tick_diff) > FILE_DELAY) {
                    tick = osKernelGetTickCount();
                    osDelay(1);
                }
            }
            
            if(f_dest != NULL) {
                if(ferror(f_dest)) {
                    clearerr(f_dest);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
                fclose(f_dest);
                f_dest = NULL;
            }
            if(f_src != NULL) {
                if(ferror(f_src)) {
                    clearerr(f_src);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
                fclose(f_src);
                f_src = NULL;
            }
        }
    }

EXIT:
    if(f_dest != NULL) {
        fclose(f_dest);
    }
    if(f_src != NULL) {
        fclose(f_src);
    }
    
    if(write_buf != NULL) {
        sys_free(write_buf);
    }
    
    if(discharge != NULL) {
        sys_free(discharge);
    }
    
    return errOK;
}


errStatus_t module_charge_copy(void) {
    
    #define CHARGE_LOG_SIZE 256
   
    FILE *f_dest = NULL;
    FILE *f_src = NULL;
    
    char path_dest[64];
    char path_src[24];
    
    I03T_Info_t *i03t = NULL;
    Charge_t *charge = NULL;
    
    char *write_buf = sys_malloc(CHARGE_LOG_SIZE);
    charge = sys_malloc(sizeof(Charge_t));
    if(charge == NULL) {
        goto EXIT;
    }
    memset(charge,0,sizeof(Charge_t));    
    
    if(write_buf == NULL) {
        goto EXIT;
    }

    for(uint8_t i03t_addr=1;i03t_addr<=CONFIG_MAX_I03T;i03t_addr++) {
        
        i03t = i03t_node_find(i03t_addr);
        
        uint32_t tick = osKernelGetTickCount();
#if FILL_FILE
        uint8_t ff_cnt = 0;
#endif
        uint16_t index = 0;
        
        if(i03t != NULL) {
            DATE_yymmddhhmmss_t time;
            memset(path_src,0,sizeof(path_src));
            sprintf(path_src,"charge%d.txt",i03t->i03t_addr);
            
            memset(path_dest,0,sizeof(path_dest));
            Bsp_RtcGetTime(&time);

            sprintf(path_dest,DRVIER_LOGGER_PATH"%d_chg%d.csv",pDEApp->device_config.i03m.i03m_addr,i03t->i03t_addr);

            usbh_printf("open %s\r\n",path_dest);
            f_dest = fopen(path_dest,"w");
            if(f_dest == NULL) {
                goto EXIT;
            }
            usbh_printf("open %s\r\n",path_src);
            f_src = fopen(path_src,"r");
            if(f_src == NULL) {
                goto EXIT;
            }
            fseek(f_src,0,SEEK_SET); 
            
            //usbh_printf("write charge_head_str");
            //write header.
            fwrite(discharge_head_str,strlen(discharge_head_str),1,f_dest);
            index = 0;
            memset(write_buf,0,CHARGE_LOG_SIZE);
#if FILE_CHECKERROR
            if(ferror(f_dest)) {
                clearerr(f_dest);
                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            }
#endif
            
            if(ferror(f_dest)) {
                clearerr(f_dest);
                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            }

			if(ferror(f_dest)) {
                clearerr(f_dest);
                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            }

            while(1) {
        
                feed_watchdog(); 
                
                memset(charge,0,sizeof(Charge_t));
                
                size_t size = fread(charge,sizeof(Charge_t),1,f_src);
                
                if(size == 1 ) {
                    uint32_t check = CRC_Get32(((uint8_t *)(charge) + 4),sizeof(Charge_t) - 4);
                    if(check == charge->crc_chk ) {
                        index = 0;
                        memset(write_buf,0,DISCHARGE_LOG_SIZE);

                        time.date = charge->time;
                        sprintf(write_buf + index,"%04d-%02d-%02d %02d:%02d:%02d,",time.bits.year + 2000,\
                                                                                time.bits.month,\
                                                                                time.bits.day,\
                                                                                time.bits.hour,\
                                                                                time.bits.min,\
                                                                                time.bits.sec);
                        index = strlen(write_buf);
                        
                        //volt,current,temp,soc,soh,status
                        sprintf(write_buf + index,"%d,%d,%d,%d,%d,%d,,\r\n",charge->voltage,\
                                                                 charge->current,\
                                                                 charge->temp,\
                                                                 charge->soc,\
                                                                 charge->soh,\
                                                                 CHARGE_STATE);
                        index = strlen(write_buf);
                        fwrite(write_buf,index,1,f_dest);
#if FILE_CHECKERROR
                        if(ferror(f_dest)) {
                            clearerr(f_dest);
                            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                        }
#endif
#if FILL_FILE
                        ff_cnt = 0;
#endif
                        
                    } else {
#if FILL_FILE 
                        if(charge->crc_chk == 0xffffffff) {
                            ff_cnt++;
                            if(ff_cnt > 3) {
                                break;
                            }
                        }
#endif
                    }
                } else {
                    
                    break;
                }
                
                int32_t tick_diff = (int32_t)(osKernelGetTickCount() - tick);
                
                if(abs(tick_diff) > FILE_DELAY) {
                    tick = osKernelGetTickCount();
                    osDelay(1);
                }
            }
            
            if(f_dest != NULL) {
                if(ferror(f_dest)) {
                    clearerr(f_dest);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
                fclose(f_dest);
                f_dest = NULL;
            }
            if(f_src != NULL) {
                if(ferror(f_src)) {
                    clearerr(f_src);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
                fclose(f_src);
                f_src = NULL;
            }
        }
    }

EXIT:
    if(f_dest != NULL) {
        fclose(f_dest);
    }
    if(f_src != NULL) {
        fclose(f_src);
    }
    
    if(write_buf != NULL) {
        sys_free(write_buf);
    }
    
    if(charge != NULL) {
        sys_free(charge);
    }
    
    return errOK;
}



