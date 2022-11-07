#include "main.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "de_modbus_protocol.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_thread_protocol_process;                        
osThreadId_t tid_thread_protocol_master_process;
 
void thread_protocol_process (void *argument);   
void thread_protocol_master_process (void *argument); 

errStatus_t thread_protocol_process_init (void) {
     const osThreadAttr_t attr_thread = {
        .name = "thread_proto",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_protocol_process = osThreadNew(thread_protocol_process, NULL, &attr_thread);
    if (tid_thread_protocol_process == NULL) {
        return (errErr);
    }

    return (errOK);
}

osMessageQueueId_t protocol_handle = NULL;

#define PROTOCOL_MSG_OBJECTS    36

errStatus_t protocol_msg_put(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length) {
    
    if(protocol_handle == NULL) return errErr;
    
    ApplProto_t *msg = sys_malloc(sizeof(ApplProto_t));
    
    if(msg != NULL) {
        
        memset(msg,0,sizeof(ApplProto_t));
       
        msg->commType = commType;
        if(length > 0 && pdata != NULL) {
            msg->pdata = sys_malloc(length);
            if(msg->pdata != NULL) {
                memcpy(msg->pdata,pdata,length);
                msg->length = length;
            } else {
                sys_free(msg);
                return errErr;
            }
        } 
        
        osStatus_t status;
        status = osMessageQueuePut(protocol_handle,&msg,osPriorityNormal,0);
        if(status != osOK) {
            if(msg->pdata != NULL) {
                sys_free(msg->pdata);
            }
            
            sys_free(msg);
            return errErr;
        }
    }
    
    return errOK;
}

bool de_protocol_process(COMM_TYPE_t type,uint8_t *pdata,uint16_t length);

void thread_protocol_process (void *argument) {

    protocol_handle = osMessageQueueNew(PROTOCOL_MSG_OBJECTS, sizeof(void *),NULL);
    
    while (1) {
        uint32_t msg;
        osStatus_t status;
       
        status = osMessageQueueGet(protocol_handle,&msg,NULL,osWaitForever);
        
        if(status == osOK) {
            ApplProto_t *pMsg = (ApplProto_t *)msg;
            if(pMsg != NULL && pMsg->pdata != NULL) {
                pDEApp->Flag.bits.sys_busy = 1;
                bool result = de_protocol_process(pMsg->commType,pMsg->pdata,pMsg->length);
                if(!result) {
                    modbus_handler(pMsg->commType,pMsg->pdata,pMsg->length);
                }
                pDEApp->Flag.bits.sys_busy = 0;
            }
            
            if(pMsg != NULL) {
                if(pMsg->pdata != NULL) {
                    sys_free(pMsg->pdata);
                }
                
                sys_free(pMsg);
            }
        } 
    }
}

