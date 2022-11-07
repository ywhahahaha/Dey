#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "cmsis_os2.h"
#include "stm32f4xx.h"

#include "typedef.h"
#include "thread_debug.h"
#include "sys_mem.h"
#include "module_file_storage.h"
#include "module_sn.h"
#include "project_config.h"

#if OS_FREERTOS == OS_TYPE
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#endif

#define DEBUG_UART  UART4

DebugMsg_t comm_debug_msg = {0};

osSemaphoreId_t comTxSem = NULL;
osSemaphoreId_t comPrintSem = NULL;
void app_debug_thread (void *argument);

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length);

const char *result_ok = "OK\r\n";
const char *result_err = "ERROR\r\n";

void debug_sendstring(char * msg) {

	while(*msg != 0x00) {
		while((DEBUG_UART->SR & 0X40) == 0) {}
        DEBUG_UART->DR = *msg;
		msg++;
	}
}

void Bsp_Rs485SendBytes(uint8_t *pdata,uint16_t length);

void debug_sendbytes(uint8_t *pdata,uint16_t bytes) {
    osSemaphoreAcquire(comTxSem,osWaitForever);
    while(bytes--) {
        while((DEBUG_UART->SR & 0X40) == 0);
        DEBUG_UART->DR = *pdata++;
    }
    osSemaphoreRelease(comTxSem);
}



static char LoggerBuf[1280];
void Bsp_Rs485Uplink1SendBytes(uint8_t *pdata,uint16_t length);
void debug_printf(const char *fmt, ...) {
#if 1
    if(!pDEApp->Flag.bits.print_flag) {
        return;
    }
    osSemaphoreAcquire(comPrintSem,osWaitForever);

    uint16_t length;
    va_list args;
    va_start(args, fmt);
    memset(LoggerBuf, 0, sizeof(LoggerBuf));
    length = vsnprintf(LoggerBuf, sizeof(LoggerBuf) - 1, fmt, args);
    if(length > 0) {
        debug_sendbytes((uint8_t *)LoggerBuf, length);
    }
    va_end(args);

    
    osSemaphoreRelease(comPrintSem);
#endif
}

errStatus_t thread_debug_init(void) {

    MX_USART4_UART_Init(115200,UART_PARITY_NONE);
    comTxSem = osSemaphoreNew(1,1,NULL);
    comPrintSem = osSemaphoreNew(1,1,NULL);
    
    /* create usb to comm thread */
    const osThreadAttr_t attr_debug_thread = {
        .name = "debug_thread",
        . stack_size = 2048,
        .priority = osPriorityNormal, 
    };
    osThreadNew(app_debug_thread,NULL,&attr_debug_thread);
    
    pDEApp->Flag.bits.print_flag = 1;
    
    return errOK;
}


typedef struct {
    const char *command;
    char *(*pFun)(const void *,void *);

} DEBUG_COMMAND_t;


char *do_about(const void *command,void *para) {
    debug_printf("Compile @ %s %s\r\n",COMPILE_DATE,COMPILE_TIME);
    debug_printf("I03M Soft V%d.%d.%d \r\n",I03M_SOFT_VERSION_X,I03M_SOFT_VERSION_Y,I03M_SOFT_VERSION_Z);
    debug_printf("I03M Hard V%d.%d.%d \r\n",I03M_HARD_VERSION_X,I03M_HARD_VERSION_Y,I03M_HARD_VERSION_Z);  
    debug_printf("I03M Addr:%d\r\n",pDEApp->device_config.i03m.i03m_addr);
    debug_printf("I03T Num:%d,cells:%d\r\n",pDEApp->device_config.i03m.i03t_number,pDEApp->device_config.i03m.cell_numbers);
    DATE_yymmddhhmmss_t now;
    Bsp_RtcGetTime(&now);
    debug_printf("Now @%04d-%02d-%02d %02d:%02d:%02d\r\n",2000 + now.bits.year,\
                                                        now.bits.month,\
                                                        now.bits.day,\
                                                        now.bits.hour,\
                                                        now.bits.min,\
                                                        now.bits.sec);
    debug_printf("mqtt enable:%d,device:%d,user:%s,pass:%s,ip:%s,port:%d\r\n",pDEApp->device_config.i03m.mqtt_config.cloud_enable,\
                                                                          pDEApp->device_config.i03m.module_type,\
                                                                          pDEApp->device_config.i03m.mqtt_config.user_name,\
                                                                          pDEApp->device_config.i03m.mqtt_config.pass_word,\
                                                                          pDEApp->device_config.i03m.mqtt_config.mqtt_addr,\
                                                                          pDEApp->device_config.i03m.mqtt_config.mqtt_port);
    debug_printf("synch i03t time:%d\r\n",pDEApp->device_config.i03m.i03t_time_synch);
    
    
    debug_printf("Magic:%x\r\n",appl_noinit.magic);
    debug_printf("File test cnt:%d\r\n",appl_noinit.file_test_err_cnt);
    debug_printf("Usb Init err cnt:%d\r\n",appl_noinit.usb_init_err_cnt);
    debug_printf("Reset cnt:%d\r\n",appl_noinit.reset_cnt);
    
    return NULL;
}

