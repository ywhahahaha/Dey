#include "main.h"

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "sys_mem.h"
#include "thread_debug.h"
#include "i03t_list.h"
#include "module_hist.h"
#include "appl_para.h"
#include "module_file_storage.h"
#include "logger.h"


errStatus_t module_hist_store(I03T_Info_t *i03t);
errStatus_t module_alarm_store(I03T_Info_t *i03t);
errStatus_t module_discharge_store(I03T_Info_t *i03t); 
errStatus_t module_charge_store(I03T_Info_t *i03t);
errStatus_t module_charge_load(I03T_Info_t *i03t);
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
#define STORAGE_MSG_OBJECTS   100 
osMessageQueueId_t storageMsgId = NULL;
 
osThreadId_t tid_thread_storage;                        // thread id
 
void thread_storage_process (void *argument);                   // thread function

errStatus_t module_file_init(void);

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type) {

    if(storageMsgId == NULL) return;    
    
    if(pDEApp->Flag.bits.query_logger_flag ||
       pDEApp->Flag.bits.usb_busy) {
        return;
    }
    
    StorageMsg_t *msg = sys_malloc(sizeof(StorageMsg_t));
    if(msg != NULL) {
        memset(msg,0,sizeof(StorageMsg_t));
        msg->i03t_addr = i03t_addr;
        msg->storageType = storage_type;
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
        status = osMessageQueuePut(storageMsgId,&msg,osPriorityHigh,0);
        if(status != osOK) {
            if(msg->pdata != NULL) {
                sys_free(msg->pdata);
            }
            
            sys_free(msg);
            

        }
    }
}
 
errStatus_t thread_storage_init (void) {
 
    const osThreadAttr_t attr_thread = {
        .name = "thread_storage",
        .stack_size = 2048,
        //.priority = osPriorityHigh1, 
        .priority = osPriorityNormal, 
    };
        
    tid_thread_storage = osThreadNew(thread_storage_process, NULL, &attr_thread);
    if (tid_thread_storage == NULL) {
        return (errErr);
    }
 
    return(errOK);
}
bool module_file_create(char *path,FileHead_t *head,uint16_t head_size,uint32_t file_size); 
void module_i03t_file_del(uint8_t i03t_addr);
void module_4g_msg_put(ACTION_TYPE_t action,uint8_t *pdata,uint16_t length);
osTimerId_t timer_storage = NULL;

void file_store_cb(void *argument) {
    
    for(uint8_t i03t_addr=1;i03t_addr<=CONFIG_MAX_I03T;i03t_addr++) {
        //if(pDEApp->device_config.i03t_nodes[i].i03t_addr != 0 && pDEApp->device_config.i03t_nodes[i].i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR) 
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL && i03t_node->flag.bits.i03t_comm_once) {
             storage_msg_put(i03t_addr,
                             NULL,
                             0,
                             StorageHistData);
            
            i03t_node->flag.bits.i03t_synch_time = 1;
        }
    } 
    
    storage_msg_put(0,NULL,0,StorageFileIndex);

}

bool FileIndex_save(FileIndexStore_t msg);

void FileIndexUpdata(void) {
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {  
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL) {
            pDEApp->index.index_info[i03t_addr-1].index_alarm = i03t_node->alarm.index;
            pDEApp->index.index_info[i03t_addr-1].index_discharge = i03t_node->discharge.index;
            pDEApp->index.index_info[i03t_addr-1].index_charge = i03t_node->charge.index;
            pDEApp->index.index_info[i03t_addr-1].index_hist = i03t_node->hist.index;
            pDEApp->index.index_info[i03t_addr-1].soc = i03t_node->discharge.soc;
            pDEApp->index.index_info[i03t_addr-1].soh = i03t_node->discharge.soh;  
            pDEApp->index.index_info[i03t_addr-1].deep_discharge_cycle = i03t_node->discharge.deep_discharge_cycle;
            pDEApp->index.index_info[i03t_addr-1].date.date = i03t_node->soc_store_time.date;
        }
    }
    
    FileIndex_save(pDEApp->index);
}

