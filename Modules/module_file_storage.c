#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#if OS_FREERTOS == OS_TYPE
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "module_file_storage.h"
#include "rl_fs.h"

#include "thread_debug.h"

#include "module_hist.h"
#include "module_alarm.h"
#include "module_index.h"

#include "i03t_list.h"
#include "logger.h"






static errStatus_t InitFileSys (void) {
  
    static fsStatus stat;
    
    const  char *opt = "/L I03M /FAT16 /LLEB";
    
    osDelay(1000);

    /* Initialize and mount drive "N0" */
    stat = finit (DRVIER_NAME);

    if (stat == fsOK) {

        for(uint8_t i = 0; i < 10; i++) {
            stat = fmount (DRVIER_NAME);
            if (stat == fsOK) {
                debug_printf("File System mount OK\r\n");
                return errOK;
            } 
            osDelay(100);
        }
 
        if (stat == fsNoFileSystem) {
            
            logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FS_FORMAT,0,NULL,__FILE__,__LINE__);
            
            for(uint8_t i=0;i<20;i++) {
                stat = fformat (DRVIER_NAME, opt);
                if (stat == fsOK) {
                    debug_printf("File System Format OK\r\n");
                    if(module_file_dir() == 0) {
                        return errFmtFilSys;
                    }
                }
                
                osDelay(100);
            }
            
        } 
    } 
    
    return errErr;
}

bool module_file_del(char *path) {
    return fdelete(path,NULL) == fsOK;
}

bool module_file_new(char *path,uint8_t *pdata,uint32_t length) {
    
    FILE *fp = NULL;
    
    fp = fopen(path,"w");
    
    if(fp == NULL) {
        return false;
    }
 
    fwrite(pdata,length,1,fp);
    if(ferror(fp)) {
        clearerr(fp);
        logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    }
    fclose(fp);
    
    fsFileInfo info;
    fsStatus status;
    memset(&info,0,sizeof(info));
    status =  ffind (path, &info);
    if (status != fsOK) {
        return false;
    }
    
    return (info.size == length);
    
}

errStatus_t module_file_test(void) {
    char path[24] = {0};
    sprintf((char *)path,"t%d.txt",rand()%10000);
    if(!module_file_new((char *)path,(uint8_t *)path,strlen(path))) {
        module_file_del(path);
        return errErr;
    }
    
    module_file_del(path);
    return errOK;
}

