#include "main.h"
#include "module_sn.h"
#include "crc_check.h"
#include "appl_para.h"
#include "thread_debug.h"


#define SN_FILE_SIZE  CONFIG_MAX_I03T * CONFIG_MAX_CELL * sizeof(SNStore_t)
    
    
errStatus_t module_sn_find(uint8_t sn[]) {
    
    uint32_t addr = ADDR_SN_CONFIG;
    uint16_t sn_index = 0;
    uint16_t sn_max = CONFIG_MAX_I03T * CONFIG_MAX_CELL;
    SNStore_t sn_info = {0};
    for(sn_index = 0;sn_index < sn_max;sn_index++,addr += sizeof(SNStore_t)) {
        storage_readbytes(addr,(uint8_t *)&sn_info,sizeof(SNStore_t));
        
        uint32_t crc = CRC_Get32(((uint8_t *)(&sn_info)) + 4,sizeof(SNStore_t) - 4);
        if(crc == sn_info.crc_check) {
            if(!memcmp(sn_info.sn,sn,CONFIG_SN_LENGTH)) {
                return errOK;
            }
        }
    }
    
    return errErr;
}

uint16_t module_sn_get_count(uint8_t i03t,bool print) {
    uint16_t count = 0;
    uint16_t i03t_cell_count = 0;
    uint32_t addr = ADDR_SN_CONFIG;
    uint16_t sn_index = 0;
    uint16_t sn_max = CONFIG_MAX_I03T * CONFIG_MAX_CELL;
    SNStore_t sn_info = {0};
    
    uint16_t i03t_cells[CONFIG_MAX_I03T] = {0};
    memset(i03t_cells,0,sizeof(i03t_cells));
    
    if(i03t == 0) {
        for(uint8_t i=0;i<CONFIG_MAX_I03T;i++) {
            //pDEApp->device_config.i03t_nodes[i].cell_number = 0;
            i03t_cells[i] = 0;
        }
    }
    
    for(sn_index = 0;sn_index < sn_max;sn_index++,addr += sizeof(SNStore_t)) {
        memset(&sn_info,0,sizeof(SNStore_t));
        storage_readbytes(addr,(uint8_t *)&sn_info,sizeof(SNStore_t));
        
        uint32_t crc = CRC_Get32(((uint8_t *)(&sn_info)) + 4,sizeof(SNStore_t) - 4);
        if(crc == sn_info.crc_check) {
            
            count++;
            
            if(i03t == 0) {
                if(sn_info.i03t_addr > 0 && sn_info.i03t_addr <= CONFIG_MAX_I03T) {
                    //pDEApp->device_config.i03t_nodes[sn_info.i03t_addr - 1].cell_number++;
                    i03t_cells[sn_info.i03t_addr - 1]++;
                }
            } else {
                if(sn_info.i03t_addr == i03t) {
                    i03t_cell_count++;
                }
            }

            if(print) {
                char sn[CONFIG_SN_LENGTH + 1] = {0};
                char cloud_id[CONFIG_CLOUD_ID_LENGTH + 1] = {0};
                memcpy(sn,sn_info.sn,CONFIG_SN_LENGTH);
                memcpy(cloud_id,sn_info.cloud_id,CONFIG_CLOUD_ID_LENGTH);
                sn[CONFIG_SN_LENGTH] = 0x00;
                cloud_id[CONFIG_CLOUD_ID_LENGTH] = 0x00;
                debug_printf("I03T:%02d,CELL:%03d,SN:%s,Group:%d,CloudId:%s\r\n",sn_info.i03t_addr,sn_info.cell_id,sn,sn_info.cell_on_current_group,cloud_id);
            }  
        }
    }
    
    pDEApp->device_config.i03m.cell_numbers = count;
    
    if(i03t == 0) {
        for(uint8_t i=0;i<CONFIG_MAX_I03T;i++) {
            pDEApp->device_config.i03t_nodes[i].sys_para.cell_number = i03t_cells[i]; 
        }
    }
    
    if(i03t != 0 && i03t <= CONFIG_MAX_I03T) {
        pDEApp->device_config.i03t_nodes[i03t - 1].sys_para.cell_number = i03t_cell_count;
    }
    
    for(uint8_t i=0;i<CONFIG_MAX_I03T && print;i++) {
        char cloud_id[CONFIG_CLOUD_ID_LENGTH + 1] = {0};
        memset(cloud_id,0,sizeof(cloud_id));
        memcpy(cloud_id,pDEApp->device_config.i03t_nodes[i].base_para.cloud_id,CONFIG_CLOUD_ID_LENGTH);
        debug_printf("Find I03T_%d[%d] mount@%d,%s, SN Count:%d\r\n",i+1,pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr,\
                                                            pDEApp->device_config.i03t_nodes[i].base_para.mount,\
                                                            cloud_id,\
                                                            pDEApp->device_config.i03t_nodes[i].sys_para.cell_number);
    }

    return count;
}