char *do_get_task_infor(const void *command,void *para) {
#if 1
    uint8_t thread_count = osThreadGetCount();
    osThreadId_t *threads = (osThreadId_t *)sys_malloc(thread_count * sizeof(osThreadId_t));
    osThreadEnumerate(threads,thread_count);
    
    debug_printf("\r\ntask name\t\ttask id\t\ttask prio\tmin remain\r\n");
    for(uint8_t i=0;i<thread_count;i++) {

        debug_printf("%-24s%08x\t%04d\t\t%04d\t\r\n",(char *)osThreadGetName(threads[i]),\
                                                     (uint32_t)threads[i],\
                                                      osThreadGetPriority(threads[i]),\
                                                      osThreadGetStackSpace(threads[i]));   


    }
    sys_free(threads);
#else
    void vTaskList( char * pcWriteBuffer );
    void vTaskGetRunTimeStats( char * pcWriteBuffer );
    
	uint8_t *task_infor = sys_malloc(1024);
    
    if(task_infor == NULL) {
        return NULL; 
    }
    //debug_printf("\r\ntask name\t\ttask id\t\ttask prio\tmin remain\r\n");
	debug_printf("任务名称\t运行状态\t优先级\t剩余堆栈\t任务序号\r\n" );

	vTaskList((char *)task_infor);
	debug_sendbytes((uint8_t  *)task_infor, strlen((const char *)task_infor)); 
    
    memset(task_infor,0,1024);
    vTaskGetRunTimeStats((char *)task_infor);
    debug_sendbytes((uint8_t  *)task_infor, strlen((const char *)task_infor)); 
    
    sys_free(task_infor);
#endif
    return NULL;
}


char *do_get_mem_infor(const void *command,void *para) {
#if OS_TYPE == OS_FREERTOS
    debug_printf("memory infor:\r\ntotal:%d,left:%d,min_left:%d\r\n",configTOTAL_HEAP_SIZE,xPortGetFreeHeapSize(),xPortGetMinimumEverFreeHeapSize());
#else
    uint32_t total,used,max_used;
    memory_info(&total,&used,&max_used);
    debug_printf("memory infor:\r\ntotal:%d,left:%d,min_left:%d\r\n",total,total-used,total-max_used);
#endif
    return NULL;
}

char *do_get_file_infor(const void *command,void *para) {
    module_file_dir();
    return NULL;
}

char *do_format(const void *command,void *para) {

    storage_msg_put(0,NULL,0,StorageFsFormat);
    
    return NULL;
}

char *do_file_test(const void *command,void *para) {
    
    storage_msg_put(0,NULL,0,StorageFileTest);
    
    return NULL;
}



char *do_list_sn(const void *command,void *para) {
    module_sn_get_count(0,true);
    return NULL;
}

