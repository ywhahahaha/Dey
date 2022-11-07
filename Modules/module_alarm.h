#ifndef __MODULE_ALARM_H_
#define __MODULE_ALARM_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#include "typedef.h"
#include "project_config.h"

typedef struct {
    uint32_t    crc_chk;
    uint32_t    time;
    uint32_t    index;
    TotalInfo_t total_info;
    AlarmInfo_t alarm;
}AlarmStorage_t;



errStatus_t module_alarm_load_protocol(uint8_t i03t_addr,LoggerQuery_t *query);

#ifdef  __cplusplus
}
#endif

#endif