void WaitSysIdle(void);
bool module_file_create(char *path,FileHead_t *head,uint16_t head_size,uint32_t file_size) {

    #define ONCE_SIZE 4096
    
    FILE *fp = NULL;
    
#if FILL_FILE
    uint8_t *p = sys_malloc(ONCE_SIZE);
    if(p == NULL) {
        return false;
    }
    
    memset(p,0xff,ONCE_SIZE);
#endif
    
    fp = fopen(path,"w+");
    
    if(fp == NULL) {
#if FILL_FILE
        sys_free(p);
#endif
        return false;
    }
#if FILL_FILE
    uint16_t write_count = file_size / ONCE_SIZE + 1;

    uint32_t tick = osKernelGetTickCount();
    
    debug_printf("Start create file:%s...\r\n",path);

    while(write_count > 0) 
    {
        
        fwrite(p,ONCE_SIZE,1,fp);
		if(ferror(fp)) {
			clearerr(fp);
			logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
			//    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
        
        write_count--;
        
        int32_t tick_diff = (int32_t)(osKernelGetTickCount() - tick);
        
        if(abs(tick_diff) > FILE_DELAY) {
            tick = osKernelGetTickCount();
            osDelay(5);
        }

        WaitSysIdle();
        
        feed_watchdog();
        
        if(bsp_pvd_get_power_flag()) {
            break;
        }
    }
    sys_free(p);
#endif
    if(ferror(fp)) {
        clearerr(fp);
    }
    fclose(fp);
    debug_printf("Create File:%s OK\r\n",path);
    return true;
}
#if 0
bool module_file_create_test(char *path,FileHead_t *head,uint16_t head_size,uint32_t file_size) {

    #define ONCE_SIZE 4096
    
    FILE *fp = NULL;
    

    uint8_t *p = sys_malloc(ONCE_SIZE);
    if(p == NULL) {
        return false;
    }
    
    memset(p,0xff,ONCE_SIZE);

    
    fp = fopen(path,"w+");
    
    if(fp == NULL) {

        sys_free(p);

        return false;
    }

    uint16_t write_count = file_size / ONCE_SIZE + 1;

    uint32_t tick = osKernelGetTickCount();
    
    debug_printf("Start create file:%s...\r\n",path);

    while(write_count > 0) 
    {
        
        fwrite(p,ONCE_SIZE,1,fp);
		if(ferror(fp)) {
			clearerr(fp);
			logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
			//    logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
        }
        
        write_count--;
        
        int32_t tick_diff = (int32_t)(osKernelGetTickCount() - tick);
        
        if(abs(tick_diff) > FILE_DELAY) {
            tick = osKernelGetTickCount();
            osDelay(5);
        }

        WaitSysIdle();
        
        feed_watchdog();
        
        if(bsp_pvd_get_power_flag()) {
            break;
        }
    }
    sys_free(p);

    if(ferror(fp)) {
        clearerr(fp);
    }
    fclose(fp);
    
    if(bsp_pvd_get_power_flag()) {
        debug_printf("power down.\r\n");
    }
     
    debug_printf("Create File:%s OK\r\n",path);
    return true;
}
#endif

bool module_file_exist(char *path) {
    fsFileInfo info;
    fsStatus status;
    memset(&info,0,sizeof(info));
    status =  ffind (path, &info);
    return (status == fsOK);
}

const SYS_FILE_t sys_files[] = {
    { FILE_HIST_DATA_1,  FILE_HIST_DATA_MAX_CNT, sizeof(CellHistStorage_t), 1 },
    { FILE_HIST_DATA_2,  FILE_HIST_DATA_MAX_CNT, sizeof(CellHistStorage_t), 2 },
    { FILE_HIST_DATA_3,  FILE_HIST_DATA_MAX_CNT, sizeof(CellHistStorage_t), 3 },
    { FILE_HIST_DATA_4,  FILE_HIST_DATA_MAX_CNT, sizeof(CellHistStorage_t), 4 },
    { FILE_HIST_DATA_5,  FILE_HIST_DATA_MAX_CNT, sizeof(CellHistStorage_t), 5 },
    { FILE_HIST_DATA_6,  FILE_HIST_DATA_MAX_CNT, sizeof(CellHistStorage_t), 6 },
    
    { FILE_ALARM_DATA_1,  FILE_ALARM_DATA_MAX_CNT, sizeof(AlarmStorage_t), 1},
    { FILE_ALARM_DATA_2,  FILE_ALARM_DATA_MAX_CNT, sizeof(AlarmStorage_t), 2},
    { FILE_ALARM_DATA_3,  FILE_ALARM_DATA_MAX_CNT, sizeof(AlarmStorage_t), 3},
    { FILE_ALARM_DATA_4,  FILE_ALARM_DATA_MAX_CNT, sizeof(AlarmStorage_t), 4},
    { FILE_ALARM_DATA_5,  FILE_ALARM_DATA_MAX_CNT, sizeof(AlarmStorage_t), 5},
    { FILE_ALARM_DATA_6,  FILE_ALARM_DATA_MAX_CNT, sizeof(AlarmStorage_t), 6},
    
#if  FILE_DISCHARGE_ENABLE == 1
    { FILE_DISCHARGE_DATA_1,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Discharge_t), 1},
    { FILE_DISCHARGE_DATA_2,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Discharge_t), 2},
    { FILE_DISCHARGE_DATA_3,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Discharge_t), 3},
    { FILE_DISCHARGE_DATA_4,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Discharge_t), 4},
    { FILE_DISCHARGE_DATA_5,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Discharge_t), 5},
    { FILE_DISCHARGE_DATA_6,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Discharge_t), 6},
    
    { FILE_CHARGE_DATA_1,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Charge_t), 1},
    { FILE_CHARGE_DATA_2,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Charge_t), 2},
    { FILE_CHARGE_DATA_3,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Charge_t), 3},
    { FILE_CHARGE_DATA_4,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Charge_t), 4},
    { FILE_CHARGE_DATA_5,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Charge_t), 5},
    { FILE_CHARGE_DATA_6,  FILE_DISCHARGE_DATA_MAX_CNT, sizeof(Charge_t), 6},
#endif
};