char *do_generate(const void *command,void *para) {
    for(uint16_t i=0;i<1000;i++) {
        for(uint8_t i03t_addr = 1;i03t_addr <=6; i03t_addr++) {
            storage_msg_put(i03t_addr,NULL,0,StorageHistData);
            osDelay(20);
            storage_msg_put(i03t_addr,NULL,0,StorageAlarmData);
            osDelay(20);
            storage_msg_put(i03t_addr,NULL,0,StorageDischarge);
            osDelay(20);
        }

    }

    return NULL;
}

char *do_list_alarm(const void *command,void *para) {
    LoggerQuery_t logger = {0};
    logger.start.date = 0;
    logger.end.date = 0;
    logger.commType = COMM_TYPE_COM;
    logger.print = 1;
    uint8_t i03t_addr = 1;
    if(strstr(command,"AT+LISTALARM1")) {
        i03t_addr = 1;
    }else if(strstr(command,"AT+LISTALARM2")) {
        i03t_addr = 2;
    }else if(strstr(command,"AT+LISTALARM3")) {
        i03t_addr = 3;
    }else if(strstr(command,"AT+LISTALARM4")) {
        i03t_addr = 4;
    }else if(strstr(command,"AT+LISTALARM5")) {
        i03t_addr = 5;
    }else if(strstr(command,"AT+LISTALARM6")) {
        i03t_addr = 6;
    }
    
    storage_msg_put(i03t_addr,(uint8_t *)&logger,sizeof(LoggerQuery_t),StorageAlarmRead);
    return NULL;
}

char *do_list_discharge(const void *command,void *para) {
    LoggerQuery_t logger = {0};
    logger.start.date = 0;
    logger.end.date = 0;
    logger.commType = COMM_TYPE_COM;
    logger.print = 1;
    uint8_t i03t_addr = 1;
    if(strstr(command,"AT+LISTDISCHARGE1")) {
        i03t_addr = 1;
    }else if(strstr(command,"AT+LISTDISCHARGE2")) {
        i03t_addr = 2;
    }else if(strstr(command,"AT+LISTDISCHARGE3")) {
        i03t_addr = 3;
    }else if(strstr(command,"AT+LISTDISCHARGE4")) {
        i03t_addr = 4;
    }else if(strstr(command,"AT+LISTDISCHARGE5")) {
        i03t_addr = 5;
    }else if(strstr(command,"AT+LISTDISCHARGE6")) {
        i03t_addr = 6;
    }
    
    storage_msg_put(i03t_addr,(uint8_t *)&logger,sizeof(LoggerQuery_t),StorageDischargeRead);
    return NULL;
}

char *do_add_sn(const void *command,void *para) {
    void module_sn_test(void);
    
    module_sn_test();
    
    return NULL;
}

char *do_list_real(const void *command,void *para) {

    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
        if(i03t_node != NULL) {
            debug_printf("\r\n------------------I03T@%d,soft:%d.%d.%d,hard:%d.%d.%d-----------------\r\n",i03t_node->i03t_addr,
                                                                VERSION_X(i03t_node->soft_version),
                                                                VERSION_Y(i03t_node->soft_version),
                                                                VERSION_Z(i03t_node->soft_version),
                                                                VERSION_X(i03t_node->hard_version),
                                                                VERSION_Y(i03t_node->hard_version),
                                                                VERSION_Z(i03t_node->hard_version));
            
            debug_printf("Alarm1:%04x(H),Alarm2:%04x(H),Alarm3:%04x(H),",i03t_node->alarm.alarm.bat_group_alarm1,\
                                                                         i03t_node->alarm.alarm.bat_group_alarm2,\
                                                                         i03t_node->alarm.alarm.bat_group_alarm3);
            
            debug_printf("Total Volt:%d(0.1V),Current:%d(0.1A),Status:%d,Temp:%d(0.1deg),SOC:%d,SOH:%d,cell cnt:%d\r\n",i03t_node->discharge.voltage,\
                                                                                i03t_node->discharge.current[0],\
                                                                                i03t_node->discharge.status,\
                                                                                i03t_node->discharge.temp,\
                                                                                i03t_node->discharge.soc,\
                                                                                i03t_node->discharge.soh,\
                                                                                pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.cell_number);
            
            debug_printf("Max volt:%d,id:%d,average:%d \r\n",i03t_node->cell_volt_peak.max,i03t_node->cell_volt_peak.max_id,i03t_node->cell_volt_peak.average);
            debug_printf("Min volt:%d,id:%d \r\n",i03t_node->cell_volt_peak.min,i03t_node->cell_volt_peak.min_id);
            debug_printf("Max Temp:%d,id:%d,average:%d \r\n",i03t_node->cell_temp_peak.max,i03t_node->cell_temp_peak.max_id,i03t_node->cell_temp_peak.average);
            debug_printf("Min Temp:%d,id:%d \r\n",i03t_node->cell_temp_peak.min,i03t_node->cell_temp_peak.min_id);
            
            for(uint16_t i = 0;i<CONFIG_MAX_CELL;i++) {
                debug_printf("Cell<%03d> Alarm:%04x(H),Volt:%05d,Res:%05d,Temp:%05d\r\n",i+1,\
                                                        i03t_node->alarm.alarm.cell_alarm[i],\
                                                        i03t_node->hist.cells[i].voltage,\
                                                        i03t_node->hist.cells[i].inter_res,\
                                                        i03t_node->hist.cells[i].temperature);
            }
        }
    }
    
    return NULL;
}

