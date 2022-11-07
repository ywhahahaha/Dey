#include "main.h"
#include "rl_fs.h"
#include "iap.h"
#include "storage.h"
#include "module_sn.h"
#include "logger.h"
#include "de_protocol.h"


//#define usbh_printf(...)   {debug_printf(__VA_ARGS__);}

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length);
errStatus_t module_hist_copy(void);
errStatus_t module_alarm_copy(void);
errStatus_t module_discharge_copy(void);
errStatus_t appl_usbh_copy_sn(void);
errStatus_t module_logger_copy(void);
errStatus_t appl_usbh_i03m_token_set(void);
errStatus_t appl_usbh_sn_cloud_id_set(void);
errStatus_t module_charge_copy(void);
void module_4g_msg_put(ACTION_TYPE_t action,uint8_t *pdata,uint16_t length);

char *sn_header = "控制器编号,设备编号,设备ID,设备SN码,控制器TOKEN\r\n";

void appl_usbh_copy_logger(uint8_t file) {
    //fsStatus status = fmkdir(DRVIER_LOGGER_PATH);
    //if(status == fsOK || status == fsAlreadyExists) 
    {
        switch(file) {
            case 1:
                module_hist_copy();
                break;
            case 2:
                module_alarm_copy();
                break;
            case 3:
                module_discharge_copy();
                module_charge_copy();
                break;
            case 4:
                appl_usbh_copy_sn();
                break;
            default:

                appl_usbh_i03m_token_set();
                appl_usbh_sn_cloud_id_set();
            
                module_hist_copy();
                module_alarm_copy();
                module_discharge_copy();
                module_charge_copy();
                appl_usbh_copy_sn();
                module_logger_copy();
                break;
        }	
    }
}

errStatus_t appl_usbh_copy_sn(void) {
	errStatus_t result = errOK;
    FILE *f_dest = NULL;
    char temp[256] = {0};
    //DATE_yymmddhhmmss_t time;
    memset(temp,0,sizeof(temp));
    //Bsp_RtcGetTime(&time);
    
    usbh_printf("start.\r\n");
    
    sprintf(temp,DRVIER_LOGGER_PATH"%d_SN.csv",pDEApp->device_config.i03m.i03m_addr);
    
    f_dest = fopen(temp,"w");
    if(f_dest == NULL) {
        return errErr;
    }
    usbh_printf("open:%s OK\r\n",temp);
    
	fwrite(sn_header,strlen(sn_header),1,f_dest);
    
    //usbh_printf("write:%s\r\n",sn_header);
    
#if FILE_CHECKERROR
	if(ferror(f_dest)) {
	   clearerr(f_dest);
	   logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
	   //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
#endif

    uint32_t addr = ADDR_SN_CONFIG;
    uint16_t sn_index = 0;
    uint16_t sn_max = CONFIG_MAX_I03T * CONFIG_MAX_CELL;
    SNStore_t sn_info = {0};
    char cloud_id[CONFIG_CLOUD_ID_LENGTH + 1] = {0};
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_I03T; i03t_addr++) {
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node == NULL) { 
            continue;
        }
        if(i03t_node->mount) {
            continue;
        }
            
        memset(temp,0,sizeof(temp));
        memset(cloud_id,0,sizeof(cloud_id));
        char sn[CONFIG_SN_LENGTH + 1] = {0};
        memcpy(sn,sn_info.sn,CONFIG_SN_LENGTH);
        memcpy(cloud_id,pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,CONFIG_CLOUD_ID_LENGTH);
        sprintf(temp,"I03M%d,I03M%d-I03T%d-%d-000,%s,,%s\r\n",pDEApp->device_config.i03m.i03m_addr,\
                                                pDEApp->device_config.i03m.i03m_addr,\
                                                i03t_addr,\
                                                i03t_addr,\
                                                cloud_id,\
                                                pDEApp->device_config.i03m.mqtt_config.user_name);
        
   
        size_t size = fwrite(temp,strlen(temp),1,f_dest);
        if(size != 1) {
            result = errErr;
            break;
        }
    }
    
	//usbh_printf(" write sn start.\r\n");
    for(sn_index = 0;sn_index < sn_max;sn_index++,addr += sizeof(SNStore_t)) {
        memset(&sn_info,0,sizeof(SNStore_t));
        storage_readbytes(addr,(uint8_t *)&sn_info,sizeof(SNStore_t));
        uint32_t crc = CRC_Get32(((uint8_t *)(&sn_info)) + 4,sizeof(SNStore_t) - 4);
        
        if(crc == sn_info.crc_check) {
 			memset(temp,0,sizeof(temp));
            memset(cloud_id,0,sizeof(cloud_id));
            char sn[CONFIG_SN_LENGTH + 1] = {0};
            memcpy(sn,sn_info.sn,CONFIG_SN_LENGTH);
            memcpy(cloud_id,sn_info.cloud_id,CONFIG_CLOUD_ID_LENGTH);
            if(sn_info.i03t_addr > CONFIG_MAX_I03T || sn_info.i03t_addr == 0) {continue;}
			sprintf(temp,"I03M%d,I03M%d-I03T%d-%d-%03d,%s,%s,%s\r\n",\
                    pDEApp->device_config.i03m.i03m_addr,\
                    pDEApp->device_config.i03m.i03m_addr,\
                    pDEApp->device_config.i03t_nodes[sn_info.i03t_addr-1].base_para.mount?pDEApp->device_config.i03t_nodes[sn_info.i03t_addr-1].base_para.mount:sn_info.i03t_addr,\
                    sn_info.i03t_addr,\
                    sn_info.cell_id,\
                    cloud_id,\
                    sn,\
                    pDEApp->device_config.i03m.mqtt_config.user_name);
            
			size_t size = fwrite(temp,strlen(temp),1,f_dest);
            
			if(size != 1) {
				result = errErr;
				break;
			}
#if FILE_CHECKERROR
			if(ferror(f_dest)) {
	            clearerr(f_dest);
	            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
	            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
				break;
			}
#endif
        }
    }
    
    //usbh_printf("write sn end.\r\n");

	if(f_dest != NULL) {
        if(ferror(f_dest)) {
            clearerr(f_dest);
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
		fclose(f_dest);
	}

	return result;

}