SNStore_t *module_sn_get_by_index(uint8_t i03t_addr,uint16_t cell_id) {
    
    if(cell_id > CONFIG_MAX_CELL ||  cell_id == 0) {
        return NULL;
    }
    
    if(i03t_addr > CONFIG_MAX_I03T || i03t_addr == 0) {
        return NULL;
    }

    uint16_t sn_index = (i03t_addr - 1) * CONFIG_MAX_CELL + (cell_id - 1);
    uint32_t addr = ADDR_SN_CONFIG + sn_index * sizeof(SNStore_t);
    SNStore_t *sn_info = sys_malloc(sizeof(SNStore_t));
    if(sn_info == NULL) {
        return NULL;
    }
    
    storage_readbytes(addr,(uint8_t *)sn_info,sizeof(SNStore_t));
 
    uint32_t crc = CRC_Get32(((uint8_t *)(sn_info)) + 4,sizeof(SNStore_t) - 4);
    if(crc == sn_info->crc_check) {
        return sn_info;
    }
    
    sys_free(sn_info);
    return NULL;  
}

SNStore_t *module_sn_get(uint8_t sn[]) {
    
    uint32_t addr = ADDR_SN_CONFIG;
    uint16_t sn_index = 0;
    uint16_t sn_max = CONFIG_MAX_I03T * CONFIG_MAX_CELL;
    SNStore_t *sn_info = sys_malloc(sizeof(SNStore_t));
    if(sn_info == NULL) {
        return NULL;
    }
    for(sn_index = 0;sn_index < sn_max;sn_index++,addr += sizeof(SNStore_t)) {
        storage_readbytes(addr,(uint8_t *)sn_info,sizeof(SNStore_t));
        
        uint32_t crc = CRC_Get32(((uint8_t *)(sn_info)) + 4,sizeof(SNStore_t) - 4);
        if(crc == sn_info->crc_check) {
            if(!memcmp(sn_info->sn,sn,CONFIG_SN_LENGTH)) {
                return sn_info;
            }
        }
    }
    sys_free(sn_info);
    return NULL;
}

