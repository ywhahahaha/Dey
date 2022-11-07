#include "main.h"
#include "typedef.h"
#include "cmsis_os2.h"                                                              // CMSIS RTOS header file
#include "bsp_lcd.h"
#include "delay.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

void thread_lcd_process (void *argument);                                           // thread function
osThreadId_t tid_thread_lcd_tx; 
extern NvbNature_t     NvbNature;
errStatus_t thread_lcd_init (void) { 
       
//    if(!pDEApp->device_config.i03m.mqtt_config.cloud_enable) {
//        return errErr; 
//    }
    
    const osThreadAttr_t attr_thread = {
        .name = "thread_lcd",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_lcd_tx = osThreadNew(thread_lcd_process, NULL, &attr_thread);
    if (tid_thread_lcd_tx == NULL) {
        return (errErr);
    }

    return (errOK);
}
extern osMutexId_t  mutexID_One;
extern osMessageQueueId_t KeyMsgId; 
uint16_t mutex_lcd = 0;


void SelectBox (uint8_t Cursor) {

    POINT_COLOR = DARKPINK;
    LCD_DrawRectangle(30,160,155,335);
    LCD_DrawRectangle(31,161,154,334);
    LCD_DrawRectangle(32,162,153,333);
    LCD_DrawRectangle(33,163,152,332);
    POINT_COLOR = BULEBRED;
    LCD_DrawRectangle(180,160,305,335);
    LCD_DrawRectangle(181,161,304,334);
    LCD_DrawRectangle(182,162,303,333);
    LCD_DrawRectangle(183,163,302,332);
    POINT_COLOR = DARKGREEN;
    LCD_DrawRectangle(330,160,455,335);
    LCD_DrawRectangle(331,161,454,334); 
    LCD_DrawRectangle(332,162,453,333);
    LCD_DrawRectangle(333,163,452,332);
    POINT_COLOR = DARKGREEN;
    LCD_DrawRectangle(480,160,605,335);
    LCD_DrawRectangle(481,161,604,334);
    LCD_DrawRectangle(482,162,603,333);
    LCD_DrawRectangle(483,163,602,332);    
    POINT_COLOR = DARKRED;
    LCD_DrawRectangle(630,160,755,335);
    LCD_DrawRectangle(631,161,754,334);
    LCD_DrawRectangle(632,162,753,333);
    LCD_DrawRectangle(633,163,752,332);
    POINT_COLOR = BLACK;
    switch (NvbNature.CurCusor) {
        case 0: 
                LCD_DrawRectangle(30,160,155,335);
                LCD_DrawRectangle(31,161,154,334);
                LCD_DrawRectangle(32,162,153,333);
                LCD_DrawRectangle(33,163,152,332);
                break;
        case 1: 
                LCD_DrawRectangle(180,160,305,335);
                LCD_DrawRectangle(181,161,304,334);
                LCD_DrawRectangle(182,162,303,333);
                LCD_DrawRectangle(183,163,302,332);
                break;
        case 2: 
                LCD_DrawRectangle(330,160,455,335);
                LCD_DrawRectangle(331,161,454,334);
                LCD_DrawRectangle(332,162,453,333);
                LCD_DrawRectangle(333,163,452,332);
                break;
        case 3: 
                LCD_DrawRectangle(480,160,605,335);
                LCD_DrawRectangle(481,161,604,334);
                LCD_DrawRectangle(482,162,603,333);
                LCD_DrawRectangle(483,163,602,332);
                break;
        case 4: 
                LCD_DrawRectangle(630,160,755,335);
                LCD_DrawRectangle(631,161,754,334);
                LCD_DrawRectangle(632,162,753,333);
                LCD_DrawRectangle(633,163,752,332);
                break;                    
   }    
}

void thread_lcd_process (void *argument) {
    uint16_t status;
    osStatus_t Qstatus;
    uint16_t msg;
    uint32_t  PressTime;  
    osMutexAcquire(mutexID_One,osWaitForever);
    LCD_Init();    
    status = osMutexRelease(mutexID_One);
    IniTNumber();
    AnalogueData();
    PressTime = osKernelGetTickCount();
    while(1) {
        
        if ((osKernelGetTickCount() - PressTime) > (120 * 1000ul)) {
            Qstatus = osMessageQueueGet(KeyMsgId,&msg,NULL,5000);
        } else {
            Qstatus = osMessageQueueGet(KeyMsgId,&msg,NULL,120 * 1000ul);
        }        
//        AnalogueData();
        if (Qstatus == osOK) {
            osMutexAcquire(mutexID_One,osWaitForever);
            LCD_CS = 0;
            PressTime = osKernelGetTickCount();
            if (NvbNature.NvbFlag) {
                if (NvbNature.NvbMutex == 0) {
                    ShowNvgBar();
                    NvbNature.NvbMutex = 1;
                }                
            SelectBox(NvbNature.CurCusor);    
            } else {
               NvbNature.NvbMutex = 0;
               switch (NvbNature.CurCusor) {
                   case 0:
                           MainMenu();
                           break;
                   case 1:
                           ShowT();
                           break;
                   case 2:
                           ShowCell();
                           break;
                   case 3:
                           ShowAlarm();
                           break;
                   case 4:
                           //≤Œ ˝…Ë÷√‘§¡Ù
                           break;                   
               }
                   
            }
            LCD_CS = 1;
            status = osMutexRelease(mutexID_One);
        } else {
            if (!NvbNature.NvbFlag) {
                osMutexAcquire(mutexID_One,osWaitForever);
                LCD_CS = 0;
                switch (NvbNature.CurCusor) {
                    case 0: 
                            MainMenuInfo();
                            break;
                    case 1: 
                            BaryInfo();
                            break;  
                    case 2: 
                            CellInfo();
                            break;  
                    case 3: 
                            AlarmInfo();
                            break;                      
                }
                LCD_CS = 1;
                status = osMutexRelease(mutexID_One);
            }
        } 
    }
}

