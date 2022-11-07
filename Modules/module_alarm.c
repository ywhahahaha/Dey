#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "cmsis_os2.h"
#include "rl_fs.h"
#include "main.h"
#include "crc_check.h"
#include "i03t_list.h"
#include "module_alarm.h"
#include "de_protocol.h"
#include "thread_debug.h"

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
errStatus_t module_alarm_store(I03T_Info_t *i03t) {
    FILE *fp = NULL;
    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    AlarmStorage_t *alarm = sys_malloc(sizeof(AlarmStorage_t));
    if(alarm == NULL) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));

    uint16_t write_index = (i03t->alarm.index % FILE_ALARM_DATA_MAX_CNT);
    uint32_t write_addr = write_index * sizeof(AlarmStorage_t);
    
    sprintf(path,"alarm%d.txt",i03t->i03t_addr);
    fp = fopen(path,"r+");
    if(fp == NULL) {
        storage_msg_put(i03t->i03t_addr,NULL,0,StorageFileCheck);
        sys_free(alarm);
        return errErr;
    }
 
    int ret = fseek(fp,write_addr,SEEK_SET); 
    
    memcpy(alarm,&i03t->alarm,sizeof(AlarmStorage_t));

    Bsp_RtcGetTime((DATE_yymmddhhmmss_t *)&alarm->time);      
  
    alarm->crc_chk = CRC_Get32((uint8_t *)alarm + 4,sizeof(AlarmStorage_t) - 4);
    
    size_t size = fwrite(alarm,sizeof(AlarmStorage_t),1,fp);
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);
    
    sys_free(alarm);
    
    i03t->alarm.index++;

    return (size == 1) ? errOK : errErr;

}
void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
errStatus_t module_alarm_load(I03T_Info_t *i03t) {
    
    FILE *fp = NULL;

    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"alarm%d.txt",i03t->i03t_addr);
    
    AlarmStorage_t *alarm = sys_malloc(sizeof(AlarmStorage_t));
    if(alarm == NULL) {
        return errErr;
    }
    memset(alarm,0,sizeof(AlarmStorage_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(alarm);
        storage_msg_put(i03t->i03t_addr,NULL,0,StorageFileCheck);
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 
    
    uint32_t tick = osKernelGetTickCount();
    bool find = false;
#if FILL_FILE 
    uint8_t ff_cnt = 0;
#endif
    while(1) {
        feed_watchdog(); 
        memset(alarm,0,sizeof(AlarmStorage_t));
        size_t size = fread(alarm,sizeof(AlarmStorage_t),1,fp);
         
        if(size == 1 ) {
            uint32_t check = CRC_Get32(((uint8_t *)(alarm) + 4),sizeof(AlarmStorage_t) - 4);
            if(check == alarm->crc_chk ) {
                if(alarm->index > i03t->alarm.index) {
                    memcpy(&i03t->alarm,alarm,sizeof(AlarmStorage_t));
                }
                find = true;
#if FILL_FILE
                ff_cnt = 0;
#endif
            } else {
#if FILL_FILE 
                if(alarm->crc_chk == 0xffffffff) {
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
        
        int32_t tick_diff =  (int32_t)(osKernelGetTickCount() - tick);
        
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

    sys_free(alarm);
    
    if(find) {
        i03t->alarm.index++;
    }
    
    return errOK;

}



int32_t module_alarm_get_index(FILE *fp,uint32_t file_pos,AlarmStorage_t *cell) {
    
    memset(cell,0,sizeof(AlarmStorage_t));

	if(fseek(fp,file_pos,SEEK_SET)) {
		return -1;
	}
        
    size_t size = fread(cell,sizeof(AlarmStorage_t),1,fp);
    
    if(size == 1 ) {
        uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(AlarmStorage_t) - 4);
        if(check == cell->crc_chk ) {
            return cell->index;
        } 
    }
    
    return -1;
}


errStatus_t module_alarm_index_check(I03T_Info_t *i03t) {
    FILE *fp = NULL;

    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    if(i03t->alarm.index == 0) {
        return errErr;
    }

    uint16_t _index = ((i03t->alarm.index-1) % FILE_ALARM_DATA_MAX_CNT);
    int32_t file_pos = _index * sizeof(AlarmStorage_t);
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"alarm%d.txt",i03t->i03t_addr);
    
    AlarmStorage_t *cell = sys_malloc(sizeof(AlarmStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(AlarmStorage_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(cell);
        return errErr;
    }
    
    int32_t index = module_alarm_get_index(fp,file_pos,cell);
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);   

    sys_free(cell);
    
    if(index < 0) {
        return errErr;
    }

    return i03t->alarm.index >= index ? errOK : errErr;

}


errStatus_t module_alarm_load_protocol(uint8_t i03t_addr,LoggerQuery_t *query) {
    FILE *fp = NULL;
    
    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"alarm%d.txt",i03t_addr);
    
    AlarmStorage_t *alarm = sys_malloc(sizeof(AlarmStorage_t));
    if(alarm == NULL) {
        return errErr;
    }
    memset(alarm,0,sizeof(AlarmStorage_t));
    
    uint8_t *proto_tx_buf = sys_malloc(1024);
    if(proto_tx_buf == NULL) {
        sys_free(alarm);
        return errErr;
    }
    
    uint8_t *p = sys_malloc(1024);
    if(p == NULL) {
        sys_free(alarm);
        sys_free(proto_tx_buf);
        return errErr;
    }
    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(alarm);
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
        
        memset(alarm,0,sizeof(AlarmStorage_t));
        
        size_t size = fread(alarm,sizeof(AlarmStorage_t),1,fp);
        
        if(size == 1 ) {
            
            uint32_t check = CRC_Get32(((uint8_t *)(alarm) + 4),sizeof(AlarmStorage_t) - 4);
            if(check == alarm->crc_chk ) {
                if(query->print) {
                    DATE_yymmddhhmmss_t date;
                    date.date = alarm->time;
                    debug_printf("%04d-%02d-%02d %02d:%02d:%02d Volt:%05d,Res:%05d,Temp:%04d,SOC:%04d,SOH:%04d\r\n",\
                                                        date.bits.year + 2000,date.bits.month,date.bits.day,\
                                                        date.bits.hour,date.bits.min,date.bits.sec,\
                                                        alarm->total_info.discharge.voltage,\
                                                        alarm->total_info.discharge.current[0],\
                                                        alarm->total_info.discharge.temp,\
                                                        alarm->total_info.discharge.soc,\
                                                        alarm->total_info.discharge.soh);
                    
                    debug_printf("Alarm1:%04X,Alarm2:%04X,Alarm3:%04X\r\n",alarm->alarm.bat_group_alarm1,\
                                                                           alarm->alarm.bat_group_alarm2,\
                                                                           alarm->alarm.bat_group_alarm3);
                    for(uint16_t i = 0;i<CONFIG_MAX_CELL;i++) {
                        debug_printf("Cell<%d> Alarm:%04X,Res:%05d,Temp:%04d\r\n",i+1,alarm->alarm.cell_alarm[i]);
                    }
                } else { 
                    DATE_yymmddhhmmss_t date;
                    date.date = alarm->time;
                    index = 0;
                    p[index++] = 0x00;
                    p[index++] = i03t_addr;
                
                    p[index++] = DataId_Alarm_Data;
                
                    p[index++] = date.bits.year;
                    p[index++] = date.bits.month;
                    p[index++] = date.bits.day;
                    p[index++] = date.bits.hour;
                    p[index++] = date.bits.min;
                    p[index++] = date.bits.sec;
                
                    p[index++] = alarm->total_info.discharge.voltage >> 8;
                    p[index++] = alarm->total_info.discharge.voltage;
                
                    p[index++] = alarm->total_info.discharge.current[0] >> 8;
                    p[index++] = alarm->total_info.discharge.current[0];
                
                    p[index++] = alarm->total_info.discharge.temp >> 8;
                    p[index++] = alarm->total_info.discharge.temp;
                    
                    p[index++] = alarm->total_info.discharge.soc >> 8;
                    p[index++] = alarm->total_info.discharge.soc;
                    
                    p[index++] = alarm->total_info.discharge.soh >> 8;
                    p[index++] = alarm->total_info.discharge.soh;
                    
                    p[index++] = alarm->total_info.discharge.available_time >> 8;
                    p[index++] = alarm->total_info.discharge.available_time;
                    
                    p[index++] = alarm->total_info.discharge.status;
                    p[index++] = alarm->total_info.dry_node_status;
                    
                    p[index++] = alarm->alarm.bat_group_alarm1 >> 8;
                    p[index++] = alarm->alarm.bat_group_alarm1;

                    p[index++] = alarm->alarm.bat_group_alarm2 >> 8;
                    p[index++] = alarm->alarm.bat_group_alarm2;

                    p[index++] = alarm->alarm.bat_group_alarm3 >> 8;
                    p[index++] = alarm->alarm.bat_group_alarm3;

                        
                    for(uint16_t i=0;i<CONFIG_MAX_CELL;i++) {
                        p[index++] = alarm->alarm.cell_alarm[i] >> 8;
                        p[index++] = alarm->alarm.cell_alarm[i];
                    }
                    
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
            } else {
#if FILL_FILE 
                if(alarm->crc_chk == 0xffffffff) {
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
    
    sys_free(alarm);
    sys_free(p);
    sys_free(proto_tx_buf);
    
    return errOK;
}

const char *alarm_head_str = "时间戳,总电压,总电流,温度,SOC,SOH,状态,可用时间,";
const char *alarm_head_group_alarm_str = "组告警1,组告警2,组告警3,";


errStatus_t module_alarm_copy(void) {
    
    #define ALARM_LOG_SIZE 256
   
    FILE *f_dest = NULL;
    FILE *f_src = NULL;
    
    char path_dest[64];
    char path_src[24];
    
    usbh_printf("start\r\n");
    
    I03T_Info_t *i03t = NULL;
    AlarmStorage_t *alarm = NULL;
    
    char *write_buf = sys_malloc(ALARM_LOG_SIZE);
    alarm = sys_malloc(sizeof(AlarmStorage_t));
    if(alarm == NULL) {
        goto EXIT;
    }
    memset(alarm,0,sizeof(AlarmStorage_t));    
    
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
            sprintf(path_src,"alarm%d.txt",i03t->i03t_addr);
            
            memset(path_dest,0,sizeof(path_dest));
            Bsp_RtcGetTime(&time);

            //sprintf(path_dest,DRVIER_LOGGER_PATH"alarm%d_%d_%04d-%02d-%02d %02d.%02d.%02d.csv",i03t->i03t_addr,\
                                                            pDEApp->device_config.i03m.i03m_addr,\
                                                            time.bits.year + 2000,\
                                                            time.bits.month,\
                                                            time.bits.day,\
                                                            time.bits.hour,\
                                                            time.bits.min,\
                                                            time.bits.sec);
            sprintf(path_dest,DRVIER_LOGGER_PATH"%d_alm%d.csv",pDEApp->device_config.i03m.i03m_addr,i03t->i03t_addr);

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
            
            //usbh_printf("write %s\r\n",alarm_head_str);
            //write header.
            fwrite(alarm_head_str,strlen(alarm_head_str),1,f_dest);
#if FILE_CHECKERROR
			if(ferror(f_dest)) {
	            clearerr(f_dest);
	            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
	            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        	}
#endif
            
            fwrite(alarm_head_group_alarm_str,strlen(alarm_head_group_alarm_str),1,f_dest);
#if FILE_CHECKERROR
			if(ferror(f_dest)) {
	            clearerr(f_dest);
	            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
	            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        	}
#endif

            index = 0;
            memset(write_buf,0,ALARM_LOG_SIZE);
            
            
            for(uint16_t i=0;i<pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number;i++) {
                
                memset(write_buf,0,ALARM_LOG_SIZE);
                
                sprintf(write_buf,"单体%d告警,",i+1);
                
                fwrite(write_buf,strlen(write_buf),1,f_dest);
#if FILE_CHECKERROR
				if(ferror(f_dest)) {
	            	clearerr(f_dest);
	            	logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
	            	//    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
				}
#endif
            }
            
            memset(write_buf,0,ALARM_LOG_SIZE);            
            write_buf[0] = 0x0d;
            write_buf[1] = 0x0a;
            
            fwrite(write_buf,2,1,f_dest);
#if FILE_CHECKERROR
			if(ferror(f_dest)) {
	            clearerr(f_dest);
	            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
	            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        	}
#endif
            
            //usbh_printf("write alarm infor:%d\r\n",i03t_addr);
            
            while(1) {
        
                feed_watchdog(); 
                
                memset(alarm,0,sizeof(AlarmStorage_t));
                
                size_t size = fread(alarm,sizeof(AlarmStorage_t),1,f_src);

                if(size == 1 ) {
                    uint32_t check = CRC_Get32(((uint8_t *)(alarm) + 4),sizeof(AlarmStorage_t) - 4);
                    if(check == alarm->crc_chk ) {
                        index = 0;
                        memset(write_buf,0,ALARM_LOG_SIZE);

                        time.date = alarm->time;
                        sprintf(write_buf + index,"%04d-%02d-%02d %02d:%02d:%02d,",time.bits.year + 2000,\
                                                                                time.bits.month,\
                                                                                time.bits.day,\
                                                                                time.bits.hour,\
                                                                                time.bits.min,\
                                                                                time.bits.sec);
                        index = strlen(write_buf);
                        
                        //volt,current,temp,soc,soh,status
                        sprintf(write_buf + index,"%d,%d,%d,%d,%d,%d,%d,",alarm->total_info.discharge.voltage,\
                                                                 alarm->total_info.discharge.current[0],\
                                                                 alarm->total_info.discharge.temp,\
                                                                 alarm->total_info.discharge.soc,\
                                                                 alarm->total_info.discharge.soh,\
                                                                 alarm->total_info.discharge.status,
                                                                 alarm->total_info.discharge.available_time);
                        index = strlen(write_buf);
                        fwrite(write_buf,index,1,f_dest);
#if FILE_CHECKERROR
						if(ferror(f_dest)) {
				            clearerr(f_dest);
				            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
				            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
			        	}
#endif
                        AlarmGroup1_t alarm_group1 = {0};
                        alarm_group1.all = alarm->alarm.bat_group_alarm1;
                        memset(write_buf,0,ALARM_LOG_SIZE);
						index = 0;
						if(alarm_group1.all == 0) {
							sprintf(write_buf+index,"无告警,");
							index = strlen(write_buf);

						} else {
							if(alarm_group1.bit.over_volt) {
								sprintf(write_buf+index,"过压%d|",alarm_group1.bit.over_volt);
								index = strlen(write_buf);
							}
							if(alarm_group1.bit.low_volt) {
								sprintf(write_buf+index,"欠压%d|",alarm_group1.bit.low_volt);
								index = strlen(write_buf);
							}
							if(alarm_group1.bit.over_temp) {
								sprintf(write_buf+index,"过温%d|",alarm_group1.bit.over_temp);
								index = strlen(write_buf);
							}
							if(alarm_group1.bit.low_soc) {
                            	sprintf(write_buf+index,"SOC低%d|",alarm_group1.bit.low_soc);
								index = strlen(write_buf);
                        	}
							
                        	sprintf(write_buf+index,",");
							index = strlen(write_buf);
                        	
						}						
						fwrite(write_buf,index,1,f_dest);
#if FILE_CHECKERROR
						if(ferror(f_dest)) {
				            clearerr(f_dest);
				            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
				            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
			        	}
#endif
                        AlarmGroup2_t alarm_group2 = {0};
                        alarm_group2.all = alarm->alarm.bat_group_alarm2;
						memset(write_buf,0,ALARM_LOG_SIZE);
						index = 0;
						if(alarm_group2.all == 0) {
							sprintf(write_buf+index,"无告警,");
							index = strlen(write_buf);

						} else {
							if(alarm_group2.bit.total_volt_sample) {
								sprintf(write_buf+index,"总压|");
								index = strlen(write_buf);
							}
							if(alarm_group2.bit.current1_sample) {
								sprintf(write_buf+index,"电流1|");
								index = strlen(write_buf);
							}
							if(alarm_group2.bit.temp_sample) {
								sprintf(write_buf+index,"温度|");
								index = strlen(write_buf);
							}
							if(alarm_group2.bit.comm_err) {
								sprintf(write_buf+index,"通讯|");
								index = strlen(write_buf);
							}
							if(alarm_group2.bit.leak) {
								sprintf(write_buf+index,"漏液|");
								index = strlen(write_buf);
							}
							sprintf(write_buf+index,",");
							index = strlen(write_buf);

						}
						fwrite(write_buf,index,1,f_dest);
#if FILE_CHECKERROR
						if(ferror(f_dest)) {
				            clearerr(f_dest);
				            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
				            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
			        	}
#endif
                        
                        AlarmGroup3_t alarm_group3 = {0};
                        alarm_group3.all = alarm->alarm.bat_group_alarm3;
						memset(write_buf,0,ALARM_LOG_SIZE);
						index = 0;
						if(alarm_group3.all == 0) {
							sprintf(write_buf+index,"无告警,");
							index = strlen(write_buf);

						} else {
							if(alarm_group3.bit.current1_over_charge) {
								sprintf(write_buf+index,"组1过冲%d|",alarm_group3.bit.current1_over_charge);
								index = strlen(write_buf);
							}
							if(alarm_group3.bit.current1_over_discharge) {
								sprintf(write_buf+index,"组1过放%d|",alarm_group3.bit.current1_over_discharge);
								index = strlen(write_buf);
							}
							if(alarm_group3.bit.current2_over_charge) {
								sprintf(write_buf+index,"组2过冲%d|",alarm_group3.bit.current2_over_charge);
								index = strlen(write_buf);
							}
							if(alarm_group3.bit.current2_over_discharge) {
								sprintf(write_buf+index,"组2过放%d|",alarm_group3.bit.current2_over_discharge);
								index = strlen(write_buf);
							}
							if(alarm_group3.bit.current3_over_charge) {
								sprintf(write_buf+index,"组3过冲%d|",alarm_group3.bit.current3_over_charge);
								index = strlen(write_buf);
							}
							if(alarm_group3.bit.current3_over_discharge) {
								sprintf(write_buf+index,"组3过放%d|",alarm_group3.bit.current3_over_discharge);
								index = strlen(write_buf);
							}
							if(alarm_group3.bit.current4_over_charge) {
								sprintf(write_buf+index,"组4过冲%d|",alarm_group3.bit.current4_over_charge);
								index = strlen(write_buf);
							}
							if(alarm_group3.bit.current4_over_discharge) {
								sprintf(write_buf+index,"组4过放%d|",alarm_group3.bit.current4_over_discharge);
								index = strlen(write_buf);
							}
							sprintf(write_buf+index,",");
							index = strlen(write_buf);

						}
						fwrite(write_buf,index,1,f_dest);
#if FILE_CHECKERROR
						if(ferror(f_dest)) {
				            clearerr(f_dest);
				            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
				            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
			        	}
#endif
						 
                        for(uint16_t i=0;i<pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number;i++) {
                            memset(write_buf,0,ALARM_LOG_SIZE);
                            AlarmCell_t cell_alarm = {0};
							cell_alarm.all = alarm->alarm.cell_alarm[i];
							index = 0;
							if(cell_alarm.all == 0) {
								sprintf(write_buf+index,"无告警,");
								index = strlen(write_buf);

							} else {
								if(cell_alarm.bit.over_volt) {
									sprintf(write_buf+index,"过压%d|",cell_alarm.bit.over_volt);
									index = strlen(write_buf);
								}
								if(cell_alarm.bit.low_volt) {
									sprintf(write_buf+index,"欠压%d|",cell_alarm.bit.low_volt);
									index = strlen(write_buf);
								}
								if(cell_alarm.bit.over_temp) {
									sprintf(write_buf+index,"过温%d|",cell_alarm.bit.over_temp);
									index = strlen(write_buf);
								}
								if(cell_alarm.bit.over_res) {
									sprintf(write_buf+index,"内阻%d|",cell_alarm.bit.over_res);
									index = strlen(write_buf);
								}
								if(cell_alarm.bit.comm_err) {
									sprintf(write_buf+index,"通讯|");
									index = strlen(write_buf);
								}
								if(cell_alarm.bit.volt_sample) {
									sprintf(write_buf+index,"电压采集|");
									index = strlen(write_buf);
								}
								if(cell_alarm.bit.temp_sample) {
									sprintf(write_buf+index,"温度采集|");
									index = strlen(write_buf);
								}
								sprintf(write_buf+index,",");
								index = strlen(write_buf);
								
							}
                            fwrite(write_buf,strlen(write_buf),1,f_dest);
#if FILE_CHECKERROR
							if(ferror(f_dest)) {
					            clearerr(f_dest);
					            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
					            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
				        	}
#endif
                        }
                        
                       memset(write_buf,0,ALARM_LOG_SIZE);            
                       write_buf[0] = 0x0d;
                       write_buf[1] = 0x0a;
                       fwrite(write_buf,2,1,f_dest);
#if FILE_CHECKERROR
					   if(ferror(f_dest)) {
				            clearerr(f_dest);
				            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
				            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
			        	}
#endif
#if FILL_FILE
                       ff_cnt = 0;
#endif
                        
                    } else {
#if FILL_FILE 
                        if(alarm->crc_chk == 0xffffffff) {
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
                
                int32_t tick_diff =  (int32_t)(osKernelGetTickCount() - tick);
                
                if(abs(tick_diff) > FILE_DELAY) {
                    tick = osKernelGetTickCount();
                    osDelay(1);
                }
            }
            
            //usbh_printf("alarm finish write:%d\r\n",i03t_addr);
            if(f_dest != NULL) {
#if 0
                if(ferror(f_dest)) {
                    clearerr(f_dest);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
#endif
                fclose(f_dest);
                
                f_dest = NULL;
            }
            if(f_src != NULL) {
#if 0
                if(ferror(f_src)) {
                    clearerr(f_src);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
#endif
                fclose(f_src);
                
                f_src = NULL;
            }
        }
    }

EXIT:
    if(f_dest != NULL) {
#if 0
        if(ferror(f_dest)) {
            clearerr(f_dest);
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
#endif
        fclose(f_dest);
    }
    if(f_src != NULL) {
#if 0
        if(ferror(f_src)) {
            clearerr(f_src);
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
#endif
        fclose(f_src);
    }
    
    if(write_buf != NULL) {
        sys_free(write_buf);
    }
    
    if(alarm != NULL) {
        sys_free(alarm);
    }
    
    return errOK;
}


