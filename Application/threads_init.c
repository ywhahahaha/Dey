#include "cmsis_os2.h"
#include "thread_debug.h"
#include "typedef.h"
#include "sys_mem.h"
#include "main.h"
#include "logger.h"



ProtocolMsg_t protocolRxMsg = {0};
ProtocolMsg_t RxMsg485_1 = {0};
ProtocolMsg_t RxMsg485_2 = {0};
ProtocolMsg_t RxMsg485_3 = {0};

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
errStatus_t thread_storage_init (void);
errStatus_t thread_comm_i03t_init (void);
errStatus_t thread_debug_init(void);
errStatus_t thread_command_init (void);
errStatus_t thread_rs485_1_init (void);
errStatus_t thread_rs485_2_init (void);
errStatus_t thread_rs485_3_init (void);
errStatus_t thread_soc_init (void);
errStatus_t thread_protocol_process_init(void);
errStatus_t thread_usbh_init (void);
errStatus_t thread_4g_init (void);
errStatus_t thread_file_init (void);

void appl_para_init(void);
void i03t_node_load(void);
void system_heap_init(void *begin_addr, void *end_addr);
void appl_hardware_init(void);
void appl_software_init(void);
void logger_infor_load(COMM_TYPE_t commType,bool load);
uint16_t module_sn_get_count(uint8_t,bool);
void appl_alarm_check_init(void);
char *do_about(const void *command,void *p);
char *do_noinit_info(const void *command,void *para);
void appl_hardware_ext_usart_init(void);
errStatus_t thread_cpustat_init (void);
void  cpustat_init(void);
void thread_init_process (void *argument) {

    HAL_ResumeTick();

    appl_noinit_load();
    
#if OS_TYPE == OS_RTX5
    system_heap_init(HEAP_BEGIN,HEAP_END);
#endif     
    
    if(appl_noinit.cpu_stat == 0x5A) {
        thread_cpustat_init();
        cpustat_init();
    }

    appl_software_init();
    
    thread_debug_init();
    
    appl_para_init();
    
    appl_hardware_init();

    logger_infor_load(COMM_TYPE_COM,true);

    i03t_node_load();

    thread_command_init();
    
    logger_infor_save_more(LOGGER_RESET,LOGGER_MORE_NONE,RCC->CSR,NULL,__FILE__,__LINE__);

    module_file_load(); 

    module_sn_get_count(0,false);

    thread_storage_init();

    thread_comm_i03t_init();  

    thread_rs485_1_init();
    
    thread_rs485_2_init();
    
    thread_rs485_3_init();
    
//    thread_4g_init();

    thread_soc_init();
    
    appl_alarm_check_init();
    
    thread_protocol_process_init();
    
//    thread_file_init();
      
    thread_key_init();
    
    thread_usbh_init();

    do_about(NULL,NULL);
    
    pDEApp->Flag.bits.sys_init_finish = 1;
    pDEApp->Flag.bits.print_flag = 0;
    pDEApp->Flag.bits.soc_trace = 0;
    thread_lcd_init();
    osThreadTerminate(osThreadGetId());
}