errStatus_t module_sn_add_mult(SNStore_t sn[],uint16_t num) {
    if(sn == NULL) {
        return errErr;
    }
    
    uint8_t i03t = sn[0].i03t_addr;
    uint16_t cell_id = sn[0].cell_id;
    
    for(uint16_t i=0;i<num;i++) {
        
        SNStore_t *temp_sn = (SNStore_t *)(sn + i);
        if(temp_sn->cell_id > CONFIG_MAX_CELL || temp_sn->cell_id == 0) {
            return errErr;
        }
        if(temp_sn->i03t_addr > CONFIG_MAX_I03T || temp_sn->i03t_addr == 0) {
            return errErr;
        }
        if(module_sn_find(temp_sn->sn) == osOK) {
            return errErr;
        }
        
        temp_sn->crc_check = CRC_Get32(((uint8_t *)(temp_sn)) + 4,sizeof(SNStore_t) - 4);
    }
    
    uint32_t addr = ADDR_SN_CONFIG + (i03t - 1) * (CONFIG_MAX_CELL * sizeof(SNStore_t)) \
        + (cell_id - 1)*sizeof(SNStore_t);
    
    uint8_t *buf = (uint8_t *)sys_malloc(sizeof(SNStore_t) * num);
    if(buf == NULL) {
        return errErr;
    }
    
    storage_readbytes(addr, buf, sizeof(SNStore_t));
    
    bool backup = false;
    for(uint16_t i=0; i<(sizeof(SNStore_t) * num);i++) {
        if(buf[i] != 0xff) {
            backup = true;
            break;
        }
    }
    
    uint32_t sn_cnt = 0;
    
    uint8_t path[32] = {0};
    
    sprintf((char *)path,FILE_SN"%d.txt",rand()%10000);
    
    if(backup) {
        SNStore_t *sn_info = NULL;
        if(!module_file_new((char *)path,(uint8_t *)ADDR_SN_CONFIG,SN_FILE_SIZE)) {
            sys_free(buf);
            return errErr;
        }
        
        FILE *fp = NULL;
        fp = fopen((char *)path,"r");
        if(fp == NULL) {
            sys_free(buf);
            return errErr;
        }
        
        storage_erasesector(ADDR_SN_CONFIG);
        
        osDelay(5);
        
        uint32_t addr_temp = ADDR_SN_CONFIG;
        
        uint16_t count = 0;
        
        while(1) {
            size_t size = fread(buf,sizeof(SNStore_t),1,fp);
            if(size == 1) {
                
                sn_cnt ++;
                sn_info = (SNStore_t *)buf;
                if(addr == addr_temp && count < num) {
                    addr += sizeof(SNStore_t); 
                    count++;
                    sn_info = (SNStore_t *)(sn + count);
                } 
                
                if(sn_info->cell_id == 0xffff) {
                    addr_temp += sizeof(SNStore_t); 
                    continue;
                } 
                
                for(uint8_t try_times = 0;try_times < 3;try_times++) {
                    if(storage_writebytes(addr_temp,(uint8_t *)sn_info,sizeof(SNStore_t))) {
                        break; 
                    }
                    osDelay(5);
                }

                addr_temp += sizeof(SNStore_t);    
                
            } else {
                break;
            }
        }
        
        if(ferror(fp)) {
            clearerr(fp);
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            //    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }

        fclose(fp);
        
        fdelete((const char *)path,NULL);
        
    } else {
        for(uint8_t try_times = 0;try_times < 3;try_times++) {
            if(storage_writebytes(addr,(uint8_t *)sn,sizeof(SNStore_t) * num)) {
                sn_cnt = CONFIG_MAX_TOTAL_CELL;
                break; 
            }
            osDelay(5);
        }
    }
    
    sys_free(buf);

    return sn_cnt == CONFIG_MAX_TOTAL_CELL ? errOK : errErr;

}

errStatus_t module_sn_add(SNStore_t *sn) {
    
    uint32_t sn_cnt = 0;
    
    if(sn == NULL) {
        return errErr;
    }
    if(sn->cell_id > CONFIG_MAX_CELL || sn->cell_id == 0) {
        return errErr;
    }
    
    if(sn->i03t_addr > CONFIG_MAX_I03T || sn->i03t_addr == 0) {
        return errErr;
    }
    
    if(sn->cell_on_current_group > CONFIG_MAX_CURRENT_CH ||  sn->cell_on_current_group < 1) {
        return errErr;
    }
    if(module_sn_find(sn->sn) == osOK) {
        return errErr;
    }

    uint32_t addr = ADDR_SN_CONFIG + (sn->i03t_addr - 1) * (CONFIG_MAX_CELL * sizeof(SNStore_t)) \
        + (sn->cell_id - 1)*sizeof(SNStore_t);
    
    uint8_t *buf = (uint8_t *)sys_malloc(sizeof(SNStore_t));
    if(buf == NULL) {
        return errErr;
    }
    
    storage_readbytes(addr, buf, sizeof(SNStore_t));

    bool backup = false;
    for(uint16_t i=0; i<sizeof(SNStore_t);i++) {
        if(buf[i] != 0xff) {
            backup = true;
            break;
        }
    }
    
    sn->crc_check = CRC_Get32(((uint8_t *)(sn)) + 4,sizeof(SNStore_t) - 4);
    
    if(backup) {
        
        SNStore_t *sn_info = (SNStore_t *)buf;
        /* if the cell id's sn is exist */
        uint32_t crc = CRC_Get32(((uint8_t *)(sn_info)) + 4,sizeof(SNStore_t) - 4);
        if(crc == sn_info->crc_check) {
            sys_free(buf);
            return errErr;
        }
        
        uint8_t path[32] = {0};
        
        sprintf((char *)path,FILE_SN"%d.txt",rand()%10000);
        if(!module_file_new((char *)path,(uint8_t *)ADDR_SN_CONFIG,SN_FILE_SIZE)) {
            sys_free(buf);
            return errErr;
        }
        
        FILE *fp = NULL;

        fp = fopen((char *)path,"r");
        
        if(fp == NULL) {
            sys_free(buf);
            return errErr;
        }
        
        storage_erasesector(ADDR_SN_CONFIG);
        
        osDelay(5);
        
        uint32_t addr_temp = ADDR_SN_CONFIG;
        
        while(1) {
            size_t size = fread(buf,sizeof(SNStore_t),1,fp);
            if(size == 1) {
                sn_cnt ++;
                
                sn_info = (SNStore_t *)buf;

                if(addr == addr_temp) {
                    sn_info = sn;
                } 
                
                if(sn_info->cell_id == 0xffff) {
                    addr_temp += sizeof(SNStore_t); 
                    continue;
                } 
                
                for(uint8_t try_times = 0;try_times < 3;try_times++) {
                    if(storage_writebytes(addr_temp,(uint8_t *)sn_info,sizeof(SNStore_t))) {
                        break; 
                    }
                    osDelay(5);
                }

                addr_temp += sizeof(SNStore_t);    
                
            } else {
                break;
            }
        }
        if(ferror(fp)) {
            clearerr(fp);
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
            //    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
        fclose(fp);
        fdelete((char *)path,NULL);
        
    } else {

        for(uint8_t try_times = 0;try_times < 3;try_times++) {
            if(storage_writebytes(addr,(uint8_t *)sn,sizeof(SNStore_t))) {
                sn_cnt = CONFIG_MAX_TOTAL_CELL;
                break; 
            }
            osDelay(5);
        }
    }
    
    sys_free(buf);

    return sn_cnt == CONFIG_MAX_TOTAL_CELL ? errOK : errErr;
}

errStatus_t  module_sn_replace(uint8_t i03t_addr,uint8_t sn_new[],uint8_t sn_old[]) {
    uint32_t sn_cnt = 0;

    if(module_sn_find(sn_new) == osOK) {
        return errErr;
    }
    
    SNStore_t *sn_info = module_sn_get(sn_old);
    if(sn_info == NULL) {
        return errErr;
    }

    uint32_t addr = ADDR_SN_CONFIG + (sn_info->i03t_addr - 1) * (CONFIG_MAX_CELL * sizeof(SNStore_t)) \
        + (sn_info->cell_id - 1)*sizeof(SNStore_t);
    
    uint8_t *buf = (uint8_t *)sys_malloc(sizeof(SNStore_t));
    if(buf == NULL) {
        sys_free(sn_info);
        return errErr;
    }


    memcpy(sn_info->sn,sn_new,CONFIG_SN_LENGTH);
    sn_info->crc_check = CRC_Get32(((uint8_t *)(sn_info)) + 4,sizeof(SNStore_t) - 4);

    uint8_t path[32] = {0};
    
    sprintf((char *)path,FILE_SN"%d.txt",rand()%10000);
    
    if(!module_file_new((char *)path,(uint8_t *)ADDR_SN_CONFIG,SN_FILE_SIZE)) {
        sys_free(sn_info);
        sys_free(buf);
        return errErr;
    }
    
    FILE *fp = NULL;

    fp = fopen((char *)path,"r");
    
    if(fp == NULL) {
        return errErr;
    }
    
    storage_erasesector(ADDR_SN_CONFIG);
    
    osDelay(5);
    
    uint32_t addr_temp = ADDR_SN_CONFIG;
    
    SNStore_t *sn_temp = NULL;
    
    while(1) {
        size_t size = fread(buf,sizeof(SNStore_t),1,fp);
        if(size == 1) {
            sn_cnt ++;
            
            sn_temp = (SNStore_t *)buf;

            if(addr == addr_temp) {
                sn_temp = sn_info;
            } 
            
            if(sn_temp->cell_id == 0xffff) {
                addr_temp += sizeof(SNStore_t); 
                continue;
            }
            
            for(uint8_t try_times = 0;try_times < 3;try_times++) {
                if(storage_writebytes(addr_temp,(uint8_t *)sn_temp,sizeof(SNStore_t))) {
                    break; 
                }
                osDelay(5);
            }

            addr_temp += sizeof(SNStore_t);            
        } else {
            break;
        }
    }
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);
    fdelete((char *)path,NULL);

    sys_free(buf);
    sys_free(sn_info);
    
    return sn_cnt == CONFIG_MAX_TOTAL_CELL ? errOK : errErr;
}

errStatus_t  module_sn_delete(SNStore_t *sn) {
    uint32_t sn_cnt = 0;

    SNStore_t *sn_info = module_sn_get(sn->sn);
    if(sn_info == NULL) {
        return errErr;
    }

    uint32_t addr = ADDR_SN_CONFIG + (sn->i03t_addr - 1) * (CONFIG_MAX_CELL * sizeof(SNStore_t)) \
        + (sn->cell_id - 1)*sizeof(SNStore_t);
    
    uint8_t *buf = (uint8_t *)sys_malloc(sizeof(SNStore_t));
    if(buf == NULL) {
        
        return errErr;
    }

    memset(sn_info,0xff,sizeof(SNStore_t));
    
    uint8_t path[32] = {0};
    
    sprintf((char *)path,FILE_SN"%d.txt",rand()%10000);    

    if(!module_file_new((char *)path,(uint8_t *)ADDR_SN_CONFIG,SN_FILE_SIZE)) {
        sys_free(sn_info);
        sys_free(buf);
        return errErr;
    }
    
    FILE *fp = NULL;

    fp = fopen((char *)path,"r");
    
    if(fp == NULL) {
        sys_free(buf);
        return errErr;
    }
    
    storage_erasesector(ADDR_SN_CONFIG);
    
    osDelay(5);
    
    uint32_t addr_temp = ADDR_SN_CONFIG;
    
    SNStore_t *sn_temp = NULL;
    
    while(1) {
        size_t size = fread(buf,sizeof(SNStore_t),1,fp);
        if(size == 1) {
            sn_cnt ++;
            
            sn_temp = (SNStore_t *)buf;

            if(addr == addr_temp) {
                sn_temp = sn_info;
            } else {
                if(sn_temp->cell_id == 0xffff) {
                    addr_temp += sizeof(SNStore_t); 
                    continue;
                }
            }

            for(uint8_t try_times = 0;try_times < 3;try_times++) {
                if(storage_writebytes(addr_temp,(uint8_t *)sn_temp,sizeof(SNStore_t))) {
                    break; 
                }
                osDelay(5);
            }

            addr_temp += sizeof(SNStore_t);            
        } else {
            break;
        }
    }
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);
    
    fdelete((char *)path,NULL);

    sys_free(buf);
    sys_free(sn_info);
    
    return sn_cnt == CONFIG_MAX_TOTAL_CELL ? errOK : errErr;
}

