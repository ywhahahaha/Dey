#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "cmsis_os2.h"
#include "rl_fs.h"
#include "main.h"
#include "crc_check.h"
#include "i03t_list.h"
#include "module_file_storage.h"
#include "thread_debug.h"
#include "logger.h"

bool module_file_exist(char *path);
bool module_file_create(char *path,FileHead_t *head,uint16_t head_size,uint32_t file_size);
errStatus_t module_logger_check(void) {
   
    if(module_file_exist(FILE_LOGGER_PATH)) {
        return errOK;
    }

    module_file_create(FILE_LOGGER_PATH,NULL,0,sizeof(logger_t) * FILE_LOGGER_MAX_CNT);
    
    return errOK;
}


errStatus_t module_logger_store(logger_msg_t msg) {
    
    FILE *fp = NULL;

    logger_t log = {0};

    memset(&log,0,sizeof(logger_t));
    
    Bsp_RtcGetTime(&log.time);
    
    memcpy(&log.msg,&msg,sizeof(logger_msg_t));
    
	log.index = pDEApp->index.logger_index;
    
    log.sum = CRC_Get32((uint8_t *)&log.time,sizeof(logger_t) - 4); 
    
    fp = fopen(FILE_LOGGER_PATH,"r+");
    if(fp == NULL) {
        return errErr;
    }
    
    uint16_t write_index = (log.index % FILE_LOGGER_MAX_CNT);
    uint32_t write_addr = write_index * sizeof(logger_t);
    
    fseek(fp,write_addr,SEEK_SET);
    
    size_t size = fwrite(&log,sizeof(logger_t),1,fp);
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);
	
    pDEApp->index.logger_index++;
    
    return errOK;
}
void get_filename(const char *path, char *name);
void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
void logger_infor_save_file(LOGGER_TYPE_t type,LOGGER_MORE_INFOR_t more,uint32_t infor,const char *str,const char *file,uint16_t is_file_line) {
    
    logger_msg_t msg = {0};
     
    msg.type = type;
    msg.more = more;
    
    if(is_file_line) {
        get_filename(file,(char *)msg.file);
    } else {
        strcpy((char *)msg.file,file);
    }
    msg.line = is_file_line;
    
    if(str != NULL) {
        strcpy((char *)msg.v.d1,str);
    } else {
        msg.v.d4[0] = infor;
    }
    
    //module_logger_store(msg);

    storage_msg_put(0,(uint8_t *)&msg,sizeof(logger_msg_t),StorageSaveLogger);
}

