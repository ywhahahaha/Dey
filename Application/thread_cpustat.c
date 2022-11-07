#include "main.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "typedef.h"
#include "de_protocol_frame.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_thread_cpustat;                        // thread id
 
void thread_cpustat_process (void *argument);                   // thread function
 


errStatus_t thread_cpustat_init (void) {

    const osThreadAttr_t attr_thread = {
        .name = "cpu_stat",
        .stack_size = 512,
        .priority = osPriorityNormal, 
    };
     
    tid_thread_cpustat = osThreadNew(thread_cpustat_process, NULL, &attr_thread);
    if (tid_thread_cpustat == NULL) {
        return (errErr);
    }

    return (errOK);
}

/* 统计任务使用 */
__IO uint8_t   OSStatRdy;        /* 统计任务就绪标志 */
__IO uint32_t  OSIdleCtr;        /* 空闲任务计数 */
__IO int32_t   OSCPUUsage;       /* CPU百分比 */
uint32_t       OSIdleCtrMax;     /* 1秒内最大的空闲计数 */
uint32_t       OSIdleCtrRun;     /* 1秒内空闲任务当前计数 */


#define        CPUStatPerid     1000

void vApplicationDaemonTaskStartupHook( void ) {
    osDelay(CPUStatPerid+100);
}

void  cpustat_init(void)
{
	OSStatRdy = false;
	
    osDelay(2u);     //

    //__disable_irq();
    OSIdleCtr    = 0uL;
	//__enable_irq();
    osDelay(CPUStatPerid); 
   	//__disable_irq();
    OSIdleCtrMax = OSIdleCtr;
    OSStatRdy    = true;
	//__enable_irq();
}


void thread_cpustat_process (void *argument) {

    while (OSStatRdy == false) {
        osDelay(CPUStatPerid+100);     /* 等待统计任务就绪 */
    }

    OSIdleCtrMax /= 100uL;
    if (OSIdleCtrMax == 0uL) {
        OSCPUUsage = 0u;
    }
	
    //__disable_irq();
    OSIdleCtr = OSIdleCtrMax * 100uL;  /* 设置初始CPU利用率 0% */
	//__enable_irq();
    
    int32_t usage = 0;

    for (;;) {
        //__disable_irq();
        OSIdleCtrRun = OSIdleCtr;    /* 获得100ms内空闲计数 */
       	//__enable_irq();            /* 计算100ms内的CPU利用率 */


        usage   = (int32_t)(100uL - OSIdleCtrRun / OSIdleCtrMax);
        OSCPUUsage = usage < 0 ? 1 : usage;
        //OSCPUUsage = usage;
        OSIdleCtr    = 0uL;          /* 复位空闲计数 */

        osDelay(CPUStatPerid);       /* 每100ms统计一次 */
    }
}

int cpustat_get(void) {
    return OSCPUUsage;
}

void cpustat_print(void) {
    debug_printf("cpu usage:%d%%\r\n",OSCPUUsage);
}