errStatus_t  module_sn_clear(uint8_t i03t_addr) {
    if(i03t_addr == 0) {
		storage_erasesector(ADDR_SN_CONFIG);
		return errOK;
	}

    uint32_t sn_cnt = 0;

    if(i03t_addr > CONFIG_MAX_I03T || i03t_addr == 0) {
        return errErr;
    }

    uint8_t *buf = (uint8_t *)sys_malloc(sizeof(SNStore_t));
    if(buf == NULL) {
        return errErr;
    }

    SNStore_t *sn_info = (SNStore_t *)buf;
    
    uint8_t path[32] = {0};
    
    sprintf((char *)path,FILE_SN"%d.txt",rand()%10000);

    if(!module_file_new((char *)path,(uint8_t *)ADDR_SN_CONFIG,SN_FILE_SIZE)) {
        sys_free(buf);
        return errErr;
    }
    
    FILE *fp = NULL;

    fp = fopen((char *)path,"r");
    
    if(fp == NULL) {
        sys_free(buf);
        return errErr;
    }
    
    storage_erasesector(ADDR_SN_CONFIG);
    
    osDelay(5);
    
    uint32_t addr_temp = ADDR_SN_CONFIG;
    
    while(1) {
        size_t size = fread(buf,sizeof(SNStore_t),1,fp);
        if(size == 1) {
            sn_cnt ++; 
            sn_info = (SNStore_t *)buf;
            if(sn_info->i03t_addr == i03t_addr) {
                memset(sn_info,0xff,sizeof(SNStore_t));
            } 
            for(uint8_t try_times = 0;try_times < 3;try_times++) {
                if(storage_writebytes(addr_temp,(uint8_t *)sn_info,sizeof(SNStore_t))) {
                    break; 
                }
                osDelay(5);
            }
            addr_temp += sizeof(SNStore_t);     
        } else {
            break;
        }
    }
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        //    //    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);
    
    fdelete((char *)path,NULL);

    sys_free(buf);

    return sn_cnt == CONFIG_MAX_TOTAL_CELL ? errOK : errErr;

}