const char *logger_head_str = "时间戳,类型,详细信息,说明,文件,行号\r\n";
errStatus_t module_logger_copy(void) {
    
    #define ALARM_LOG_SIZE 256
   
    FILE *f_dest = NULL;
    FILE *f_src = NULL;
    
    char path_dest[64];

    char *write_buf = sys_malloc(ALARM_LOG_SIZE);
    logger_t *logger = sys_malloc(sizeof(logger_t));
    if(logger == NULL) {
        goto EXIT;
    }
    memset(logger,0,sizeof(logger_t));    
    
    if(write_buf == NULL) {
        goto EXIT;
    }
    {
        
 
        uint32_t tick = osKernelGetTickCount();
#if FILL_FILE 
        uint8_t ff_cnt = 0;
#endif
        uint16_t index = 0;
        
        {
            DATE_yymmddhhmmss_t time;
    
            
            memset(path_dest,0,sizeof(path_dest));
            //Bsp_RtcGetTime(&time);

            sprintf(path_dest,DRVIER_LOGGER_PATH"%d_log.csv",pDEApp->device_config.i03m.i03m_addr);

            usbh_printf("open %s\r\n",path_dest);
            f_dest = fopen(path_dest,"w");
            if(f_dest == NULL) {
                goto EXIT;
            }
            usbh_printf("open %s\r\n",FILE_LOGGER_PATH);
            f_src = fopen(FILE_LOGGER_PATH,"r");
            if(f_src == NULL) {
                goto EXIT;
            }
            fseek(f_src,0,SEEK_SET); 
            
            //usbh_printf("write logger_head_str\r\n");
            //write header.
            fwrite(logger_head_str,strlen(logger_head_str),1,f_dest);
#if FILE_CHECKERROR
            if(ferror(f_dest)) {
                clearerr(f_dest);
                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            }
#endif
            //usbh_printf("start write logger.\r\n");

            while(1) {
        
                feed_watchdog(); 
                
                memset(logger,0,sizeof(logger_t));
                
                size_t size = fread(logger,sizeof(logger_t),1,f_src);
                
                
                
                if(size == 1 ) {
                    uint32_t check = CRC_Get32(((uint8_t *)(logger) + 4),sizeof(logger_t) - 4);
                    if(check == logger->sum ) {
                        index = 0;
                        memset(write_buf,0,ALARM_LOG_SIZE);
						//时间戳
                        time.date = logger->time.date;
                        sprintf(write_buf + index,"%04d-%02d-%02d %02d:%02d:%02d,",time.bits.year + 2000,\
                                                                                time.bits.month,\
                                                                                time.bits.day,\
                                                                                time.bits.hour,\
                                                                                time.bits.min,\
                                                                                time.bits.sec);
                        index = strlen(write_buf);
						
						
						
                        if(logger->msg.type >= LOGGER_TYPE_MAX) {
                            continue;
                        }
						//类型
                        sprintf(write_buf + index,"%s,",logger_type[logger->msg.type]);
						index = strlen(write_buf);

						
                        switch(logger->msg.type) {
                            case LOGGER_RESET: {
								char buf[64] = {0};
								logger_get_reboot(logger->msg.v.d4[0],buf);
								sprintf(write_buf + index,"复位,%s,",buf);
								index = strlen(write_buf);
								break;
							}
								
                            
                            case LOGGER_CONFIG:
								sprintf(write_buf + index,"参数配置,,");
								index = strlen(write_buf);
                                break;
                            
                            case LOGGER_OPERATE:
                                switch(logger->msg.more) {
                                    case LOGGER_MORE_FS_FORMAT:
                                        sprintf(write_buf + index,"文件系统格式化,%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_FILE_CREATE:
                                        sprintf(write_buf + index,"创建文件,%s,",logger->msg.v.d1);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_FILE_WRITE_OPEN_FAILED:
                                        sprintf(write_buf + index,"文件写失败,%s,",logger->msg.v.d1);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_FILE_OPERATE_FAILED:
                                        sprintf(write_buf + index,"文件操作失败,%s,",logger->msg.v.d1);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_SYNCH_PARA_I03T:
                                        sprintf(write_buf + index,"配置I03T,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_SYNCH_SN_I03T:
										sprintf(write_buf + index,"同步SN,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_ADD_I03T:
                                        sprintf(write_buf + index,"添加I03T,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_DEL_I03T:
										sprintf(write_buf + index,"删除I03T,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
                                        break;
									case LOGGER_MORE_ADD_SN:
										sprintf(write_buf + index,"添加SN,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
										break;
    								case LOGGER_MORE_DEL_ALL_SN:
										sprintf(write_buf + index,"删除所有SN,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
										break;
                                    case LOGGER_MORE_DEL_HIST:
										sprintf(write_buf + index,"删除历史数据,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_DEL_ALARM:
										sprintf(write_buf + index,"删除告警记录,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_DEL_DISCHARGE:
										sprintf(write_buf + index,"删除放电数据,I03T%d,",logger->msg.v.d4[0]);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_UPGRADE:
										sprintf(write_buf + index,"升级I03M,,");
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_SOC_INIT:
                                        sprintf(write_buf + index,"初始化SOC,%s,",logger->msg.v.d1);
										index = strlen(write_buf);
                                        break;
                                    case LOGGER_MORE_MQTT:
                                        sprintf(write_buf + index,"网络连接状态,%s,",logger->msg.v.d4[0]?"连接":"断开");
                                        index = strlen(write_buf);    
                                        break;
                                    
                                    case LOGGER_MORE_CONFIGTIME:
                                        {
                                            DATE_yymmddhhmmss_t date = {0};
                                            date.date = logger->msg.v.d4[0];
                                            sprintf(write_buf + index,"校时%04d-%02d-%02d %02d:%02d:%02d,",date.bits.year + 2000,\
                                                                                                            date.bits.month,\
                                                                                                            date.bits.day,\
                                                                                                            date.bits.hour,\
                                                                                                            date.bits.min,\
                                                                                                            date.bits.sec);
                                            index = strlen(write_buf);  
                                        }
                                        
                                        break;
                                    default:
                                        break;
                                }
                                break;
                                
                            case LOGGER_EXCEPT:
                                break;
                            
                            case LOGGER_OTHERS:
                                break;
                            
                            case LOGGER_WARNING:
                                sprintf(write_buf + index,"%s,,",logger->msg.v.d1);
								index = strlen(write_buf);
                                break;
                        }
                        
                       
                       //文件,行号
						sprintf(write_buf + index,"%s,%d\r\n",logger->msg.file,logger->msg.line);
                        
						fwrite(write_buf,strlen(write_buf),1,f_dest);
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
                        if(logger->sum == 0xffffffff) {
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
            
            //usbh_printf("end write logger.\r\n");
            
            if(f_dest != NULL) {
                if(ferror(f_dest)) {
                    clearerr(f_dest);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
                fclose(f_dest);
                f_dest = NULL;
            }
            if(f_src != NULL) {
                if(ferror(f_src)) {
                    clearerr(f_src);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
                fclose(f_src);
                f_src = NULL;
            }
        }
    }

EXIT:
    if(f_dest != NULL) {
        if(ferror(f_dest)) {
            clearerr(f_dest);
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
        fclose(f_dest);
    }
    if(f_src != NULL) {
        if(ferror(f_src)) {
            clearerr(f_src);
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
        fclose(f_src);
    }
    
    if(write_buf != NULL) {
        sys_free(write_buf);
    }
    
    if(logger != NULL) {
        sys_free(logger);
    }
    
    return errOK;
}


