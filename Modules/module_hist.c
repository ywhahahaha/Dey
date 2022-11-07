#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "cmsis_os2.h"
#include "rl_fs.h"
#include "main.h"
#include "module_hist.h"
#include "crc_check.h"
#include "i03t_list.h"
#include "de_protocol.h"

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
errStatus_t module_hist_store(I03T_Info_t *i03t) {
    FILE *fp = NULL;
    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));
    
    uint16_t write_index = (i03t->hist.index % FILE_HIST_DATA_MAX_CNT);
    uint32_t write_addr = write_index * sizeof(CellHistStorage_t);
    
    sprintf(path,"hist%d.txt",i03t->i03t_addr);
    fp = fopen(path,"r+");
    if(fp == NULL) {
        sys_free(cell);
        storage_msg_put(i03t->i03t_addr,NULL,0,StorageFileCheck);
        return errErr;
    }
    
    memcpy(cell,&i03t->hist,sizeof(CellHistStorage_t));
 
    fseek(fp,write_addr,SEEK_SET); 

    Bsp_RtcGetTime((DATE_yymmddhhmmss_t *)&cell->time);    
  
    cell->crc_chk = CRC_Get32((uint8_t *)(cell) + 4,sizeof(CellHistStorage_t) - 4);
    
    size_t size = fwrite(cell,sizeof(CellHistStorage_t),1,fp);
    
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
  
    fclose(fp);
    
    i03t->hist.index++;
    
    sys_free(cell);

    return (size == 1) ? errOK : errErr;

}
void debug_printf(const char *fmt, ...);
errStatus_t module_hist_load(I03T_Info_t *i03t) {
    FILE *fp = NULL;
    
    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"hist%d.txt",i03t->i03t_addr);
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(CellHistStorage_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(cell);
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 
    
    uint32_t tick = osKernelGetTickCount();
#if FILL_FILE 
    uint8_t ff_cnt = 0;
#endif
    uint8_t find = false;
    while(1) {
        
        feed_watchdog(); 
        
        memset(cell,0,sizeof(CellHistStorage_t));
        
        size_t size = fread(cell,sizeof(CellHistStorage_t),1,fp);
        
        if(size == 1 ) {
            uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(CellHistStorage_t) - 4);
            if(check == cell->crc_chk ) {
                if(cell->index > i03t->hist.index) {
                    memcpy(&i03t->hist,cell,sizeof(CellHistStorage_t));
                }
                find = true;
#if FILL_FILE
                ff_cnt = 0;
#endif
            } else {
#if FILL_FILE 
                if(cell->crc_chk == 0xffffffff) {
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

    sys_free(cell);
    
    if(find) {
        i03t->hist.index++;
    }
    
    return find ? errOK : errErr;

}



int32_t module_hist_get_index(FILE *fp,uint32_t file_pos,CellHistStorage_t *cell) {
    
    memset(cell,0,sizeof(CellHistStorage_t));

	if(fseek(fp,file_pos,SEEK_SET)) {
		return -1;
	}
        
    size_t size = fread(cell,sizeof(CellHistStorage_t),1,fp);
    
    if(size == 1 ) {
        uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(CellHistStorage_t) - 4);
        if(check == cell->crc_chk ) {
            return cell->index;
        } 
    }
    
    return -1;
}


errStatus_t module_hist_index_check(I03T_Info_t *i03t) {
    FILE *fp = NULL;

    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    if(i03t->hist.index == 0) {
        return errErr;
    }

    uint16_t _index = ((i03t->hist.index-1) % FILE_HIST_DATA_MAX_CNT);
    int32_t file_pos = _index * sizeof(CellHistStorage_t);
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"hist%d.txt",i03t->i03t_addr);
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(CellHistStorage_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(cell);
        return errErr;
    }
    
    int32_t index = module_hist_get_index(fp,file_pos,cell);
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

    return i03t->hist.index >= index ? errOK : errErr;

}

errStatus_t module_hist_load_protocol(uint8_t i03t_addr,LoggerQuery_t *query) {
    FILE *fp = NULL;
    
    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"hist%d.txt",i03t_addr);
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(CellHistStorage_t));
    
    uint8_t *proto_tx_buf = sys_malloc(PROTOCOL_RX_MAX_SIZE);
    if(proto_tx_buf == NULL) {
        sys_free(cell);
        return errErr;
    }
    
    uint8_t *p = sys_malloc(PROTOCOL_RX_MAX_SIZE);
    if(p == NULL) {
        sys_free(cell);
        sys_free(proto_tx_buf);
        return errErr;
    }
    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(cell);
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
        
        memset(cell,0,sizeof(CellHistStorage_t));
        
        size_t size = fread(cell,sizeof(CellHistStorage_t),1,fp);
        
        if(size == 1 ) {
            uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(CellHistStorage_t) - 4);
            if(check == cell->crc_chk ) {
                if(query->print){
                    DATE_yymmddhhmmss_t date;
                    date.date = cell->time;
                    debug_printf("%04d-%02d-%02d %02d:%02d:%02d Volt:%05d,Res:%05d,Temp:%04d,SOC:%04d,SOH:%04d\r\n",\
                                                        date.bits.year + 2000,date.bits.month,date.bits.day,\
                                                        date.bits.hour,date.bits.min,date.bits.sec,\
                                                        cell->total_info.discharge.voltage,\
                                                        cell->total_info.discharge.current[0],\
                                                        cell->total_info.discharge.temp,\
                                                        cell->total_info.discharge.soc,\
                                                        cell->total_info.discharge.soh);
                    for(uint16_t i = 0;i<CONFIG_MAX_CELL;i++) {
                        debug_printf("Cell<%d> Volt:%05d,Res:%05d,Temp:%04d\r\n",i+1,\
                                                        cell->cells[i].voltage,\
                                                        cell->cells[i].inter_res,\
                                                        cell->cells[i].temperature);
                    }
                }  else { 
                    DATE_yymmddhhmmss_t date;
                    date.date = cell->time;
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
                
                    p[index++] = cell->total_info.discharge.voltage >> 8;
                    p[index++] = cell->total_info.discharge.voltage;
                
                    p[index++] = cell->total_info.discharge.current[0] >> 8;
                    p[index++] = cell->total_info.discharge.current[0];
                
                    p[index++] = cell->total_info.discharge.temp >> 8;
                    p[index++] = cell->total_info.discharge.temp;
                    
                    p[index++] = cell->total_info.discharge.soc >> 8;
                    p[index++] = cell->total_info.discharge.soc;
                    
                    p[index++] = cell->total_info.discharge.soh >> 8;
                    p[index++] = cell->total_info.discharge.soh;
                    
                    p[index++] = cell->total_info.discharge.available_time >> 8;
                    p[index++] = cell->total_info.discharge.available_time;
                    
                    p[index++] = cell->total_info.discharge.status;
                    p[index++] = cell->total_info.dry_node_status;
                    
                    for(uint16_t i=0;i<CONFIG_MAX_CELL;i++) {
                        p[index++] = cell->cells[i].voltage >> 8;
                        p[index++] = cell->cells[i].voltage;
                        
                        p[index++] = cell->cells[i].inter_res >> 8;
                        p[index++] = cell->cells[i].inter_res;
                        
                        p[index++] = cell->cells[i].temperature >> 8;
                        p[index++] = cell->cells[i].temperature;
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
                
#if FILL_FILE
                ff_cnt = 0;
#endif
                find = true;
            } else {
#if FILL_FILE 
                if(cell->crc_chk == 0xffffffff) {
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

    sys_free(cell);
    sys_free(p);
    sys_free(proto_tx_buf);
    
    return errOK;
}


#if 0
errStatus_t module_hist_load_bs(I03T_Info_t *i03t) {
    FILE *fp = NULL;
    
    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"hist%d.txt",i03t->i03t_addr);
    
    fsFileInfo info;
    info.fileID = 0; 
    if(ffind (path, &info) != fsOK) {
        return errErr;
    }
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(CellHistStorage_t));
    
    
	int index1;
	int index2;
	int index3;

	int file_pos1 = 0;
	int file_pos2 = sizeof(CellHistStorage_t) / 2 * sizeof(CellHistStorage_t);
	int file_pos3 = (info.size / sizeof(CellHistStorage_t) - 1) * sizeof(CellHistStorage_t);

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 

	int temp_index;
	while(1) {
		temp_index = module_hist_find_index_big(fp, cell, file_pos1, file_pos2);
		if(temp_index > 0) {
			index1 = temp_index;
		} else {
			if(temp_index == -1) {
				index1 = index2;
			} 

			index1 = index1;
		}

		temp_index = module_hist_find_index_big(fp, cell, file_pos2, file_pos3);
		if(temp_index > 0) {
			index2 = temp_index;
		} else {
			if(temp_index == -1) {
				index2 = index3;
			} 
		}
		
	}

    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
   
    fclose(fp);   

    sys_free(cell);
    
    return errOK;

}
#endif


const char *hist_head_str = "时间戳,总电压,总电流,温度,SOC,SOH,状态,可用时间,";


errStatus_t module_hist_copy(void) {
    
    #define HIST_LOG_SIZE 256
   
    FILE *f_dest = NULL;
    FILE *f_src = NULL;
    
    char path_dest[64];
    char path_src[24];
    
    usbh_printf("start.\r\n");
    
    I03T_Info_t *i03t = NULL;
    CellHistStorage_t *cell = NULL;
    
    char *write_buf = sys_malloc(HIST_LOG_SIZE);
    cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        goto EXIT;
    }
    
    memset(cell,0,sizeof(CellHistStorage_t));    
    
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
        
        size_t ret = 0;
        
        if(i03t != NULL) {
            DATE_yymmddhhmmss_t time;
            memset(path_src,0,sizeof(path_src));
            sprintf(path_src,"hist%d.txt",i03t->i03t_addr);
            //strcpy(path_src,"hist1.txt");
            
            memset(path_dest,0,sizeof(path_dest));
            //Bsp_RtcGetTime(&time);
            
            //sprintf(path_dest,DRVIER_LOGGER_PATH"hist%d_%d_%04d-%02d-%02d %02d.%02d.%02d.csv",i03t->i03t_addr,\
                                                            pDEApp->device_config.i03m.i03m_addr,\
                                                            time.bits.year + 2000,\
                                                            time.bits.month,\
                                                            time.bits.day,\
                                                            time.bits.hour,\
                                                            time.bits.min,\
                                                            time.bits.sec);
            sprintf(path_dest,DRVIER_LOGGER_PATH"%d_his%d.csv",pDEApp->device_config.i03m.i03m_addr,i03t->i03t_addr);
            
            //sprintf(path_dest,DRVIER_LOGGER_PATH"hist_%d_%02d.csv",i03t->i03t_addr,\
                                                            time.bits.sec);
            
            //strcpy(path_dest,"U0:abc.csv");
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

            //write header.
            ret = fwrite(hist_head_str,strlen(hist_head_str),1,f_dest);
            //usbh_printf("write %s\r\n",hist_head_str);

            index = 0;
            memset(write_buf,0,HIST_LOG_SIZE);