/*
控制器编号,设备token
I03M1,rpHvHJjBOJWcAG6hoFKv
*/
errStatus_t appl_usbh_i03m_token_set(void) {

    errStatus_t result = errOK;
    FILE *fp = NULL;
    char line[256] = {0};
    fp = fopen(DRVIER_I03M_TOKEN_PATH,"r");
    if(fp == NULL) {
        return errErr;
    }

    fseek(fp,0,SEEK_SET); 
	uint16_t line_cnt = 0;

    char token[64] = {0};

    int i03m_addr = 0;
    while (fgets (line, sizeof (line), fp) != NULL)  {
        
        feed_watchdog();
        
        if(line_cnt++ == 0) {
            memset(line,0,sizeof(line));
			continue;
		}


        int ret  = 0;
        memset(token,0,sizeof(token));
        ret = sscanf(line, "I03M%d,%[^,\r\n]", &i03m_addr,token);
        
        if(ret == 2 && i03m_addr == pDEApp->device_config.i03m.i03m_addr) {
            if(!memcmp(pDEApp->device_config.i03m.mqtt_config.user_name,token,strlen(token))) {
                break;
            }
            memset(pDEApp->device_config.i03m.mqtt_config.user_name,0,sizeof(pDEApp->device_config.i03m.mqtt_config.user_name));
            memcpy((char *)pDEApp->device_config.i03m.mqtt_config.user_name,token,64);
            appl_para_save();
            module_4g_msg_put(ACTION_TYPE_REGISTER,NULL,0);
            break;
        }

		memset(line,0,sizeof(line));
        
    }
#if 0
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
#endif
    
    fclose(fp);

	return result;

}

void FileIndexClear(uint8_t i03t_addr);
void soc_msg_put(uint8_t i03t_addr,uint8_t command);
void App_soc_terminate(unsigned char bat_pack_label);
void appl_usbh_clear_i03t(void) {
    
    for(uint8_t i03t_addr=1;i03t_addr<=CONFIG_MAX_I03T;i03t_addr++) {
        pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.i03t_addr = 0; 
        pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.mount = 0;
        pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number = 0;
        memset(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,0,sizeof(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id));
        i03t_node_remove_addr(i03t_addr);
        module_i03t_file_flush(i03t_addr);
        FileIndexClear(i03t_addr);
        App_soc_terminate(i03t_addr);
    }
    module_sn_clear(0);
    i03t_node_update();
    module_sn_get_count(0, false);
    
}


