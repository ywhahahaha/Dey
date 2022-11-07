#include "main.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/


void thread_file_process (void *argument);                   // thread function
osThreadId_t tid_thread_file_tx; 

errStatus_t thread_file_init (void) { 
       
//    if(!pDEApp->device_config.i03m.mqtt_config.cloud_enable) {
//        return errErr; 
//    }
    
 
    const osThreadAttr_t attr_thread = {
        .name = "thread_file",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_file_tx = osThreadNew(thread_file_process, NULL, &attr_thread);
    if (tid_thread_file_tx == NULL) {
        return (errErr);
    }

    return (errOK);
}

extern uint16_t mutex_lcd;
void thread_file_process (void *argument) {
    
    char path[24] = {0};
    int i = 2;
    osDelay(8000);
    while(1) {
    
    while (mutex_lcd!=0);
    mutex_lcd += 1;
    sprintf((char *)path,"t%d.txt",rand()%10000);
    if(!module_file_new((char *)path,(uint8_t *)path,strlen(path))) {
        module_file_del(path);
        debug_printf("creat file failed\r\n");
    } else {
         debug_printf("creat file succed\r\n");
         module_file_del(path);
    }
    mutex_lcd -= 1;
        osDelay(1000);
        
    }
}