#if FILE_CHECKERROR
            if(ferror(f_dest)) {
                clearerr(f_dest);
                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            }
#endif
            for(uint16_t i=0;i<pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number;i++) {
                memset(write_buf,0,HIST_LOG_SIZE);
                sprintf(write_buf,"单体%d电压,单体%d内阻,单体%d温度,",i+1,i+1,i+1);
                ret = fwrite(write_buf,strlen(write_buf),1,f_dest);
                
                
                if(ret != 1) {
                    
                }
#if FILE_CHECKERROR
                if(ferror(f_dest)) {
                    clearerr(f_dest);
                    logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                }
#endif
            } 
            
            memset(write_buf,0,HIST_LOG_SIZE);            
            write_buf[0] = 0x0d;
            write_buf[1] = 0x0a;
            ret = fwrite(write_buf,2,1,f_dest);

            while(1) {
        
                feed_watchdog(); 
                
                memset(cell,0,sizeof(CellHistStorage_t));
                
                size_t size = fread(cell,sizeof(CellHistStorage_t),1,f_src);

                if(size == 1 ) {
                    uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(CellHistStorage_t) - 4);
                    if(check == cell->crc_chk ) {
                        index = 0;
                        memset(write_buf,0,HIST_LOG_SIZE);

                        time.date = cell->time;
                        sprintf(write_buf + index,"%04d-%02d-%02d %02d:%02d:%02d,",time.bits.year + 2000,\
                                                                                time.bits.month,\
                                                                                time.bits.day,\
                                                                                time.bits.hour,\
                                                                                time.bits.min,\
                                                                                time.bits.sec);
                        index = strlen(write_buf);
                        
                        //volt,current,temp,soc,soh,status
                        sprintf(write_buf + index,"%d,%d,%d,%d,%d,%d,%d,",cell->total_info.discharge.voltage,\
                                                                 cell->total_info.discharge.current[0],\
                                                                 cell->total_info.discharge.temp,\
                                                                 cell->total_info.discharge.soc,\
                                                                 cell->total_info.discharge.soh,\
                                                                 cell->total_info.discharge.status,
                                                                 cell->total_info.discharge.available_time);
                        index = strlen(write_buf);
                        fwrite(write_buf,index,1,f_dest);
#if FILE_CHECKERROR
                        if(ferror(f_dest)) {
                            clearerr(f_dest);
                            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
                        }
#endif
                        for(uint16_t i=0;i<pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number;i++) {
                            memset(write_buf,0,HIST_LOG_SIZE);
                            sprintf(write_buf,"%d,%d,%d,",cell->cells[i].voltage,\
                                                                  cell->cells[i].inter_res,\
                                                                  cell->cells[i].temperature);
                            fwrite(write_buf,strlen(write_buf),1,f_dest);
                            
                        }
                        
                       memset(write_buf,0,HIST_LOG_SIZE);            
                       write_buf[0] = 0x0d;
                       write_buf[1] = 0x0a;
                       fwrite(write_buf,2,1,f_dest);
                        
#if FILL_FILE
                       ff_cnt = 0;
#endif
                        
                    } else {
#if FILL_FILE 
                        if(cell->crc_chk == 0xffffffff) {
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
    
    if(cell != NULL) {
        sys_free(cell);
    }
    
    return errOK;
}

