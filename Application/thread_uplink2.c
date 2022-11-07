#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "strTools.h"
#include "sys_mem.h"
#include "main.h"
#include "module_4g.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_thread_uplink2;                        // thread id
 
void thread_uplink2_process (void *argument);                   // thread function
#define MSG_AT_RESP    8          
osMessageQueueId_t mid_MsgAtResp = NULL;
void Module4G_RxMsgHandle(LinkerMsg_t *msg);

errStatus_t thread_uplink2_init (void) {

    if(!pDEApp->device_config.i03m.mqtt_config.cloud_enable) {
        return errErr; 
    }
        
    mid_MsgAtResp = osMessageQueueNew(MSG_AT_RESP,sizeof(void *),NULL);
    
    const osThreadAttr_t attr_thread = {
        .name = "thread_uplink2",
        .stack_size = 2048,
        .priority = osPriorityNormal1, 
    };
     
    tid_thread_uplink2 = osThreadNew(thread_uplink2_process, NULL, &attr_thread);
    if (tid_thread_uplink2 == NULL) {
        return (errErr);
    }

    return (errOK);
}

char *commandHandle(char *command,void *para); 
bool de_protocol_process(COMM_TYPE_t type,uint8_t *pdate,uint16_t length); 
void modbus_handler(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length);
errStatus_t protocol_msg_put(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length);
void thread_uplink2_process (void *argument) {
    
    while (1) {
        
        osDelay(PROTOCOL_RX_PULL_TIME);

        if(protocolRxMsgUplink2.length > 3 && osKernelGetTickCount() > protocolRxMsgUplink2.tick) {
            
            LinkerMsg_t *msg = sys_malloc(sizeof(LinkerMsg_t));

			if(msg != NULL) {
				msg->length = protocolRxMsgUplink2.length;
				msg->pdata = sys_malloc(msg->length + 1);

				if(msg->pdata != NULL) {
					memset(msg->pdata,0,msg->length + 1);
					memcpy(msg->pdata,protocolRxMsgUplink2.buffer,msg->length);
                    memset(&protocolRxMsgUplink2,0,sizeof(protocolRxMsgUplink2));
				} else {
                    memset(&protocolRxMsgUplink2,0,sizeof(protocolRxMsgUplink2));
                    sys_free(msg);
                    msg = NULL;
                    continue;
                }
			} else {
				memset(&protocolRxMsgUplink2,0,sizeof(protocolRxMsgUplink2));
				continue;
			}

            if(pDEApp->Flag.bits.g4_trace) {
                debug_printf("%s",msg->pdata);
            }
            
            Module4G_RxMsgHandle(msg);
            
            if(pModule4GInfor->pending) {
                osStatus_t status = osMessageQueuePut(mid_MsgAtResp,&msg,osPriorityNormal,0);
                if(status != osOK) {
                    sys_free(msg->pdata);
                    sys_free(msg);
                }
            } else {
                sys_free(msg->pdata);
                sys_free(msg);
            }
        }
    }
}
