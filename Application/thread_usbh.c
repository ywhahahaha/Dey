#include <stdio.h>

#include "main.h"
#include "rl_fs.h"                      
#include "rl_usb.h"                    

#include "USBH_MSC.h"
#include "bsp_timer.h"
#include "bsp_gpio.h"


osThreadId_t tid_thread_usbh;                        // thread id
 
void thread_usbh_process (void *argument);                   // thread function
 
errStatus_t thread_usbh_init (void) {
     const osThreadAttr_t attr_thread = {
        .name = "thread_usbh",
        .stack_size = 2048,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_usbh = osThreadNew(thread_usbh_process, NULL, &attr_thread);
    if (tid_thread_usbh == NULL) {
        return (errErr);
    }

    return (errOK);
}


void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
void thread_usbh_process (void *argument) {
    usbStatus usb_status;                 
    int32_t   msc_status;                 

    (void)argument;
    
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 15, 0);
    
    osDelay(15000);

    for(uint8_t i=0;i<3;i++) {
        usb_status = USBH_Initialize (0U);    
        if (usb_status == usbOK) {
            break;
        }
        osDelay(1000);
        USBH_Uninitialize(0);
        __HAL_RCC_USB_OTG_FS_FORCE_RESET();
        __HAL_RCC_USB_OTG_FS_RELEASE_RESET();
        osDelay(1000);
    }

    if (usb_status != usbOK) {
        appl_noinit.usb_init_err_cnt++;
        appl_noinit_store();
        debug_printf("USBH_Initialize Failed.\r\n");                   
    }
    
    for (;;) {
        msc_status = USBH_MSC_DriveGetMediaStatus ("U0:"); 
        if (msc_status == USBH_MSC_OK) {
            if (pDEApp->Flag.bits.usb_con == 0U) {                    
                                  
                debug_printf("Found U Disk.\r\n");
                msc_status = USBH_MSC_DriveMount ("U0:");
                if (msc_status != USBH_MSC_OK) {
                    continue;  
                }
                pDEApp->Flag.bits.usb_con = 1U;    
                Bsp_Beeps(1);   
                debug_printf("Mount U Disk OK.\r\n");
                storage_msg_put(0,NULL,0,StorageUdisk); 
            }
        } else {
            if (pDEApp->Flag.bits.usb_con == 1U) {                  
                pDEApp->Flag.bits.usb_con = 0U;
            }
        }
        
        osDelay(1000U);
    }
}