void FileIndexClear(uint8_t i03t_addr) {
    
    if(i03t_addr == 0 || i03t_addr > CONFIG_MAX_IO3T_MODBUS_ADDR) {
        return;
    }
    
    pDEApp->index.index_info[i03t_addr-1].index_alarm = 0;
    pDEApp->index.index_info[i03t_addr-1].index_discharge = 0;
    pDEApp->index.index_info[i03t_addr-1].index_hist = 0;
    pDEApp->index.index_info[i03t_addr-1].index_charge = 0;
    pDEApp->index.index_info[i03t_addr-1].soc = 1000;
    pDEApp->index.index_info[i03t_addr-1].soh = 1000; 
    pDEApp->index.index_info[i03t_addr-1].deep_discharge_cycle = CONFIG_DEEP_DISCHARGE_CYCLE;
    pDEApp->index.index_info[i03t_addr-1].date.date = 0;
    
    FileIndex_save(pDEApp->index);
    
} 
extern osThreadId_t tid_thread_4g_tx;
extern osThreadId_t tid_thread_comm_i03t; 
errStatus_t  module_sn_clear(uint8_t i03t_addr);
errStatus_t appl_usbh_check_access(void);
errStatus_t appl_para_save(void);
errStatus_t module_hist_load(I03T_Info_t *i03t);
errStatus_t appl_usbh_upgrade(void);
errStatus_t module_logger_store(logger_msg_t msg);
errStatus_t appl_usbh_upgrade_i03t(void);
int32_t USBH_MSC_DriveUnmount (const char *drive_name);
void appl_usbh_copy_logger(uint8_t file);
void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length);
bool module_file_create_test(char *path,FileHead_t *head,uint16_t head_size,uint32_t file_size);
errStatus_t module_file_test(void);
void module_i03t_del_charge(uint8_t i03t_addr);

