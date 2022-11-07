#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "strTools.h"
#include "sys_mem.h"
#include "main.h"
#include "module_4g.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
 char *commandHandle(char *command,void *para); 
bool de_protocol_process(COMM_TYPE_t type,uint8_t *pdate,uint16_t length); 
void modbus_handler(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length);
errStatus_t protocol_msg_put(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length);
void Module4G_RxMsgHandle(LinkerMsg_t *msg);

osThreadId_t tid_thread_485_3;                        // thread id
void thread_485_3_process (void *argument);                   // thread function

errStatus_t thread_rs485_3_init (void) {

    const osThreadAttr_t attr_thread = {
        .name = "thread_485_3",
        .stack_size = 1024,
        .priority = osPriorityNormal1, 
    };
     
    tid_thread_485_3 = osThreadNew(thread_485_3_process, NULL, &attr_thread);
    if (tid_thread_485_3 == NULL) {
        return (errErr);
    }

    return (errOK);
}


void Module4G_Handle(uint8_t *pdata,uint16_t length);
void thread_485_3_process (void *argument) {
    
    while (1) {
        
        osDelay(PROTOCOL_RX_PULL_TIME_1);

        if(RxMsg485_3.length > 3 && osKernelGetTickCount() > RxMsg485_3.tick) {
            
            switch(pDEApp->device_config.i03m.rs485_cfg[2].port_type) {
                case PORT_ETH:
                case PORT_4G:
                    Module4G_Handle(RxMsg485_3.buffer,RxMsg485_3.length);
                    break;
                default:
                   protocol_msg_put(COMM_TYPE_485_3,RxMsg485_3.buffer,RxMsg485_3.length);
                    break;
            }
            
            memset(&RxMsg485_3,0,sizeof(ProtocolMsg_t));
        }
    }
}
