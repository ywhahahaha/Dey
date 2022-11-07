#ifndef __DE_PROTOCOL_FRAME_H_
#define __DE_PROTOCOL_FRAME_H_

#ifdef __cplusplus
 extern "C" {
#endif
     
#include <stdint.h>
#include "typedef.h"



#pragma pack(1)
typedef struct {
	uint8_t  sof;
	uint8_t  dest;
	uint8_t  source;
	uint8_t  ctrl;
	uint8_t  ser;
	uint8_t  data_length_h;
	uint8_t  data_length_l;
	uint8_t  tlvs[]; 
}de_protocol_t;
#pragma pack()


#ifdef __cplusplus
}
#endif

#endif
