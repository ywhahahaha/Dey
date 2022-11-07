#ifndef __MODULE_DISCHARGE_H_
#define __MODULE_DISCHARGE_H_

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>

errStatus_t module_discharge_load_protocol(uint8_t i03t_addr,LoggerQuery_t *query);

#ifdef  __cplusplus
}
#endif

#endif