char *do_set_soc(const void *command,void *para) {

    uint8_t i03t_addr = 1;
    if(strstr(command,"AT+SOC=1,")) {
        i03t_addr = 1;
    }else if(strstr(command,"AT+SOC=2,")) {
        i03t_addr = 2;
    }else if(strstr(command,"AT+SOC=3,")) {
        i03t_addr = 3;
    }else if(strstr(command,"AT+SOC=4,")) {
        i03t_addr = 4;
    }else if(strstr(command,"AT+SOC=5,")) {
        i03t_addr = 5;
    }else if(strstr(command,"AT+SOC=6,")) {
        i03t_addr = 6;
    }
    
    char *index = strstr(command,",");
    if(index == NULL) {
        return NULL;
    }
    
    int soc = atoi(index + 1);
    
    I03T_Info_t *i03t = i03t_node_find(i03t_addr);
    if(i03t != NULL) {
        i03t->discharge.soc = soc;
    }
    
    debug_printf("OK\r\n");

    return NULL;
}
void logger_infor_load(COMM_TYPE_t commType,bool load);
char *do_get_logger(const void *command,void *para) { 
    if(strstr(command,"AT+LOGGER=G")) {
        //logger_infor_save_more(LOGGER_OPERATE,LOGGER_MORE_FILE_OPERATE_FAILED,0,__FUNCTION__,__FILE__,__LINE__);
    } else {
        logger_infor_load(COMM_TYPE_COM,false);
    }
    
    
    debug_printf("OK\r\n");
    
    return NULL;

}

char *do_set_print(const void *command,void *para) {
    if(strstr(command,"AT+PRINT=1") != NULL) {
        pDEApp->Flag.bits.print_flag = 1;
    } else {
        pDEApp->Flag.bits.print_flag = 0;
    }
    debug_printf(result_ok);
    return NULL;
}

char *do_soc_trace(const void *command,void *para) {
    if(strstr(command,"AT+SOCTRACE=1") != NULL) {
        pDEApp->Flag.bits.soc_trace = 1;
    } else {
        pDEApp->Flag.bits.soc_trace = 0;
    }
    debug_printf(result_ok);
    return NULL;
}

char *do_4g_trace(const void *command,void *para) {
    if(strstr(command,"AT+G4TRACE=1") != NULL) {
        pDEApp->Flag.bits.g4_trace = 1;
    } else {
        pDEApp->Flag.bits.g4_trace = 0;
    }
    debug_printf(result_ok);
    return NULL;
}