void module_sn_test(void) {
    SNStore_t sn = {0};
    
    storage_erasesector(ADDR_SN_CONFIG);
    
    for(uint8_t i=0;i<CONFIG_MAX_I03T;i++) {
        
        for(uint16_t cell_id = 1;cell_id <= 100;cell_id++) {
            sn.i03t_addr = i + 1;
            sn.cell_id = cell_id;
            
            memset(sn.sn,cell_id,CONFIG_SN_LENGTH);
            sn.sn[0] = i+1;
            module_sn_add(&sn);

            if(module_sn_find(sn.sn) != errOK) {
                debug_printf("module_sn_add i03t:%d,cell_id:%d err!\r\n",sn.i03t_addr,sn.cell_id);
            } else {
                debug_printf("module_sn_add i03t:%d,cell_id:%d OK!\r\n",sn.i03t_addr,sn.cell_id);
            }
        }
    }
    uint8_t sn_new[CONFIG_SN_LENGTH];
    memset(sn_new,200,CONFIG_SN_LENGTH);
    module_sn_replace(sn.i03t_addr,sn_new,sn.sn);
    
    if(module_sn_find(sn_new) == errOK) {
        debug_printf("module_sn_replace i03t:%d,cell_id:%d OK!\r\n",sn.i03t_addr,sn.cell_id);
    } else {
        debug_printf("module_sn_replace i03t:%d,cell_id:%d Err.!\r\n",sn.i03t_addr,sn.cell_id);
    }
    
    sn.i03t_addr = 1;
    sn.cell_id = 1;
    memset(sn.sn,1,CONFIG_SN_LENGTH);
    module_sn_delete(&sn);
    
    if(module_sn_find(sn.sn) == errOK) {
        debug_printf("module_sn_delete i03t:%d,cell_id:%d err!\r\n",sn.i03t_addr,sn.cell_id);
    } else {
        debug_printf("module_sn_delete i03t:%d,cell_id:%d OK!\r\n",sn.i03t_addr,sn.cell_id);
    }
    
    module_sn_add(&sn);
    
    if(module_sn_find(sn.sn) == errOK) {
        debug_printf("module_sn_add_1 i03t:%d,cell_id:%d OK!\r\n",sn.i03t_addr,sn.cell_id);
    } else {
        debug_printf("module_sn_add_1 i03t:%d,cell_id:%d err!\r\n",sn.i03t_addr,sn.cell_id);
    }
    
}