/*
控制器编号,设备编号,设备ID,设备SN码
I03M1,I03M1-I03T1-0-000-1,e76e9c39dff64204baf87e3f374536f7,
I03M1,I03M1-I03T1-1-001-1,715ba792a5c545e8810e807b93fb818e,CELL-2022-02-01-001
I03M1,I03M1-I03T1-2-001-1,608b4698e3b64d90a137578d2f8fd993,CELL-2022-02-02-001

XXX...I03M1,XXX...I03M1-I03T1-2-001-1,608b4698e3b64d90a137578d2f8fd993,CELL-2022-02-02-001
*/

errStatus_t appl_usbh_sn_cloud_id_set(void) {
	errStatus_t result = errOK;
    FILE *fp = NULL;
    char line[256] = {0};
    fp = fopen(DRVIER_I03M_DEV_INFO_PATH,"r");
    if(fp == NULL) {
        return errErr;
    }

    bool clear_i03t = false;
    fseek(fp,0,SEEK_SET); 

	uint16_t line_cnt = 0;
    char i03m_key[8] = {0};
    sprintf(i03m_key,"I03M%d,",pDEApp->device_config.i03m.i03m_addr);

    
    while (fgets (line, sizeof (line), fp) != NULL)  {
        feed_watchdog();
        
        if(line_cnt++ == 0) {
            memset(line,0,sizeof(line));
			continue;
		}

        /* I03M-X */
		char *p = strstr(line,i03m_key);
		if(p == NULL) {
            memset(line,0,sizeof(line));
			continue;
        }

		p += strlen(i03m_key);

        /* I03T-X OR X-XXX*/
        int i03m_addr = 0; 
        int mount_addr = 0;
        int i03t_addr = 0;
        int cell_id = 0;
        int current_group = 0;
        int ret = 0;
        
        char cloud_id[CONFIG_CLOUD_ID_LENGTH+2] = {0};
        char sn_str[CONFIG_SN_LENGTH+2] = {0};
        
        memset(cloud_id,0,sizeof(cloud_id));
        memset(sn_str,0,sizeof(sn_str));
        
        if(!clear_i03t) {
            clear_i03t = true;
            appl_usbh_clear_i03t();
        }
        
        //XXX...I03M1,XXX...I03M1-I03T.......
        p = strstr(p,"I03M");
        if(p == NULL) {
            continue;
        }

        ret = sscanf(p, "I03M%d-I03T%d-%d-%d-%d,%[^,],%[^,\r\n]", &i03m_addr,&mount_addr,&i03t_addr,&cell_id,&current_group,cloud_id,sn_str);

        if(i03m_addr != pDEApp->device_config.i03m.i03m_addr) {
            continue;
        }
        //I03M1-I03T1-1-000-1,e76e9c39dff64204baf87e3f374536f7,
        if(ret == 6) { //bat group.
            if(i03t_addr == mount_addr && i03t_addr <= CONFIG_MAX_I03T && i03t_addr > 0 && cell_id == 0) {
                pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.i03t_addr = i03t_addr;
                pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.mount = 0;
                pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.current_group_number = current_group;
                i03t_node_add_addr(i03t_addr,0);
                
                memset(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,0,sizeof(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id));

                memcpy((char *)pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,cloud_id,CONFIG_CLOUD_ID_LENGTH);
                
                debug_printf("Add [I03T%d],cloud:[%s],mount@[I03M%d]\r\n",i03t_addr,cloud_id,i03m_addr);
            }
        } 
        //cells
        //I03M1-I03T1-1-001-1,715ba792a5c545e8810e807b93fb818e,CELL-2022-02-01-001
        //I03M1-I03T1-2-001-1,715ba792a5c545e8810e807b93fb818e,CELL-2022-02-01-001
        else if(ret == 7) {
            if(i03t_addr <= CONFIG_MAX_I03T && i03t_addr > 0 &&\
               mount_addr <= CONFIG_MAX_I03T && mount_addr > 0 &&\
               cell_id != 0) {
                I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);                   
                if(i03t_node == NULL) {
                    pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.i03t_addr = i03t_addr;
                    pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.mount = mount_addr;
                    i03t_node_add_addr(i03t_addr,mount_addr);
                    I03T_Info_t *i03t = (I03T_Info_t *)i03t_node_find(i03t_addr);
                    if(i03t != NULL) {
                        i03t->mount = mount_addr;
                        i03t->discharge.soc = 1000;
                        i03t->discharge.soh = 1000;
                        i03t->discharge.index = 0;
                        i03t->charge.index = 0;
                        i03t->alarm.index = 0;
                        i03t->hist.index = 0;
                        i03t->flag.all = 0;
                    }
                    
                    debug_printf("Add [I03T%d],mount@[I03T%d]\r\n",i03t_addr,mount_addr);
                    
                    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_ADD_I03T,i03t_addr,NULL,__FILE__,__LINE__);
                }
                SNStore_t sn = {0};
                memset(&sn,0,sizeof(SNStore_t));
                sn.i03t_addr = i03t_addr;
                sn.cell_id = cell_id;
                sn.cell_on_current_group = current_group;
                memcpy(sn.sn,sn_str,CONFIG_SN_LENGTH);
                memcpy(sn.cloud_id,cloud_id,CONFIG_CLOUD_ID_LENGTH);
                module_sn_add(&sn);  
                
                i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
                i03t_node->flag.bits.synch_sn_force = 1;
                i03t_node->flag.bits.synch_config = 0;

                debug_printf("Add cell[%03d],group:[%d],cloud:[%s],mount@[I03T%d]\r\n",cell_id,current_group,cloud_id,i03t_addr);
            }
        } else if(ret == 5) {
            //I03M1-I03T1-0-000-1,,
            if(i03t_addr == mount_addr && i03t_addr <= CONFIG_MAX_I03T && i03t_addr > 0 && cell_id == 0) {
                pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.i03t_addr = i03t_addr;
                pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.mount = 0;
                pDEApp->device_config.i03t_nodes[i03t_addr-1].cap.current_group_number = current_group;
                i03t_node_add_addr(i03t_addr,0);
                memset(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,0,sizeof(pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id));
 
                memcpy((char *)pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,cloud_id,CONFIG_CLOUD_ID_LENGTH);
                
                debug_printf("Add [I03T%d],group:[%d],cloud:[%s],mount@[I03M%d]\r\n",i03t_addr,current_group,cloud_id,i03m_addr);
            } else {
                //I03M1-I03T1-1-001-1,,CELL-2022-02-01-001
                memset(cloud_id,0,sizeof(cloud_id));
                memset(sn_str,0,sizeof(sn_str));
                ret = sscanf(p, "I03M%d-I03T%d-%d-%d-%d,,%[^,\r\n]", &i03m_addr,&mount_addr,&i03t_addr,&cell_id,&current_group,sn_str);

                if(ret == 6 && i03t_addr <= CONFIG_MAX_I03T && i03t_addr > 0 &&\
                   mount_addr <= CONFIG_MAX_I03T && mount_addr > 0 &&\
                   cell_id != 0) {
                       
                    I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
                    if(i03t_node == NULL) {
                        pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.i03t_addr = i03t_addr;
                        pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.mount = mount_addr;
                        i03t_node_add_addr(i03t_addr,mount_addr);
                        I03T_Info_t *i03t = (I03T_Info_t *)i03t_node_find(i03t_addr);
                        if(i03t != NULL) {
                            i03t->mount = mount_addr;
                            i03t->discharge.soc = 1000;
                            i03t->discharge.soh = 1000;
                            i03t->discharge.index = 0;
                            i03t->charge.index = 0;
                            i03t->alarm.index = 0;
                            i03t->hist.index = 0;
                            i03t->flag.all = 0;
                        }
                        
                        debug_printf("Add [I03T%d],mount@[I03T%d]\r\n",i03t_addr,mount_addr);
                        
                        logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_ADD_I03T,i03t_addr,NULL,__FILE__,__LINE__);
                    }
                    SNStore_t sn = {0};
                    memset(&sn,0,sizeof(SNStore_t));
                    sn.i03t_addr = i03t_addr;
                    sn.cell_id = cell_id;
                    sn.cell_on_current_group = current_group;
                    memcpy(sn.sn,sn_str,CONFIG_SN_LENGTH);
                    
                    module_sn_add(&sn);  
                    
                    i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
                    i03t_node->flag.bits.synch_sn_force = 1;
                    i03t_node->flag.bits.synch_config = 0;
                    
                    debug_printf("Add cell[%03d],group:[%d],cloud:[%s],mount@[I03T%d]\r\n",cell_id,current_group,cloud_id,i03t_addr);
                }
            
            }
        } else {
            break;
        }
        
		memset(line,0,sizeof(line));
        
    }

    debug_printf("finish config on udisk.\r\n");
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);

    i03t_node_update();
 
    module_sn_get_count(0, false);

    appl_para_save();
   
	return result;
}