#define SYS_FILE_SIZE  sizeof(sys_files)/sizeof(sys_files[0])
errStatus_t module_logger_check(void);
void module_file_check(void) {
    
    FileHead_t file_head = {0};
    
    for(int i=0;i<SYS_FILE_SIZE;i++) {
        if(i03t_node_find(sys_files[i].i03t_addr) != NULL) {
            if(!module_file_exist((char *)sys_files[i].file)) {
                module_file_create((char *)sys_files[i].file,\
                                    &file_head,\
                                    sizeof(FileHead_t),\
                                    sys_files[i].max_count * sys_files[i].storge_size);
            }
        }
    }

    module_logger_check();     
}
void module_i03t_file_flush(uint8_t i03t_addr) {
    for(int i=0;i<SYS_FILE_SIZE;i++) {
        if(sys_files[i].i03t_addr == i03t_addr) {
            module_file_create((char *)sys_files[i].file,NULL,0,0);
        }
    } 
}

void module_i03t_file_del(uint8_t i03t_addr) {
    for(int i=0;i<SYS_FILE_SIZE;i++) {
        if(sys_files[i].i03t_addr == i03t_addr) {
            if(module_file_exist((char *)sys_files[i].file)) {      
#if FILE_DELETE
                module_file_del((char *)sys_files[i].file);
#else
                module_file_create((char *)sys_files[i].file,NULL,0,0);
#endif
            }
        }
    } 
}

void module_i03t_del_hist(uint8_t i03t_addr) {
    char path[32] = {0};
    
    sprintf(path,"hist%d.txt",i03t_addr);
    if(module_file_exist(path)) {
#if FILE_DELETE
        module_file_del(path);
#else
        module_file_create(path,NULL,0,0);
#endif
    }

}

void module_i03t_del_alarm(uint8_t i03t_addr) {
    char path[32] = {0};
    
    sprintf(path,"alarm%d.txt",i03t_addr);
    if(module_file_exist(path)) {
#if FILE_DELETE
        module_file_del(path);
#else
        module_file_create(path,NULL,0,0);
#endif
    }
}

void module_i03t_del_discharge(uint8_t i03t_addr) {
    char path[32] = {0};
    
    sprintf(path,"dis%d.txt",i03t_addr);
    if(module_file_exist(path)) {
#if FILE_DELETE
        module_file_del(path);
#else
        module_file_create(path,NULL,0,0);
#endif
    }
}

void module_i03t_del_charge(uint8_t i03t_addr) {
    char path[32] = {0};
    
    sprintf(path,"charge%d.txt",i03t_addr);
    if(module_file_exist(path)) {
#if FILE_DELETE
        module_file_del(path);
#else
        module_file_create(path,NULL,0,0);
#endif
    }
}

