#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "main.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
osMessageQueueId_t commandMsgId = NULL;
#define COMMAND_MSG_OBJECTS    8
void thread_command_process (void *argument);                   // thread function
 
 
void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length) {
    if(commandMsgId == NULL) return;
    
    SysMsgCmd_t *msg = sys_malloc(sizeof(SysMsgCmd_t));
    if(msg != NULL) {
        
        memset(msg,0,sizeof(SysMsgCmd_t));
       
        msg->cmd = command;
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
        status = osMessageQueuePut(commandMsgId,&msg,osPriorityNormal,0);
        if(status != osOK) {
            if(msg->pdata != NULL) {
                sys_free(msg->pdata);
            }
            
            sys_free(msg);
        }
    }
}
errStatus_t thread_command_init (void) {
     const osThreadAttr_t attr_thread = {
        .name = "thread_command",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    osThreadNew(thread_command_process, NULL, &attr_thread);
    return (errOK);
}
//running    100 
//comm err   500


//err
//always on  err.
//toggle synch err.
void appl_hardware_ext_usart_init(void);
void thread_command_process (void *argument) {
    
    commandMsgId = osMessageQueueNew(COMMAND_MSG_OBJECTS,sizeof(void *),NULL);

    uint8_t running_led_cnt = 0;
    uint8_t alarm_led_cnt = 0;
    uint8_t commu_led_cnt = 0;
    
    uint8_t running_delay = 1;
    uint8_t alarm_delay = 1;
    uint8_t commu_delay = 5;

    for(uint8_t i=0;i<6;i++) {
        Bsp_LedRunningToggle();
        Bsp_LedErrorToggle();
        Bsp_LedCommuToggle();
        osDelay(500);

    }
    Bsp_LedErrorOff();
    Bsp_LedRunningOff();
    Bsp_LedCommuOff();
    while (1) {
        uint32_t msg;
        osStatus_t status;
       
        status = osMessageQueueGet(commandMsgId,&msg,NULL,100);
        if(status == osOK) {
            SysMsgCmd_t *command = (SysMsgCmd_t *)msg;
            if(command != NULL) {
                switch(command->cmd) {
                    case SysCmdReset:{
                            Bsp_LedErrorOff();
                            Bsp_LedRunningOff();
                            osDelay(4000);
                            HAL_NVIC_SystemReset();
                        }
                        break;
                    case SysCmdSetBaudRate:
                        osDelay(500);
                        break;
                    default:
                        break;
                }
                if(command->pdata != NULL) {
                    sys_free(command->pdata);
                }
                sys_free(command);
            }
        }
        
        /* usbh busy */
        if(pDEApp->Flag.bits.usb_busy) {
            running_delay = 1;
        } else if(pDEApp->Flag.bits.synch_err) {
            running_delay = 5;
        } else {
            running_delay = 10;
        }
        running_led_cnt++;
        if(running_led_cnt >= running_delay) {
            Bsp_LedRunningToggle();
            running_led_cnt = 0;
        }

        if(!pDEApp->device_config.i03m.i03t_number) {
            Bsp_LedErrorOff();
        } else {
            if(pDEApp->Flag.bits.alarm_group_flag && pDEApp->Flag.bits.alarm_cell_flag) {
                Bsp_LedErrorOn();
                alarm_led_cnt = 0;
            } else if(pDEApp->Flag.bits.alarm_group_flag || pDEApp->Flag.bits.alarm_cell_flag) {
                alarm_delay = 5;
                alarm_led_cnt++;
                if(alarm_led_cnt >= alarm_delay) {
                    Bsp_LedErrorToggle();
                    alarm_led_cnt = 0;
                }
            } else {
                alarm_led_cnt = 0;
                Bsp_LedErrorOff();
            }
        }
        
        
        if(pDEApp->Flag.bits.comm_err) {
            commu_led_cnt++;
            if(commu_led_cnt >= commu_delay) {
                commu_led_cnt = 0;
                Bsp_LedCommuToggle();
            }
            
        } else {
            Bsp_LedCommuOff();
        }
    }
}