char *do_list_file_index(const void *command,void *para) {
    debug_printf("Index:%d\r\n",pDEApp->index.index);
    for(uint8_t i=0;i<CONFIG_MAX_I03T;i++) {
        debug_printf("I03T%d:hist:%d,alarm:%d,discharge:%d,charge:%d,soc:%d,soh:%d,deep_cycle:%.3f\r\n",i+1,\
                                                                            pDEApp->index.index_info[i].index_hist,
                                                                            pDEApp->index.index_info[i].index_alarm,
                                                                            pDEApp->index.index_info[i].index_discharge,
                                                                            pDEApp->index.index_info[i].index_charge,
                                                                            pDEApp->index.index_info[i].soc,
                                                                            pDEApp->index.index_info[i].soh,
                                                                            pDEApp->index.index_info[i].deep_discharge_cycle);
    }
    debug_printf("logger_index:%d\r\n",pDEApp->index.logger_index);
    return NULL;
}

void IAP_EraseAppBack(void);
char *do_clear_backapp(const void *command,void *para) {
    IAP_EraseAppBack();
    debug_printf("OK\r\n");
    return NULL;
}

char *do_file_flush(const void *command,void *para) {
    storage_msg_put(4,NULL,0,StorageFileFlush);
    return NULL;
}

char *do_reboot(const void *command,void *para) {
    command_msg_put(SysCmdReset,NULL,0);
    return NULL;
}

void cpustat_print(void);
char *do_cpu_handle(const void *command,void *para) {

    if(strstr(command,"AT+CPU=1") != NULL) {
        appl_noinit.cpu_stat = 0x5A;
        appl_noinit_store();
        command_msg_put(SysCmdReset,NULL,0);
        debug_printf("OK\r\n");
    } 
    if(strstr(command,"AT+CPU=0") != NULL) {
        appl_noinit.cpu_stat = 0;
        appl_noinit_store();
        command_msg_put(SysCmdReset,NULL,0);
        debug_printf("OK\r\n");
    } 
    if(strstr(command,"AT+CPU?") != NULL) {
        cpustat_print();
    } 

    return NULL;
}

const DEBUG_COMMAND_t commandList[] = {
    {"help",            do_about},
    {"AT+GETTASKINFO",  do_get_task_infor},
    {"AT+GETMEMINFO",   do_get_mem_infor},
    {"AT+GETFILEINFO",  do_get_file_infor},
    {"AT+FSFORMAT",     do_format},
    {"AT+FILEFLUSH",    do_file_flush},
    {"AT+LISTSN",       do_list_sn},
    {"AT+GENERATE",     do_generate},
    {"AT+LISTALARM",    do_list_alarm},
    {"AT+LISTDISCHARGE",do_list_discharge},
    {"AT+LISTREAL",     do_list_real},
    {"AT+ADDSN",        do_add_sn},
    {"AT+SOC=",         do_set_soc},
    {"AT+LOGGER",       do_get_logger},
    {"AT+PRINT=",       do_set_print},
    {"AT+SOCTRACE=",    do_soc_trace},
    {"AT+LISTFILEINDEX",do_list_file_index},
    {"AT+CLRBACKAPP",   do_clear_backapp},
    {"AT+G4TRACE=",     do_4g_trace},
    {"AT+REBOOT",       do_reboot},
    {"AT+CPU",          do_cpu_handle},

    
};

#define MAX_COMMAND sizeof(commandList)/sizeof(commandList[0])
    
char *commandHandle(char *command,void *para) {
    for(uint8_t index = 0; index < MAX_COMMAND; index++) {
        char *pIndexCmdStr = strstr(command, commandList[index].command);

        if(pIndexCmdStr != NULL) {
            if(commandList[index].pFun != NULL) {
                char *response = commandList[index].pFun(command,para);
                return response;
            }
        }
    }
    
    return NULL;
}
bool de_protocol_process(COMM_TYPE_t type,uint8_t *pdata,uint16_t length);
void app_debug_thread (void *argument) {
    
    (void)argument;

    while(1) {
        
        osDelay(PROTOCOL_RX_PULL_TIME);

        if(comm_debug_msg.length > 3  && osKernelGetTickCount() > comm_debug_msg.tick) {

            bool result = de_protocol_process(COMM_TYPE_COM,comm_debug_msg.buffer,comm_debug_msg.length);
            if(!result) {
                commandHandle((char *)comm_debug_msg.buffer,NULL);
            }
            
            memset(&comm_debug_msg, 0, sizeof(comm_debug_msg));
        }  
        
    }
    
}
