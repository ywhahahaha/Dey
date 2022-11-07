#include "main.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "de_protocol_frame.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_thread_uplink1;                        // thread id
 
void thread_uplink1_process (void *argument);                   // thread function
 


errStatus_t thread_uplink1_init (void) {

    const osThreadAttr_t attr_thread = {
        .name = "thread_uplink1",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_uplink1 = osThreadNew(thread_uplink1_process, NULL, &attr_thread);
    if (tid_thread_uplink1 == NULL) {
        return (errErr);
    }

    return (errOK);
}

errStatus_t protocol_msg_put(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length);
de_protocol_t *de_protocol_check(uint8_t *p,uint16_t length);
bool modbus_crc16_check_conti_write(uint8_t *pdata,uint16_t length);
bool modbus_crc16_check(uint8_t *pdata,uint16_t length);
void thread_uplink1_process (void *argument) {

    uint8_t cnt = 0;
    
    uint8_t protocol_access = 0;
    
    while(1) {

        osDelay(PROTOCOL_RX_PULL_TIME_1); 
        
        if(protocolRxMsgUplink1.length > 3 && osKernelGetTickCount() > protocolRxMsgUplink1.tick) {
            protocol_access = 0;
            de_protocol_t *pMsg = de_protocol_check(protocolRxMsgUplink1.buffer, protocolRxMsgUplink1.length);
            if(pMsg == NULL) {
                if(protocolRxMsgUplink1.length > 10) {
                    if(modbus_crc16_check_conti_write(protocolRxMsgUplink1.buffer,protocolRxMsgUplink1.length)) {
                        protocol_access = 1;
                    }
                } else {
                    if(modbus_crc16_check(protocolRxMsgUplink1.buffer,protocolRxMsgUplink1.length)) {
                        protocol_access = 1;
                    }
                } 
                
                if(!protocol_access) {
                    cnt++;
                    if(cnt > 10) {
                        pMsg = de_protocol_check(protocolRxMsgUplink1.buffer, protocolRxMsgUplink1.length);
                        memset((void *)&protocolRxMsgUplink1,0,sizeof(protocolRxMsgUplink1)); 
                        cnt = 0;
                    }
                    continue;
                }
            } 
            
            cnt = 0;
            errStatus_t err = protocol_msg_put(COMM_TYPE_485_4,protocolRxMsgUplink1.buffer,protocolRxMsgUplink1.length);
            if(err != errOK) {
 
            }

            memset((void *)&protocolRxMsgUplink1,0,sizeof(protocolRxMsgUplink1));
        }        
    }
}