osMutexId_t  mutexID_One = NULL;                      /* 保存互斥量ID */
void thread_storage_process (void *argument) {  
    uint16_t status;    
    storageMsgId = osMessageQueueNew(STORAGE_MSG_OBJECTS,sizeof(void *),NULL);

    const osTimerAttr_t timer_attr = {
        .name = "file_store",
    };
    
    timer_storage = osTimerNew(file_store_cb,osTimerPeriodic,NULL,&timer_attr);
    if(pDEApp->device_config.i03m.storage_period  < 10) {
        pDEApp->device_config.i03m.storage_period = 300;
    }
    osTimerStart(timer_storage,pDEApp->device_config.i03m.storage_period * 1000ul);
    const osMutexAttr_t Thread_Mutex_attr =
    {
	.name         = "Mutex One",                         /* 互斥量名字 */
	.attr_bits    = osMutexPrioInherit | osMutexRobust,  /* 继承更高优先级(解决优先级反转问题），线程终止时自动释放互斥量 */
    };
    mutexID_One = osMutexNew(&Thread_Mutex_attr);     
    
    if(module_file_test() != errOK) {
        
        appl_noinit.file_test_err_cnt++;
        
        if(appl_noinit.file_test_err_cnt >= 3) {
            storage_msg_put(0,NULL,0,StorageFsFormat);
            appl_noinit.file_test_err_cnt = 0;
            
        } else {
            appl_noinit_store();
            HAL_NVIC_SystemReset();
        }
    } else {
        //
        logger_set_reset_reg();
    }
    
    appl_noinit.reset_cnt++;
    appl_noinit_store();
    
    RCC->CSR |= RCC_CSR_RMVF;     
    while (1) {
        uint32_t msg;
        osStatus_t status;
        status = osMessageQueueGet(storageMsgId,&msg,NULL,osWaitForever);
        if(status == osOK) {
            osMutexAcquire(mutexID_One,osWaitForever);
            I03T_Info_t *i03t_node = NULL;
            StorageMsg_t *storage_msg = (StorageMsg_t *)msg;
            if(msg != NULL) {
                if(!bsp_pvd_get_power_flag()) {
                    switch(storage_msg->storageType) {
                        case StorageParaInfor:
                            appl_para_save();
                            logger_infor_save_file(LOGGER_CONFIG,LOGGER_MORE_NONE,0,NULL,__FILE__,__LINE__);
                            break;
                        
                        case StorageHistData: {
                                i03t_node = i03t_node_find(storage_msg->i03t_addr);
                                module_hist_store(i03t_node);
                                FileIndexUpdata();
                            }
                            break;
                            
                        case StorageAlarmData:
                            {
                                i03t_node = i03t_node_find(storage_msg->i03t_addr);
                                module_alarm_store(i03t_node);
                                FileIndexUpdata();
                            }
                            break;
                        case StorageDischarge:
                            {
                                i03t_node = i03t_node_find(storage_msg->i03t_addr);
                                module_discharge_store(i03t_node);
                                FileIndexUpdata();
                            }
                            break; 
                            
                        case StorageCharge:
                            {
                                i03t_node = i03t_node_find(storage_msg->i03t_addr);
                                module_charge_store(i03t_node);
                                FileIndexUpdata();
                            }
                            break;
                        
                        case StorageFileIndex:
                            FileIndexUpdata();
                            break;

                        case StorageFileCheck:
                            module_file_check();
                            break;
                        
                        case StorageFileDelete:
                            module_i03t_file_del(storage_msg->i03t_addr); 
                            module_file_check();
                            break;
                        
                        case StorageFileFlush:
                            module_i03t_file_flush(storage_msg->i03t_addr);
                            module_file_check();
                            break;
                        
                        case  StorageFileDeleteHist:
                            module_i03t_del_hist(storage_msg->i03t_addr);
                            i03t_node = i03t_node_find(storage_msg->i03t_addr);
                            if(i03t_node != NULL) {
                                i03t_node->hist.index = 0;
                            }
                            module_file_check();
                            logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_DEL_HIST,storage_msg->i03t_addr,NULL,__FILE__,__LINE__);
                            break;
                                
                        case StorageFileDeleteAlarm:
                            module_i03t_del_alarm(storage_msg->i03t_addr);
                            i03t_node = i03t_node_find(storage_msg->i03t_addr);
                            if(i03t_node != NULL) {
                                i03t_node->alarm.index = 0;
                            }
                            module_file_check();
                            logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_DEL_ALARM,storage_msg->i03t_addr,NULL,__FILE__,__LINE__);

                            break;
                        
                        case StorageFileDeleteDischarge:
                            module_i03t_del_discharge(storage_msg->i03t_addr);
                            module_i03t_del_charge(storage_msg->i03t_addr);
                            i03t_node = i03t_node_find(storage_msg->i03t_addr);
                            if(i03t_node != NULL) {
                                i03t_node->discharge.index = 0;
                                i03t_node->charge.index = 0;
                            }
                            
                            
                            module_file_check();
                            logger_infor_save_file(LOGGER_OPERATE,LOGGER_MORE_DEL_DISCHARGE,storage_msg->i03t_addr,NULL,__FILE__,__LINE__);

                            break;
                        
                        case StorageHistRead:{
                                pDEApp->Flag.bits.query_logger_flag = 1;
                                LoggerQuery_t *query = (LoggerQuery_t *)storage_msg->pdata;
                               
                                module_hist_load_protocol(storage_msg->i03t_addr,query);
                            }
                            break;
                            
                        case StorageAlarmRead:
                            {
                                pDEApp->Flag.bits.query_logger_flag = 1;
                                LoggerQuery_t *query = (LoggerQuery_t *)storage_msg->pdata;
                                module_alarm_load_protocol(storage_msg->i03t_addr,query);
                            }
                            break;
                        
                        case StorageDischargeRead:
                            {
                                pDEApp->Flag.bits.query_logger_flag = 1;
                                LoggerQuery_t *query = (LoggerQuery_t *)storage_msg->pdata;
                                module_discharge_load_protocol(storage_msg->i03t_addr,query);
                            }
                            break;

                        case StorageFsFormat:
                            {
                                bool result = false;
                                osThreadSuspend(tid_thread_comm_i03t);
                                for(uint8_t trytimes = 0;trytimes < 10;trytimes++) {
                                    if( module_file_system_format()) {
                                        debug_printf("file system format success!\r\n");
                                        if(module_file_dir() == 0) {
                                            result = true;
                                            Bsp_Beeps(3);
                                            break;
                                        }
                                    } else {
                                        debug_printf("file system format failed!\r\n");
                                        module_file_dir();
                                    }
                                    
                                    osDelay(100);
                                }
                                logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FS_FORMAT,result,NULL,__FILE__,__LINE__);
                                osThreadResume(tid_thread_comm_i03t);
                                command_msg_put(SysCmdReset,NULL,0);
                            }
                            break;
                            
                        case StorageParaFactory:
                            appl_para_factory();
                            appl_para_save();
                            module_sn_clear(0); 
                            pDEApp->Flag.bits.usb_busy = 0;
                            storage_msg_put(0,NULL,0,StorageFsFormat);
                            break;
                        
                        case StorageUdisk:
                            pDEApp->Flag.bits.usb_busy = 1;
                            if(tid_thread_4g_tx != NULL) {
                                osThreadSuspend(tid_thread_4g_tx);
                            }
                            if(tid_thread_comm_i03t != NULL) {
                                osThreadSuspend(tid_thread_comm_i03t);
                            }
                            
                            if(appl_usbh_check_access() == errOK) {
                                if(appl_usbh_upgrade() != errOK) {
                                    if(appl_usbh_upgrade_i03t() != errOK) {
                                        appl_usbh_copy_logger(0);
                                    }   
                                }  
                            }
                            if(tid_thread_comm_i03t != NULL) {
                                osThreadResume(tid_thread_comm_i03t);
                            }
                            
                            if(tid_thread_4g_tx != NULL) {
                                osThreadResume(tid_thread_4g_tx);
                            }
                            
                            //HAL_NVIC_SystemReset();
                            break;
                            
                        case StorageSaveLogger:{
                                logger_msg_t *msg = (logger_msg_t *)storage_msg->pdata;
                                module_logger_store(*msg);
                                FileIndexUpdata();
                            }
                            break;

                        case StorageUDiskUpgrade:
                            pDEApp->Flag.bits.usb_busy = 1;
                            appl_usbh_upgrade();
                            break;
                        
                        case StorageUDiskExportAll:
                            pDEApp->Flag.bits.usb_busy = 1;
                            appl_usbh_copy_logger(0);
                            break;
                        case StorageUDiskExportHist:
                            pDEApp->Flag.bits.usb_busy = 1;
                            appl_usbh_copy_logger(1);
                            break;
                        case StorageUDiskExportAlarm:
                            pDEApp->Flag.bits.usb_busy = 1;
                            appl_usbh_copy_logger(2);
                            break;
                        case StorageUDiskExportDischarge:
                            pDEApp->Flag.bits.usb_busy = 1;
                            appl_usbh_copy_logger(3);
                            break;
                        case StorageUDiskExportSN:
                            pDEApp->Flag.bits.usb_busy = 1;
                            appl_usbh_copy_logger(4);
                            break;
                        
                        case StorageFileTest:
                            //module_file_create_test("test.txt",NULL,0,25*1024*1024);
                            break;
                        
                        default:
                            break;
                    }
                }
                
                if(pDEApp->Flag.bits.usb_busy) {
                    pDEApp->Flag.bits.usb_busy = 0;   
                    Bsp_Beeps(3);
                }

                pDEApp->Flag.bits.query_logger_flag = 0;
                
                if(storage_msg->pdata != NULL) {
                    sys_free(storage_msg->pdata);
                }
                sys_free(storage_msg);
                
            }
            status = osMutexRelease(mutexID_One);
        }
        
    }
}