errStatus_t module_charge_load(I03T_Info_t *i03t);
errStatus_t module_hist_load(I03T_Info_t *i03t);
errStatus_t module_alarm_load(I03T_Info_t *i03t);
errStatus_t module_discharge_load(I03T_Info_t *i03t);
void module_file_load_all(void) {
    for(int i03t_addr = 1; i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {
        I03T_Info_t *node = i03t_node_find(i03t_addr);
        
        errStatus_t status;
        
        if(node != NULL) {
            
            node->alarm.index = 0;
            node->hist.index = 0;
            node->discharge.index = 0;
            node->charge.index = 0;
            node->discharge.soc = 1000;
            node->discharge.soh = 1000;
            
            
            module_alarm_load(node);
            
#if FILE_DISCHARGE_ENABLE == 1
            module_discharge_load(node);
            module_charge_load(node);
#endif
            status = module_hist_load(node);
            if(status == errOK) {
                node->discharge.soc = node->hist.total_info.discharge.soc;
                node->discharge.soh = node->hist.total_info.discharge.soh;
            }
        }
    }
}

uint32_t module_file_dir(void) {
    uint32_t cnt = 0;
    fsStatus status;
    int32_t freebytes;
    fsDriveInfo drinfo;
    fsFileInfo info;
    
    freebytes = ffree(DRVIER_NAME);
    status = finfo(DRVIER_NAME,&drinfo);
    
    debug_printf("result:%d,capacity:%d,",status,drinfo.capacity);
    debug_printf("free bytes:%d\r\n",freebytes);
    

    info.fileID = 0; 
 
    while (ffind (DRVIER_NAME"*.*", &info) == fsOK) {
        debug_printf ("\n%-32s %8d bytes,Id:%4d",
                info.name,
                info.size,
                info.fileID);
        
        cnt++;
        
        if(cnt > CONFIG_MAX_FILES) {
            break;
        }
    }
    
    return cnt;
}

errStatus_t module_file_init(void) {
    
    errStatus_t err = InitFileSys();
    if(err == errErr) {
        configASSERT(false);
    }
    
    return err;
}
errStatus_t module_hist_index_check(I03T_Info_t *i03t);
errStatus_t module_alarm_index_check(I03T_Info_t *i03t);
errStatus_t module_discharge_index_check(I03T_Info_t *i03t);

void module_file_load(void) {

    errStatus_t err = module_file_init();
    
    module_file_check(); 
    
    if(FileIndex_load(&pDEApp->index)) {
        if(err == errFmtFilSys) {
            for(int i=0;i<CONFIG_MAX_I03T;i++) {
                pDEApp->index.index_info[i].index_alarm = 0;
                pDEApp->index.index_info[i].index_discharge = 0;
                pDEApp->index.index_info[i].index_hist = 0;
                pDEApp->index.index_info[i].index_charge = 0;
                pDEApp->index.index_info[i].date.date = 0;
                
            }
    
            pDEApp->index.logger_index = 0;
        } 

        debug_printf("File index load success.\r\n");
        
        for(int i03t_addr = 1; i03t_addr<=CONFIG_MAX_I03T; i03t_addr++) {
            I03T_Info_t *i03t = i03t_node_find(i03t_addr);
            if(i03t != NULL) {
                i03t->hist.index = pDEApp->index.index_info[i03t_addr-1].index_hist;
                i03t->alarm.index = pDEApp->index.index_info[i03t_addr-1].index_alarm;
                i03t->discharge.index = pDEApp->index.index_info[i03t_addr-1].index_discharge;
                i03t->charge.index = pDEApp->index.index_info[i03t_addr-1].index_charge;
                i03t->discharge.soc = pDEApp->index.index_info[i03t_addr-1].soc;
                i03t->discharge.soh = pDEApp->index.index_info[i03t_addr-1].soh;
                i03t->discharge.deep_discharge_cycle = pDEApp->index.index_info[i03t_addr-1].deep_discharge_cycle;
                
                i03t->soc_store_time.date = pDEApp->index.index_info[i03t_addr-1].date.date;
                
                debug_printf("I03T[%d],hist@%d,alarm@%d,discharge@%d,charge@%d,SOC:%d,SOH:%d. \r\n",i03t_addr,\
                                                                        i03t->hist.index,\
                                                                        i03t->alarm.index,\
                                                                        i03t->discharge.index,\
                                                                        i03t->charge.index,\
                                                                        i03t->discharge.soc,\
                                                                        i03t->discharge.soh);
                
            }
        } 
        
        
    } else {
        debug_printf("File index load failed,start search file index.\r\n");
        module_file_load_all();
        pDEApp->index.logger_index = 0;
    }
    
    
}

void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length);
bool module_file_system_format(void) {
 
    static fsStatus stat;
    const  char *opt = "/L I03M /FAT16 /LLEB";

#if 0    
    stat = finit (DRVIER_NAME);
    if(stat != fsOK) {
        result = false; 
        goto EXIT;
    }

    stat = fmount (DRVIER_NAME);
    if(stat != fsOK) {
        debug_printf("File System fmount Failed\r\n");
        result =  false; 
        goto EXIT;
    }
#endif

    stat = fformat (DRVIER_NAME, opt);

    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {  
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL) {
            i03t_node->alarm.index = 0;
            i03t_node->discharge.index = 0;
            i03t_node->charge.index = 0;
            i03t_node->hist.index = 0;
            i03t_node->soc_store_time.date = 0;
        }
    }
    
    for(int i=0;i<CONFIG_MAX_I03T;i++) {
        pDEApp->index.index_info[i].index_alarm = 0;
        pDEApp->index.index_info[i].index_discharge = 0;
        pDEApp->index.index_info[i].index_charge = 0;
        pDEApp->index.index_info[i].index_hist = 0;
        pDEApp->index.index_info[i].date.date = 0;
    }
    
    pDEApp->index.logger_index = 0;
    
    FileIndex_save(pDEApp->index);

    return (stat == fsOK);

}

