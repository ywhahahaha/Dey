#ifndef __APPL_COMM_I03T_H_
#define __APPL_COMM_I03T_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <string.h>

errStatus_t appl_comm_i03t_request_cell_data(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout);
errStatus_t appl_comm_i03t_request_status_data(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout);
errStatus_t appl_comm_i03t_request_cell_alarmdata(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout);
errStatus_t appl_comm_i03t_request_paradata(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout);
errStatus_t appl_comm_i03t_request_sn(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout,uint8_t trytimes);
errStatus_t appl_comm_i03t_request_config(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout);
errStatus_t appl_comm_i03t_request_synch_sn(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout);
errStatus_t appl_comm_i03t_request_intres_sample(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout);
errStatus_t appl_comm_i03t_request_reset(COMM_TYPE_t commType,uint8_t i03t_addr,uint16_t timeout);
#ifdef  __cplusplus
}
#endif

#endif