errStatus_t appl_usbh_check_access(void) {
    fsFileInfo info;
    fsStatus status;
    
    memset(&info,0,sizeof(info));

    status =  ffind (DRVIER_ACCESS_PATH, &info);
    if(status != fsOK) {
        return errErr;
    }
    
    debug_printf("Udisk Access OK\r\n");
    
    return errOK;
}
errStatus_t appl_comm_i03t_request_upgrade(COMM_TYPE_t commType,
                        uint8_t i03t_addr,
                        uint8_t dataId,
                        uint8_t *pdata,
                        uint16_t length,
                        uint16_t pack_no,
                        uint16_t timeout);
extern osThreadId_t tid_thread_comm_i03t; 
errStatus_t appl_usbh_upgrade_i03t(void) {
    errStatus_t result = errErr;
    fsFileInfo info;
    fsStatus status;
    
    int x = 0;
    int y = 0;
    int z = 0;
    int ret;
    
    uint8_t i03t_cnt = 0;
    
    bool upgrade_flag = false;
    
    uint32_t file_sum = 0;
    uint32_t file_size = 0;
    uint16_t pack_size = 0;
    
    uint32_t temp_sum = 0;
    uint32_t temp_size = 0;
    
    memset(&info,0,sizeof(info));
    
    char path[64] = {0};
    strcpy(path,DRVIER_UPGRADE_PATH"I03T*.bin");
    status =  ffind (path, &info);
    if(status != fsOK) {
        return errErr;
    }
    
    if(info.size > APP_SIZE) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));
    
    sprintf(path,DRVIER_UPGRADE_PATH"%s",info.name);
    
    debug_printf("File path:%s\r\n",path);
    
    FILE *fp = NULL;
    fp = fopen(path,"r");
    if(fp == NULL) {
        debug_printf("File path:%s open failed.\r\n",path);
        return errErr;
    }
    
    fseek(fp,0,SEEK_SET); 

    uint8_t *buf = sys_malloc(1024);
    
    memset(buf,0,1024);
    size_t size = fread(buf,1,1024,fp);
    if(size != 1024) {
        goto EXIT;
    }
    

    if(buf[0] != 0x55 || buf[1] != 0xAA) {
        goto EXIT;
    }

    ret = sscanf((char *)buf + 2,"%d.%d.%d#",&x,&y,&z);
                    
    if(ret != 3) {
        goto EXIT;
    }

    
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL) {
            if(x != VERSION_X(i03t_node->soft_version) ||
               y != VERSION_Y(i03t_node->soft_version) ||
               z != VERSION_Z(i03t_node->soft_version)) {
                upgrade_flag = true;
                break;
            }
               
            i03t_cnt++;
        }
    }
    
    if(!upgrade_flag && i03t_cnt) {
        goto EXIT;
    }

    //file size
    file_size =  (uint32_t)buf[24] << 24 \
                |(uint32_t)buf[25] << 16 \
                |(uint32_t)buf[26] << 8 \
                |(uint32_t)buf[27];
    
    file_sum =   (uint32_t)buf[28] << 24 \
                |(uint32_t)buf[29] << 16 \
                |(uint32_t)buf[30] << 8 \
                |(uint32_t)buf[31];
    
    pack_size = (uint16_t)buf[32] << 8 \
                |buf[33];
    
    if(pack_size != 1024) {
        goto EXIT;
    }
    
    

    while(1) {
        
        feed_watchdog(); 
        
        memset(buf,0,1024);
        
        size = fread(buf,1,1024,fp);
        
        for(int i=0;i<size;i++) {
            temp_sum += buf[i];
        }
        temp_size += size;
        
        if(size != 1024) {
            break;
        }
    }
    
    if(temp_size == file_size && temp_sum == file_sum) {
        
        osThreadSuspend(tid_thread_comm_i03t);
        
        for(uint8_t trytimes = 0;trytimes < 3;trytimes++) {
            
            uint16_t pack_no = 0;
            
            fseek(fp,0,SEEK_SET);
            
            memset(buf,0,1024);
                
            size = fread(buf,1,1024,fp);
    
            appl_comm_i03t_request_upgrade(COMM_I03T_PORT,
                        0xff,
                        DataId_Upgrade_Header,
                        buf,
                        size,
                        pack_no++,
                        0);
            
            osDelay(1000);

            while(1) {
                
                feed_watchdog(); 
                
                memset(buf,0,1024);
                
                size = fread(buf,1,1024,fp);

                
                appl_comm_i03t_request_upgrade(COMM_I03T_PORT,
                        0xff,
                        DataId_Upgrade_FileData,
                        buf,
                        size,
                        pack_no++,
                        0);
                
                osDelay(100);
                
                if(size != 1024) {
                    break;
                }
            } 
        }
        
        osThreadResume(tid_thread_comm_i03t);

        result = errOK;

    }
    
