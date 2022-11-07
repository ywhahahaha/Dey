#include "main.h"
#include "typedef.h"
#include "cmsis_os2.h"                                                              // CMSIS RTOS header file
#include "bsp_gpio.h"
osMessageQueueId_t KeyMsgId = NULL;
extern uint8_t  Exist_t[CONFIG_MAX_I03T];
#define KEY_MSG_OBJECTS   100 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

void thread_key_process (void *argument);                                           // thread function
osThreadId_t tid_thread_key_tx; 
NvbNature_t     NvbNature = {0};
errStatus_t thread_key_init (void) { 
       
//    if(!pDEApp->device_config.i03m.mqtt_config.cloud_enable) {
//        return errErr; 
//    }
    
    const osThreadAttr_t attr_thread = {
        .name = "thread_key",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_key_tx = osThreadNew(thread_key_process, NULL, &attr_thread);
    if (tid_thread_key_tx == NULL) {
        return (errErr);
    }

    return (errOK);
}

uint8_t UpFindTNumber (uint8_t CurTnumber) {
    for (;CurTnumber < CONFIG_MAX_I03T; CurTnumber++) {
        if (Exist_t[CurTnumber]) {
            break;
        }
    }
    if (CurTnumber >= CONFIG_MAX_I03T) {
        for (uint8_t i = 0; i < CONFIG_MAX_I03T; i++ ) {
            if (Exist_t[i]) {
                CurTnumber = i;
                break;
            }            
        }
        if (CurTnumber >= CONFIG_MAX_I03T) {
            CurTnumber = 0;
        }
    }
    return CurTnumber;
}

int8_t DownFindTNumber (int8_t CurTnumber) {
    int8_t i,Tnumber;
    
    Tnumber = CurTnumber;
    if (CurTnumber < 0) {
        for (i = CONFIG_MAX_I03T - 1; i >=0; i--) {
            if (Exist_t[i]) {
                CurTnumber = i;
                break;
            }
        }
        if (i < 0) {
            CurTnumber = 0;
        }
    } else {
        for (; CurTnumber >= 0; CurTnumber--) {
            if (Exist_t[CurTnumber]) {
                break;
            }
        }
        if (CurTnumber < 0) {
            for (CurTnumber = CONFIG_MAX_I03T - 1; CurTnumber >=0; CurTnumber--) {
                if (Exist_t[CurTnumber]) {
                    break;
                }
            }
        if (CurTnumber < 0) {
            CurTnumber = 0;
        }            
        }
    }
    return CurTnumber;
}

uint8_t MinTNumber (void) {
    uint8_t i;
    for ( i = 0; i < CONFIG_MAX_I03T; i++) {
        if (Exist_t[i]) { 
            break;
        }
    }
    if (i >= CONFIG_MAX_I03T) {
        i = 0;
    }
    return i;
}


void thread_key_process (void *argument) {
    osStatus_t Qstatus;
    uint16_t msg = 1;
    static uint16_t ToalNum = 0;
    uint16_t    temp;
    Bsp_KeyInit();
    KeyMsgId = osMessageQueueNew(KEY_MSG_OBJECTS,sizeof(void *),NULL);   
    
    while (1) {
        if (!KeyUp() || !KeyDown() || !KeyLeft() || !KeyRight() || !KeyEnsur() || !KeyBack()) {
            osDelay(10);           
        }
        if (!KeyUp() || !KeyDown() || !KeyLeft() || !KeyRight() || !KeyEnsur() || !KeyBack()) {
            FindI03T();
            if(!KeyUp()) {
               if (NvbNature.NvbFlag) {
                    
               } else {
                    switch (NvbNature.CurCusor) {
                        case 0:    ;break;
                        case 1:
                                NvbNature.TPageNature.Tindex = UpFindTNumber(NvbNature.TPageNature.Tindex + 1);
                                break;
                        case 2: 
                                NvbNature.CellPageNature.Tindex = UpFindTNumber(NvbNature.CellPageNature.Tindex + 1);
                                break;
                        case 3: 
                                NvbNature.AlarmPageNature.Tindex = UpFindTNumber(NvbNature.AlarmPageNature.Tindex + 1);
                                break;
                    }
               }
            }
            if(!KeyDown()) {
               if (NvbNature.NvbFlag) {
                    
               } else {
                    switch (NvbNature.CurCusor) {
                        case 0:    ;break;
                        case 1:
                                NvbNature.TPageNature.Tindex = DownFindTNumber((int8_t)NvbNature.TPageNature.Tindex - 1);
                                break;
                        case 2: 
                                NvbNature.CellPageNature.Tindex = DownFindTNumber((int8_t)NvbNature.CellPageNature.Tindex - 1);
                                break;
                        case 3: 
                                NvbNature.AlarmPageNature.Tindex = DownFindTNumber((int8_t)NvbNature.AlarmPageNature.Tindex - 1);
                                break;
                    }
               }                
            } 
            if(!KeyLeft()) {
                if (NvbNature.NvbFlag) {
                      if (NvbNature.CurCusor == 0) {
                           NvbNature.CurCusor = 4; 
                      } else {
                        NvbNature.CurCusor -= 1;
                      }                 
                } else {
                        switch (NvbNature.CurCusor) {
                            case 0:     ;break;
                            case 1:     ;break;
                            case 2: if (!NvbNature.CellPageNature.CellPageNum) {
                                        NvbNature.CellPageNature.CellPageNum = 0;
                                        } else {
                                            NvbNature.CellPageNature.CellPageNum--;
                                        }                                      
                                    break;
                            case 3: if (!NvbNature.AlarmPageNature.AlarmPageNum) {
                                        NvbNature.AlarmPageNature.AlarmPageNum = 0;                
                                    } else {
                                        NvbNature.AlarmPageNature.AlarmPageNum--;
                                    }                                   
                                    break;
                        }
                    }                                
            } 
            if(!KeyRight()) {
                if (NvbNature.NvbFlag) {
                      if (NvbNature.CurCusor == 4) {
                           NvbNature.CurCusor = 0; 
                      } else {
                        NvbNature.CurCusor += 1;
                      }                 
                } else {
                        switch (NvbNature.CurCusor) {
                            case 0:     ;break;
                            case 1:     ;break;
                            case 2:
                                    NvbNature.CellPageNature.CellPageNum++;
                                    break;
                            case 3: 
                                    NvbNature.AlarmPageNature.AlarmPageNum++;
                                    break;
                        }                    
                    }                               
            } 
            if(!KeyEnsur()) {
                if (NvbNature.NvbFlag) {
                    NvbNature.NvbFlag = 0;
                    if (NvbNature.CurCusor == 4) {
                        NvbNature.NvbFlag = 1;
                    }
                } else {
                    switch (NvbNature.CurCusor) {
                        case 0:   
                                 NvbNature.CurCusor = 3;
                                 NvbNature.AlarmPageNature.Tindex = MinTNumber();
                                 break;
                        case 1: 
                                NvbNature.CurCusor = 2;
                                NvbNature.OldTNumber = NvbNature.TPageNature.Tindex;
                                NvbNature.CellPageNature.Tindex = NvbNature.TPageNature.Tindex;
                                break;
                    }
                } 
            } 
            if(!KeyBack()) {
                if (NvbNature.NvbFlag) {
                    NvbNature.NvbFlag = 0;
                    NvbNature.CurCusor = 0;                    
                } else {
                    NvbNature.NvbFlag = 1;
                }
            }
            temp = NvbNature.CurCusor + NvbNature.NvbFlag + 
                NvbNature.AlarmPageNature.AlarmPageNum + NvbNature.AlarmPageNature.Tindex + 
                NvbNature.CellPageNature.Tindex + NvbNature.CellPageNature.CellPageNum + NvbNature.TPageNature.Tindex;
            if (NvbNature.NvbFlag) {
                NvbNature.AlarmPageNature.AlarmPageNum = 0;
                NvbNature.AlarmPageNature.Tindex = MinTNumber();
                NvbNature.CellPageNature.CellPageNum = 0;
                NvbNature.CellPageNature.Tindex = MinTNumber();
                NvbNature.TPageNature.Tindex = MinTNumber();                
            }
            if (ToalNum != temp) {
                    Qstatus = osMessageQueuePut(KeyMsgId,&msg,osPriorityHigh,0);
                    ToalNum = temp;
                }          
            while (1) {
               if (KeyUp() && KeyDown() && KeyLeft() && KeyRight() && KeyEnsur() && KeyBack()) {
                    break;
               }
               osDelay(10);
            }
        }       
        osDelay(10);
    }
    
}