EXIT:
    if(fp) {
        fclose(fp);
    }
    
    if(buf) {
        sys_free(buf);
    }
    
    return result;
}

errStatus_t appl_usbh_upgrade(void) {
    errStatus_t result = errErr;
    fsFileInfo info;
    fsStatus status;
    
    uint32_t file_sum = 0;
    uint32_t file_size = 0;
    uint16_t pack_size = 0;
    
    uint32_t temp_sum = 0;
    uint32_t temp_size = 0;
    
    memset(&info,0,sizeof(info));
    
    char path[64] = {0};
    strcpy(path,DRVIER_UPGRADE_PATH"I03M*.bin");
    status =  ffind (path, &info);
    if(status != fsOK) {
        return errErr;
    }
    
    if(info.size > APP_SIZE) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));
    
    sprintf(path,DRVIER_UPGRADE_PATH"%s",info.name);
    
    debug_printf("File path:%s\r\n",path);
    
    FILE *fp = NULL;
    fp = fopen(path,"r");
    if(fp == NULL) {
        debug_printf("File path:%s open failed.\r\n",path);
        return errErr;
    }
    
    fseek(fp,0,SEEK_SET); 

    uint8_t *buf = sys_malloc(1024);
    
    memset(buf,0,1024);
    size_t size = fread(buf,1,1024,fp);
    if(size != 1024) {
        goto EXIT;
    }
    

    if(buf[0] != 0x55 || buf[1] != 0xAA) {
        goto EXIT;
    }


    //file size
    file_size =  (uint32_t)buf[24] << 24 \
                |(uint32_t)buf[25] << 16 \
                |(uint32_t)buf[26] << 8 \
                |(uint32_t)buf[27];
    
    file_sum =   (uint32_t)buf[28] << 24 \
                |(uint32_t)buf[29] << 16 \
                |(uint32_t)buf[30] << 8 \
                |(uint32_t)buf[31];
    
    pack_size = (uint16_t)buf[32] << 8 \
                |buf[33];
    
    if(pack_size != 1024) {
        goto EXIT;
    }
    
    if(!memcmp(buf,(uint8_t *)APP_BACK_ADDRESS,1024)) { 
        IAP_Init(file_size,file_sum,pack_size,0);
        if(IAP_FlashCheck()) { 
            IAP_Deinit();
            goto EXIT;
        }
    }
    

    while(1) {
        feed_watchdog(); 
        memset(buf,0,1024);
        size = fread(buf,1,1024,fp);
        for(int i=0;i<size;i++) {
            temp_sum += buf[i];
        }
        temp_size += size;
        
        if(size != 1024) {
            break;
        }
    }
    
    if(temp_size == file_size && temp_sum == file_sum) {
        
        IAP_Init(file_size,file_sum,pack_size,1);
        
        fseek(fp,0,SEEK_SET);
        
        uint32_t write_addr = APP_BACK_ADDRESS;
        
        while(1) {
            
            feed_watchdog(); 
            
            memset(buf,0,1024);
            
            size = fread(buf,1,1024,fp);
            
            for(uint8_t i=0;i<3;i++) {
                if(storage_writebytes(write_addr,buf,size)) {
                    break;
                }
                osDelay(10);
            }
            
            write_addr += size;
            if(size != 1024) {
                break;
            }
            
           
        } 

        if(IAP_FlashCheck()) {   
            
            command_msg_put(SysCmdReset,NULL,0);
            
            logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_UPGRADE,0,NULL,__FILE__,__LINE__);

            result =  errOK;
        } else {
            IAP_Deinit();
        }
    }
    
EXIT:
    if(fp) {
        if(ferror(fp)) {
            clearerr(fp);
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
        fclose(fp);
    }
    
    if(buf) {
        sys_free(buf);
    }
    
    return result;
}
