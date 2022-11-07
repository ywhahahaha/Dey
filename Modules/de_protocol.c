#include <string.h>
#include <stdbool.h>
#include "de_protocol.h"
#include "de_protocol_frame.h"
#include "module_sn.h"
#include "main.h"
#include "cmsis_os2.h"
#include "appl_alarm.h"
//#include "iap.h"

static uint8_t protocol_ser = 0;
static uint8_t query_object = 0;
static uint8_t query_dataid = 0;
//logger
static DATE_yymmddhhmmss_t query_start_ts = {0};
static DATE_yymmddhhmmss_t query_end_ts = {0};

static uint8_t crc8( uint8_t * pdata, uint16_t length ){
	uint8_t crc = 0;

	if ( length <= 0 ) {
		return crc;
	}
	while( length-- != 0 ) {
		for ( uint16_t i = 0x80; i != 0; i /= 2 ) {
			if ( (crc & 0x80) != 0) {
				crc *= 2;
				crc ^= 0x07; //
			} else {
				crc *= 2;
			}
			if ( (*pdata & i) != 0 ) {
				crc ^= 0x07;
			}
		}
		pdata++;
	}
	
	return crc;
}

static uint8_t *de_protocol_find_head(uint8_t *pdata,uint16_t *length) {
    uint16_t temp = *length;
     
    for(uint16_t index = 0;index < temp;index++) {
        if(pdata[index] == 0x68) {
            *length = temp - index;
            return pdata + index;
        }
    }
    
    return NULL;
}

de_protocol_t *de_protocol_check(uint8_t *p,uint16_t length) {
	
	if(p == NULL) return NULL;
    
    uint16_t _length = length;
    
    uint8_t *pdata = de_protocol_find_head(p,&_length);
    
    de_protocol_t *protocol = (de_protocol_t *)pdata;
    
    if(protocol == NULL) {
        return NULL;
    }

	if(protocol->sof != 0x68) return NULL;

	if(_length < 9) return NULL;

	uint16_t data_area_length = (uint16_t)protocol->data_length_h << 8 | protocol->data_length_l;

	uint16_t frame_length = data_area_length + 9;
	
	if(frame_length > _length) return NULL;

	if(pdata[frame_length-1] != 0x16) return NULL;
	
	uint8_t checksum = crc8(pdata,frame_length - 2);

	return (checksum == pdata[frame_length-2]) ? protocol : NULL;
}


uint16_t fill_query_logger( COMM_TYPE_t commType,
                            uint8_t dest,
                            uint8_t ser,
                            uint8_t i03t_addr,
                            uint8_t data_id,
                            uint32_t start_time,
                            uint32_t end_time,
                            uint8_t *p);

uint16_t fill_query_status(uint8_t i03t,uint8_t dataid,uint8_t *p,uint8_t *remain);

uint16_t de_protocol_slave_package_fill(uint8_t  *p,
							  uint8_t        ctrl,
		                      uint8_t        src,
					          uint8_t        dest,
					          uint8_t        ser,
					          uint16_t       tag_mask,
                              uint8_t        *pdata,
                              uint16_t       length)
{
    uint16_t index = 0;
	uint16_t index_tlv_length;
	uint16_t index_v_length;
	uint16_t temp_length;
	p[index++] = 0x68;

	p[index++] = dest;
	p[index++] = src;
	p[index++] = ctrl;
	p[index++] = ser;

	index_tlv_length = index;
	p[index++] = 0;
	p[index++] = 0;


    if(tag_mask & TAG_MASK_Logger) {
        p[index++] = TAG_Logger;

		index_v_length = index;
	    p[index++] = 0;
		p[index++] = 0;

        memcpy(p + index,pdata,length);
        index += length;
        
		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;

	}

	temp_length = (index - index_tlv_length - 2);
	p[index_tlv_length] = temp_length >> 8;
	p[index_tlv_length + 1] = temp_length;

	p[index] = crc8(p,index);
	index++;
	p[index++] = 0x16;

	return index;
}
uint16_t fill_query_config_sn(uint8_t i03t,uint8_t dataid,uint8_t *p,uint8_t *remain);
uint16_t fill_query_config(uint8_t addr,uint8_t *p);
void LoggerRequest(Logger_Msg_t logger);
uint16_t de_protocol_slave_package( COMM_TYPE_t commType,
                                    uint8_t  *p,
                                    uint8_t        ctrl,
                                    uint8_t        src,
                                    uint8_t        dest,
                                    uint8_t        ser,
                                    uint16_t       tag_mask,
                                    HandleResult  *result,
                                    uint8_t        resultcnt,
                                    uint8_t       *remain)
{
	uint16_t index = 0;
	uint16_t index_tlv_length;
	uint16_t index_v_length;
	uint16_t temp_length;
	p[index++] = 0x68;

	p[index++] = dest; 
	p[index++] = src;  
	p[index++] = ctrl;
	p[index++] = ser;

	index_tlv_length = index;
	p[index++] = 0;
	p[index++] = 0;

	if(tag_mask & TAG_MASK_Status) {
		p[index++] = TAG_Status;

		index_v_length = index;
	    p[index++] = 0;
		p[index++] = 0;

        index += fill_query_status(query_object,query_dataid,p+index,remain);
        if(*remain) {
            p[4] &= ~SER_EOF;
        } else {
            p[4] |= SER_EOF;
        }

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;

	}

	if(tag_mask & TAG_MASK_Control) {
        p[index++] = TAG_Control;
		index_v_length = index;
		p[index++] = 0;
		p[index++] = 0;

        for(uint8_t i=0;i<resultcnt;i++) {
            p[index++] = result[i].DataId;
            p[index++] = result[i].result;
        }

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;

	}
    
    if(tag_mask & TAG_MASK_Logger) {
        p[index++] = TAG_Logger;

		index_v_length = index;
	    p[index++] = 0;
		p[index++] = 0;

        uint16_t  temp = fill_query_logger(commType,dest,ser,query_object,query_dataid,query_start_ts.date,query_end_ts.date,p+index);
        if(temp == 0) {
            return 0;
        }
        
        index += temp;

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;

	}

	if(tag_mask & TAG_MASK_Config) {
		p[index++] = TAG_Config;
		index_v_length = index;
		p[index++] = 0;
		p[index++] = 0;

		if(ctrl & CTRL_WriteAck) {
			for(uint8_t i=0;i<resultcnt;i++) {
				p[index++] = result[i].DataId;
				p[index++] = result[i].result;
			}
		} else {

            if(query_dataid == ConfigDataId_AddSN || query_dataid == ConfigDataId_AddSNCloudId) {
                index += fill_query_config_sn(query_object,query_dataid,p+index,remain);  
                if(*remain) {
                    p[4] &= ~SER_EOF;
                } else {
                    p[4] |= SER_EOF;
                }
            } else {
                index += fill_query_config(query_object,p+index);
            }
		}

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;
	}
    
    if(tag_mask & TAG_MASK_Upgrade) {
		p[index++] = TAG_Upgrade;
		index_v_length = index;
		p[index++] = 0;
		p[index++] = 0;

		
        for(uint8_t i=0;i<resultcnt;i++) {
            p[index++] = result[i].DataId;
            p[index++] = result[i].result;
        }

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;
	}
    
    if(tag_mask & TAG_MASK_Transfer) {
        p[index++] = TAG_TransferData;
		index_v_length = index;
		p[index++] = 0;
		p[index++] = 0;

        for(uint8_t i=0;i<resultcnt;i++) {
            if(result[i].transfer) {
                p[index++] = result[i].DataId;
                memcpy(p+index,result[i].extData,result[i].extLength);
            } else {
                p[index++] = result[i].DataId;
                p[index++] = result[i].result;
            }
        }

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;
    }
    
	temp_length = (index - index_tlv_length - 2);
	p[index_tlv_length] = temp_length >> 8;
	p[index_tlv_length + 1] = temp_length;

	p[index] = crc8(p,index);
	index++;
	p[index++] = 0x16;

	return index;


}


uint16_t de_protocol_master_package(uint8_t       *p,
							         uint8_t        ctrl,
		                             uint8_t        src,
					                 uint8_t        dest,
					                 uint8_t        ser,
					                 uint32_t       tag_mask,
									 uint8_t        query_detail,
									 uint8_t       *p_content,
									 uint16_t       p_length,
						             HandleResult  *result,
						             uint8_t        resultcnt)
{
	uint16_t index = 0;
	uint16_t index_tlv_length;
	uint16_t index_v_length;
	uint16_t temp_length;
	p[index++] = 0x68;

	p[index++] = dest;
	p[index++] = src;
	p[index++] = ctrl;
	p[index++] = ser;

	index_tlv_length = index;
	p[index++] = 0;
	p[index++] = 0;

	if(tag_mask & TAG_MASK_Query) {
		p[index++] = TAG_Query;

		index_v_length = index;
		p[index++] = 0;
		p[index++] = 0;

        if(p_length > 0) {
			memcpy(p+index,p_content,p_length);
			index += p_length;
		}

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;
	}


	if(tag_mask & TAG_MASK_Control) {
		p[index++] = TAG_Control;

		index_v_length = index;
		p[index++] = 0;
		p[index++] = 0;

		if(p_length > 0) {
			memcpy(p+index,p_content,p_length);
			index += p_length;
		}

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;

	}

	if(tag_mask & TAG_MASK_Config) {
		p[index++] = TAG_Config;

		index_v_length = index;
		p[index++] = 0;
		p[index++] = 0;

		if(p_length > 0) {
			memcpy(p+index,p_content,p_length);
			index += p_length;
		}

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;
	}

	if(tag_mask & TAG_MASK_Upgrade) {
		p[index++] = TAG_Upgrade;

		index_v_length = index;
		p[index++] = 0;
		p[index++] = 0;

		if(p_length > 0) {
			memcpy(p+index,p_content,p_length);
			index += p_length;
		}

		temp_length = index - index_v_length - 2;
		p[index_v_length] = temp_length >> 8;
		p[index_v_length + 1] = temp_length;

	}

	temp_length = (index - index_tlv_length - 2);
	p[index_tlv_length] = temp_length >> 8;
	p[index_tlv_length + 1] = temp_length;

	p[index] = crc8(p,index);
	index++;
	p[index++] = 0x16;

	return index;

}
void i03t_module_query_status_response_handle(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,uint8_t ser);
void i03t_module_query_config_response_handle(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,uint8_t ser);
void i03t_module_config_response_handle(uint8_t *pdata,uint16_t length,uint8_t ser);
void i03t_module_control_response_handle(uint8_t *pdata,uint16_t length,uint8_t ser);
void de_protocol_master_process(COMM_TYPE_t type,uint8_t *pdata,uint16_t length) {
	de_protocol_t *pMsg;

	uint16_t tlvs_length;
	uint16_t index;
	uint8_t   T;
	uint16_t  L;
	uint8_t  *V;
	uint8_t  control;
	//uint16_t  tagmask = 0;

	//HandleResult result = {0};
	//uint8_t      result_cnt = 0;

	

	pMsg = de_protocol_check(pdata, length);
    
    if(pMsg == NULL)
		return;
    
    if(pMsg->dest != pDEApp->device_config.i03m.i03m_addr && pMsg->dest != 0xFF && pMsg->dest != 0x00) {
        return;
    }

	tlvs_length = (uint16_t)pMsg->data_length_h << 8 | pMsg->data_length_l;
	control = pMsg->ctrl;
	index = 0;

	while(index + 4 < tlvs_length) {
		T = pMsg->tlvs[index++];
		L = pMsg->tlvs[index] << 8 | pMsg->tlvs[index+1];
		index += 2;
		V = pMsg->tlvs + index;
		switch(T) {
			case TAG_Status:
				i03t_module_query_status_response_handle(pMsg->source,V,L,pMsg->ser);
				break;

			case TAG_Control:
                if(control & CTRL_WriteAck) {
                    i03t_module_control_response_handle(V,L,pMsg->ser);
				} 
				break;

			case TAG_Config:
				if(control & CTRL_WriteAck) {
                    i03t_module_config_response_handle(V,L,pMsg->ser);
				} else {
					i03t_module_query_config_response_handle(pMsg->source,V,L,pMsg->ser);
				}
				break;

			case TAG_Upgrade:
				break;
			default:
				break;
		}
		index += L;
	}
}
void debug_sendbytes(uint8_t *pdata,uint16_t bytes);
void protocol_send_bytes(COMM_TYPE_t type,uint8_t *pdata,uint16_t length) {
    switch(type) {
        case COMM_TYPE_ETH:
            break;
        case COMM_TYPE_USB:
            break;
        
        case COMM_TYPE_COM:
            debug_sendbytes(pdata,length);
            break;
        case COMM_TYPE_485_1:
            Bsp_Rs485_1_SendBytes(pdata,length);
            break;
        case COMM_TYPE_485_2:
            Bsp_Rs485_2_SendBytes(pdata,length);
            break;
        case COMM_TYPE_485_3:
            Bsp_Rs485_3_SendBytes(pdata,length);
            break;
        case COMM_TYPE_485_4:
            Bsp_Rs485_4_SendBytes(pdata,length);
            break;

        default:
            break;
    }
}
void IAP_SetStatus(void);
void protocol_set_query_start_cell_id(uint16_t id);
void protocol_handle_upgrade(uint8_t *pdata,uint16_t length,HandleResult *result,int *resultcnt);
void protocol_handle_config(uint8_t *pdata,uint16_t length,HandleResult *result,int *resultcnt);
void protocol_handle_control(uint8_t *pdata,uint16_t length,HandleResult *result,int *resultcnt);
bool de_protocol_process(COMM_TYPE_t type,uint8_t *pdata,uint16_t length) {
    
	de_protocol_t *pMsg;
	uint8_t *p = NULL;
	uint8_t break_flag = 0;
	uint8_t ctrl_code = CTRL_Response;
	
	uint16_t  tlvs_length;
	uint16_t  index;
	uint8_t   T;
	uint16_t  L;

	uint32_t  tagmask = 0;
    
	
	HandleResult *result = NULL;
	int           result_cnt = 0;

    pMsg = de_protocol_check(pdata, length);
    
    if(pMsg == NULL)
		return false;
    
    if(pMsg->dest != pDEApp->device_config.i03m.i03m_addr && pMsg->dest != 0xFF && pMsg->dest != 0x00) {
        return false;
    }

	tlvs_length = (uint16_t)pMsg->data_length_h << 8 | pMsg->data_length_l;
	
	index = 0;
    
    uint8_t ser_ = SER_EOF | SER_SOF;
	uint8_t query_what;
	while(index + 4 < tlvs_length && !break_flag ) {
		T = pMsg->tlvs[index++];
		L = pMsg->tlvs[index] << 8 | pMsg->tlvs[index+1];
		index += 2;

		switch(T) {
			case TAG_Query:
				{
					query_what = pMsg->tlvs[index++];
					switch(query_what) {
					case TAG_Status:
						tagmask |= TAG_MASK_Status;
                        query_object = pMsg->tlvs[index++];   //0: i03M, other:i03T
                        query_dataid = pMsg->tlvs[index++]; //0: all, other: dataid
                        protocol_set_query_start_cell_id(1);
						break_flag = 1;
						break;
					case TAG_Config:
						tagmask |= TAG_MASK_Config;
                        query_object = pMsg->tlvs[index++];   //0: i03M, other:i03T
                        query_dataid = pMsg->tlvs[index++]; //0: all, other: dataid
                        protocol_set_query_start_cell_id(1);
						break_flag = 1;
						break;
					case TAG_Control:
						tagmask |= TAG_MASK_Control;
						break_flag = 1;
						break;
                    case TAG_Logger:
                        tagmask |= TAG_MASK_Logger;
                        ser_ = SER_SOF;
                        query_object = pMsg->tlvs[index++];             //0: i03M, other:i03T 
                        query_dataid = pMsg->tlvs[index++];             //0: all, other: dataid
                        
                        query_start_ts.bits.year = pMsg->tlvs[index++];
                        query_start_ts.bits.month = pMsg->tlvs[index++];
                        query_start_ts.bits.day = pMsg->tlvs[index++];
                        query_start_ts.bits.hour = pMsg->tlvs[index++];
                        query_start_ts.bits.min = pMsg->tlvs[index++];
                        query_start_ts.bits.sec = pMsg->tlvs[index++];
                    
                        query_end_ts.bits.year = pMsg->tlvs[index++];
                        query_end_ts.bits.month = pMsg->tlvs[index++];
                        query_end_ts.bits.day = pMsg->tlvs[index++];
                        query_end_ts.bits.hour = pMsg->tlvs[index++];
                        query_end_ts.bits.min = pMsg->tlvs[index++];
                        query_end_ts.bits.sec = pMsg->tlvs[index++];
						break_flag = 1;
                    
                        break;
                    
					default:
						break_flag = 1;
						break;
					}
				}
				break;

			case TAG_Control:
                ctrl_code |= CTRL_WriteAck;
				tagmask |= TAG_MASK_Control;
				result = (HandleResult *)sys_malloc(10 * sizeof(HandleResult));
				if(result == NULL) {
					return true;
				}
				protocol_handle_control(pMsg->tlvs + index,L,result,&result_cnt);
				break_flag = 1;
				index += L;
				break;


			case TAG_Config:
				ctrl_code |= CTRL_WriteAck;
				tagmask |= TAG_MASK_Config;
				result = (HandleResult *)sys_malloc(10 * sizeof(HandleResult));
				if(result == NULL) {
					return true;
				}
                                
				protocol_handle_config(pMsg->tlvs + index,L,result,&result_cnt);
                
				break_flag = 1;
				index += L;
                
				break;
            case TAG_Logger:
                return false;
				
			case TAG_Upgrade:
				ctrl_code |= CTRL_WriteAck;
				tagmask |= TAG_MASK_Upgrade;
				result = (HandleResult *)sys_malloc(6 * sizeof(HandleResult));
				if(result == NULL) {
					return true;
				}
                IAP_SetStatus();
				protocol_handle_upgrade(pMsg->tlvs + index,L,result,&result_cnt);
				index += L;
				break;

			case TAG_TransferData:{
                uint8_t transfer_type = pMsg->tlvs[index++];
                tagmask |= TAG_MASK_Transfer;
                CodeResult_t err = Result_MsgErr;
                switch(transfer_type) {
                    case 0x01:
                        if(L < 9) {
                            break;
                        }

                        if(de_protocol_check(pMsg->tlvs+index, L-1) == NULL) {
                            break;
                        }
                             
                        err = Result_OK;
                        //CellTranferRequest(pMsg->tlvs+index,L-1);
                        protocol_send_bytes(COMM_I03T_PORT,pMsg->tlvs+index,L-1);
                        break;
                    default:
                        break;
                }
                result = (HandleResult *)sys_malloc(sizeof(HandleResult));
                result[0].DataId = transfer_type;
                result[0].result = err;
                result[0].extLength = 0;
                result_cnt = 1;
                index += L;
                }
                
				break;
			default:
				break;
		}
	}
	
    if(!(pMsg->ctrl & CTRL_Response)) {
        p = (uint8_t *)sys_malloc(PROTOCOL_RX_MAX_SIZE);
        if(p == NULL) {
            goto Exit;
        }
        uint8_t remain = 0;
        
        while(1) {
            
            uint16_t send_length = de_protocol_slave_package(type,
                                                        p,
                                                        ctrl_code,
                                                        pDEApp->device_config.i03m.i03m_addr,
                                                        pMsg->source,
                                                        ser_ | (pMsg->ser & 0x3F),
                                                        tagmask,
                                                        result,
                                                        result_cnt,
                                                        &remain);

            if(send_length > 0) {
                protocol_send_bytes(type,p,send_length);
            }
            
            if(!remain) {
                break;
            } 
            
            if(T == TAG_Query && query_what == TAG_Logger) {
                osDelay(20);
            } else {
                osDelay(200);
            }
        }
        
        sys_free(p);
        p = NULL;
    }

Exit:
	if(result != NULL) {
		sys_free(result);
	}
    
    return true;
}

//------------------------------------------------------
#if 1

osMessageQueueId_t  mid_I03TComm = NULL;                                 // message queue id

void i03t_comm_init(void) {
    mid_I03TComm = osMessageQueueNew(4, sizeof(void *),NULL);
}


void i03t_module_control_response_handle(uint8_t *pdata,uint16_t length,uint8_t ser) {
	uint16_t index = 0;
	uint8_t err_count = 0;

	while(index < length && !err_count) {
		uint8_t data_id = pdata[index++];
		if(Result_OK != pdata[index++]) {
			err_count++;
			break;
		}
	}

	ModuleRxMsg_t *p = sys_malloc(sizeof(ModuleRxMsg_t));
    if(p == NULL) {
        return;
    }
    
    p->err = (err_count ? errErr : errOK);
	p->tag = TAG_Control;
	p->ser = ser;
	osStatus_t status =  osMessageQueuePut(mid_I03TComm,&p,osPriorityNormal,500);
    if(status != osOK) {
        sys_free(p);
    }
}

void i03t_module_config_response_handle(uint8_t *pdata,uint16_t length,uint8_t ser) {
	uint16_t index = 0;
	uint8_t err_count = 0;

	while(index < length && !err_count) {
		uint8_t data_id = pdata[index++];
		if(Result_OK != pdata[index++]) {
			err_count++;
			break;
		}
	}

	ModuleRxMsg_t *p = sys_malloc(sizeof(ModuleRxMsg_t));
    if(p == NULL) {
        return;
    }
    
    p->err = (err_count ? errErr : errOK);
	p->tag = TAG_Config;
	p->ser = ser;
	osStatus_t status =  osMessageQueuePut(mid_I03TComm,&p,osPriorityNormal,500);
    if(status != osOK) {
        sys_free(p);
    }
}

void i03t_module_query_config_response_handle(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,uint8_t ser) {
    uint16_t index = 0;
    errStatus_t err = errOK;
    
    bool query_para = false;
    
    if(i03t_addr < 1 || i03t_addr > CONFIG_MAX_IO3T_MODBUS_ADDR) {
        return;
    }
    
    uint8_t i03t_index = i03t_addr - 1;
    
    I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
    if(i03t_node == NULL) {
        return;
    }
    
    Convt_t convt = {0};
        
    uint8_t err_cnt = 0;
    
    I03T_Para_t *config = (I03T_Para_t *)sys_malloc(sizeof(I03T_Para_t));
    if(config == NULL) {
        return;
    }

    memset(config,0,sizeof(I03T_Para_t));

    memcpy(&config->base_para,&pDEApp->device_config.i03t_nodes[i03t_index].base_para,sizeof(I03T_BasePara_t));
    
    while(index < length) {
        uint8_t data_id = pdata[index++];
        
        switch(data_id) {
            case 0:
                index ++;
                break;
            case ConfigDataId_BaudRate:
                index += 4;
                break;

            case ConfigDataId_TotalVoltageSamplePeriod:
                query_para = true;
                config->sample_period.period_total_volt = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;
            
            case ConfigDataId_CellPullPeriod:
                 config->sample_period.period_cell_poll = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;     
            
            case ConfigDataId_ResSamplePeriod:
                config->sample_period.period_int_res = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;   
            
            case ConfigDataId_CellNumber:
                config->sys_para.cell_number = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;         
            
            case ConfigDataId_CriticalCurrentLimits:
                 config->charge_status_threshold.critical_current = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;       
            
            case ConfigDataId_FastToFloat:
                config->charge_status_threshold.fast_to_float = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;                 
            case ConfigDataId_FloatToFast:
                config->charge_status_threshold.float_to_fast = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;                 
            case ConfigDataId_ChargeToDischarge:
                config->charge_status_threshold.charge_to_discharge = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;           
            case ConfigDataId_DrySwitch:
                config->dry_node.dry_switch = pdata[index++];
                break;                   
            case ConfigDataId_DryOutputThreshold:
                config->dry_node.out_threshold = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;          
            case ConfigDataId_DryResumeThreshold:
                config->dry_node.output_resume_threshold = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;          
            case ConfigDataId_DryDelay:
                config->dry_node.delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;                    
            case ConfigDataId_DryOutputMinVoltage:
                config->dry_node.output_min_volt = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;         
            case ConfigDataId_433WorkFreq:
                config->wireless.work_freq = pdata[index++];
                break;                 
            case ConfigDataId_433AssistFreq:
                config->wireless.assist_freq = pdata[index++];
                break;        
            case ConfigDataId_Protocol:
                config->sys_para.protocol_type = pdata[index++];
                break;
            case ConfigDataId_NominalCap:
                config->cap.nominal_cap = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;           
            case ConfigDataId_NominalVolt:
                config->cap.nominal_volt = pdata[index++];
                break;                 
            case ConfigDataId_FloatVolt:
                config->cap.float_volt = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;                   
            case ConfigDataId_TempCompensate:
                config->cap.temp_compensate = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;              
            case ConfigDataId_DischargeMethod:
                config->cap.discharge_method = pdata[index++];
                break;             
            case ConfigDataId_ChargeMethod:
                config->cap.charge_method = pdata[index++];
                break;                
            case ConfigDataId_ChargeEfficiency:
                config->cap.charge_efficiency = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;            
            case ConfigDataId_MaxDischargeCap:
                config->cap.max_discharge_cap = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;             
            case ConfigDataId_CellMinVolt:
                config->cap.cell_min_voltage = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;                 
            case ConfigDataId_CellMaxVolt:
                config->cap.cell_max_voltage = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;                 
            case ConfigDataId_DischargeMaxCurrent:
                config->cap.discharge_max_current = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;  
            
            case ConfigDataId_DischargeMinCurrent:
                config->cap.discharge_min_current = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;   
            
            case ConfigDataId_MinCalculateTemp:
                config->cap.min_calculate_temp = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;    
            
            case ConfigDataId_MaxCalculateTemp:
                config->cap.max_calculate_temp = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break; 
            
            case ConfigDataId_CapInitFlg:
                config->cap.cap_init = pdata[index++];
                break; 
            
            case ConfigDataId_CurrentGroupNumber:
                config->cap.current_group_number = pdata[index++];
                break;
            
            case ConfigDataId_TotalVoltageThreshold:
                config->alarm_threshold.total_voltage.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.total_voltage.low.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.low.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.low.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.low.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.low.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.low.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.low.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.low.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_voltage.low.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;
                
                
            case ConfigDataId_ChargeOverCurrentThreshold:
                config->alarm_threshold.total_current.over_charge.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_charge.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_charge.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_charge.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_charge.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_charge.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_charge.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_charge.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_charge.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;  
            
            case ConfigDataId_DischargeOverCurrentThreshold:
                config->alarm_threshold.total_current.over_discharge.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_discharge.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_discharge.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_discharge.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_discharge.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_discharge.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_discharge.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_discharge.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.total_current.over_discharge.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;
            case ConfigDataId_TempOverThreshold:
                config->alarm_threshold.temp.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.temp.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.temp.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.temp.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.temp.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.temp.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.temp.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.temp.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.temp.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;           
            case ConfigDataId_SocLowThreshold:
                config->alarm_threshold.soc.low.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.soc.low.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.soc.low.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.soc.low.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.soc.low.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.soc.low.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.soc.low.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.soc.low.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.soc.low.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;             
            case ConfigDataId_CellVoltOverThreshold:
                config->alarm_threshold.cell.voltage.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.cell.voltage.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.cell.voltage.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;       
            case ConfigDataId_CellVoltLowThreshold:
                config->alarm_threshold.cell.voltage.low.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.low.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.low.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.cell.voltage.low.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.low.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.low.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.cell.voltage.low.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.low.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.voltage.low.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;        
            case ConfigDataId_CellTempOverThreshold:
                config->alarm_threshold.cell.temp.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.temp.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.temp.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.cell.temp.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.temp.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.temp.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.cell.temp.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.temp.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.temp.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;       
            case ConfigDataId_CellInterResOverThreshold:
                config->alarm_threshold.cell.res.over.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.res.over.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.res.over.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.cell.res.over.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.res.over.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.res.over.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.cell.res.over.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.res.over.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.cell.res.over.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;   
                
            case ConfigDataId_LeakThreshold:
                config->alarm_threshold.leak.leak.level_1 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.leak.leak.level_2 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.leak.leak.level_3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.leak.leak.level_1_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.leak.leak.level_2_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.leak.leak.level_3_resume = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                config->alarm_threshold.leak.leak.level_1_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.leak.leak.level_2_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->alarm_threshold.leak.leak.level_3_delay = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;
                
            case ConfigDataId_AlarmCorrelation:
                config->dry_node.alarm_correlation.alarm_correlation = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->dry_node.alarm_correlation.alarm_correlation_group = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                config->dry_node.alarm_correlation.alarm_correlation_cells = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;
            
            case ConfigDataId_VoltageLevel:
                config->sys_para.voltage_level = pdata[index++];
                break;         
            case ConfigDataId_CurrentLevel:
                config->sys_para.current_level = pdata[index++];
                break;                
            case ConfigDataId_CellSNLength:
                config->sys_para.sn_length = pdata[index++];
                break;                
            case ConfigDataId_AlarmMaxLevel:
                config->sys_para.alarm_max_level = pdata[index++];
                break;               
            case ConfigDataId_CellWorkMode:
                config->sys_para.cell_mode = pdata[index++];
                break;                
            case ConfigDataId_VoltageCalib:
                {
                    uint8_t ch = pdata[index++];
                    
                    config->calib.total_voltage.zero_offset = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    
                    convt.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index + 1] << 16 | (uint16_t)pdata[index + 2] << 8 | pdata[index + 3];
                    index += 4;
                    config->calib.total_voltage.kb.k = convt.f;
                    
                    convt.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index + 1] << 16 | (uint16_t)pdata[index + 2] << 8 | pdata[index + 3];
                    index += 4;
                    config->calib.total_voltage.kb.b = convt.f;
                    
                }
                break;                
            case ConfigDataId_CurrentCalib:
                {
                    uint8_t ch = pdata[index++];
                    if(ch > CONFIG_MAX_CURRENT_CH || ch < 1) {
                        err_cnt++;
                        break;
                    }
                    ch--;
                    config->calib.current[ch].zero_offset = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    
                    convt.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index + 1] << 16 | (uint16_t)pdata[index + 2] << 8 | pdata[index + 3];
                    index += 4;
                    config->calib.current[ch].kb.k = convt.f;
                    
                    convt.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index + 1] << 16 | (uint16_t)pdata[index + 2] << 8 | pdata[index + 3];
                    index += 4;
                    config->calib.current[ch].kb.b = convt.f;
                    
                }
                break;                
            case ConfigDataId_TempCalib:
                {
                    convt.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index + 1] << 16 | (uint16_t)pdata[index + 2] << 8 | pdata[index + 3];
                    index += 4;
                    config->calib.temp.kb.k = convt.f;
                    
                    convt.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index + 1] << 16 | (uint16_t)pdata[index + 2] << 8 | pdata[index + 3];
                    index += 4;
                    config->calib.temp.kb.b = convt.f;
                }
                break;                   
            case ConfigDataId_Volt5VCalib:
                {
                    convt.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index + 1] << 16 | (uint16_t)pdata[index + 2] << 8 | pdata[index + 3];
                    index += 4;
                    config->calib.voltage_5v.kb.k = convt.f;
                    
                    convt.u = (uint32_t)pdata[index] << 24 | (uint32_t)pdata[index + 1] << 16 | (uint16_t)pdata[index + 2] << 8 | pdata[index + 3];
                    index += 4;
                    config->calib.voltage_5v.kb.b = convt.f;
                }
                break;    

            case ConfigDataId_AddSN://
                {
                    SNStore_t sn_info = {0};
                    sn_info.i03t_addr = i03t_addr;
                    sn_info.cell_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    
                    sn_info.cell_on_current_group = pdata[index++];
                    
                    memcpy(sn_info.sn,pdata + index,CONFIG_SN_LENGTH);
                    index += 20;

                    SNStore_t *sn_temp = (SNStore_t *)module_sn_get(sn_info.sn);
                    if(sn_temp == NULL) {
                        i03t_node->flag.bits.synch_sn_need = 1;
                        i03t_node->sn_request.sn_err_cnt++;   
                        i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
                    } else {
                        if(sn_temp->cell_id != sn_info.cell_id \
                           || sn_temp->i03t_addr != sn_info.i03t_addr \
                           || memcmp(sn_temp->sn,sn_info.sn,CONFIG_SN_LENGTH)) {
                            
                            i03t_node->flag.bits.synch_sn_need = 1;
                            i03t_node->sn_request.sn_err_cnt++;  
                            i03t_node->sn_request.sn_synch_time = osKernelGetTickCount();
                            
                        } else {
                            i03t_node->sn_request.sn_cnt++;
                        }
                           
                        sys_free(sn_temp);
                    }
                    
                    if(pDEApp->Flag.bits.print_flag) {
                        char sn[CONFIG_SN_LENGTH + 1] = {0};
                        memcpy(sn,sn_info.sn,CONFIG_SN_LENGTH);
                        debug_printf("I03T%d->cell id:%d,cell sn:%s,synch_sn_need:%d,err_cnt:%d,sn_cnt:%d\r\n",i03t_addr,
                                                                             sn_info.cell_id,\
                                                                             sn,
                                                                             i03t_node->flag.bits.synch_sn_need,\
                                                                             i03t_node->sn_request.sn_err_cnt,\
                                                                             i03t_node->sn_request.sn_cnt);
                    }
                    
                }
                break;
  
            default:
                err_cnt ++;
                break;
        }
        
        if(err_cnt > 0) {
            break;
        }
    }
    
  
    if(query_para) {      
/*
        uint8_t *temp1 = (uint8_t *)config;
        uint8_t *temp2 = (uint8_t *)&pDEApp->device_config.i03t_nodes[i03t_index];
        static uint8_t ch1;
        static uint8_t ch2;
        for(uint16_t i=0;i<sizeof(I03T_Para_t) - sizeof(CalibPara_t);i++) {
            ch1 = *temp1++;
            ch2 = *temp2++;
            if(ch1 != ch2) {
                i03t_node->flag.bits.synch_config = 0;
            }
        }
*/
        
        //pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number = config->sys_para.cell_number;
        config->sys_para.cell_number = pDEApp->device_config.i03t_nodes[i03t_index].sys_para.cell_number;
        memcpy(&config->alarm_threshold.soc,&pDEApp->device_config.i03t_nodes[i03t_index].alarm_threshold.soc,sizeof(ThresholdSOC_t));
        memcpy(&pDEApp->device_config.i03t_nodes[i03t_index].calib,&config->calib,sizeof(CalibPara_t));
        if(memcmp(config,&pDEApp->device_config.i03t_nodes[i03t_index],sizeof(I03T_Para_t) - sizeof(CalibPara_t))) {
            i03t_node->flag.bits.synch_config = 0;
        } else {
            i03t_node->flag.bits.synch_config = 1;
            memcpy(&pDEApp->device_config.i03t_nodes[i03t_index].calib,&config->calib,sizeof(CalibPara_t));
        }
    }
    

    sys_free(config);

	ModuleRxMsg_t *p = sys_malloc(sizeof(ModuleRxMsg_t));
    if(p == NULL) {
        return;
    }
    
    p->err = err;
	p->tag = TAG_Config;
	p->ser = ser;
	osStatus_t status =  osMessageQueuePut(mid_I03TComm,&p,osPriorityNormal,500);
    if(status != osOK) {
        sys_free(p);
    }
}

void i03t_module_query_status_response_handle(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,uint8_t ser) {
	uint16_t index = 0;
    errStatus_t err = errOK;
    
    if(i03t_addr < 1 || i03t_addr > CONFIG_MAX_IO3T_MODBUS_ADDR) {
        return;
    }
    
    //uint8_t i03t_index = i03t_addr - 1;
    
    I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
    if(i03t_node == NULL) {
        return;
    }
    
    //Convt_t convt = {0};
        
    uint8_t err_cnt = 0;
    
    while(index < length) {
        uint8_t data_id = pdata[index++];
        
        switch(data_id) {
            case 0:
                index ++;
                break;
            case StatusDataId_BatGroupStatus:
                {
                    uint8_t ch = pdata[index++];
                    uint8_t status = (ChargeStatus_t)pdata[index++];
                    if(ch == 1) {
                        i03t_node->discharge.status = status;
                    } 
                }
                break;						   					   
            case StatusDataId_BatGroupVoltage:
                i03t_node->discharge.voltage =  (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;  
            case StatusDataId_BatGroupCurrent:
                {
                    uint8_t ch = pdata[index++];
                    if(ch == 0 || ch > CONFIG_MAX_CURRENT_CH) {
                        index += 2;
                        break;
                    }
                    
                    i03t_node->discharge.current[ch-1] =  (int16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                }
                break;  					  					   
            case StatusDataId_BatGroupSOC:
                index += 3;
                break;   
            
            case StatusDataId_BatAvailableTime:
                index += 3;
                break; 
            
            case StatusDataId_SOH:
                index += 3;
                break;       
            
            case StatusDataId_DryNodeStatus:
                i03t_node->hist.total_info.dry_node_status = pdata[index++];
                break;    						   
            case StatusDataId_CellNumber:
                index += 2;
                break;   
            
            case StatusDataId_I03TNumber:
                index++;
                break;  
            
            case StatusDataId_CellPeakInfo:
                i03t_node->cell_volt_peak.max = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_volt_peak.min = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_volt_peak.average = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_volt_peak.max_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_volt_peak.min_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_volt_peak.diff = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;   
            
            case StatusDataId_TempPeakInfo:
                i03t_node->cell_temp_peak.max = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_temp_peak.min = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_temp_peak.average = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_temp_peak.max_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_temp_peak.min_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_temp_peak.diff = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
            
                i03t_node->discharge.temp = i03t_node->cell_temp_peak.average;
                break; 

            case StatusDataId_ResPeakInfo:
                i03t_node->cell_res_peak.max = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_res_peak.min = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_res_peak.average = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_res_peak.max_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_res_peak.min_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                i03t_node->cell_res_peak.diff = (int16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;
            
            case StatusDataId_SoftVersion:
                i03t_node->soft_version = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;
            case StatusDataId_HardVersion:
                i03t_node->hard_version = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                index += 2;
                break;
            case StatusDataId_CellInfo:
                {
                    uint16_t cell_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    if(cell_id > CONFIG_MAX_CELL || cell_id == 0) {
                        err_cnt ++;
                        break;
                    }
                    
                    i03t_node->hist.cells[cell_id-1].id = cell_id;
                    i03t_node->hist.cells[cell_id-1].voltage = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    i03t_node->hist.cells[cell_id-1].inter_res = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    i03t_node->hist.cells[cell_id-1].temperature = (int16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    
                    i03t_node->hist_cell_cnt++;
                }
                break; 
            case StatusDataId_BatGroupAlarmInfo:
                {
                    uint16_t temp = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    
                    if(!i03t_node->mount) {
                        AlarmLevel_t level = appl_alarm_get_status(i03t_node,AlarmTypeSocLow);
                        if(level > pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.alarm_max_level) {
                            level = (AlarmLevel_t)pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.alarm_max_level;
                        }
                        temp &=  ~(3u << I03T_GROUP1_ALARM_SOC_LOW_POS);
                        i03t_node->alarm.alarm.bat_group_alarm1 = (temp | (level << I03T_GROUP1_ALARM_SOC_LOW_POS));

                        temp = ((uint16_t)pdata[index] << 8 | pdata[index + 1]);
                        index += 2;
                        temp &= ~(1 << I03T_GROUP2_ALARM_COMM_ERR_POS);
                        i03t_node->alarm.alarm.bat_group_alarm2 = temp;
                        
                        i03t_node->alarm.alarm.bat_group_alarm3 = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                        index += 2;
                    } else {
                        i03t_node->alarm.alarm.bat_group_alarm1 = (temp & (3 << I03T_GROUP1_ALARM_TEMP_HI_POS));

                        i03t_node->alarm.alarm.bat_group_alarm2 = 0;
                        index += 2;

                        i03t_node->alarm.alarm.bat_group_alarm3 = 0;
                        index += 2;
                    }

                }
                break; 
            case StatusDataId_CellAlarmInfo:
                {
                    uint16_t cell_id = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                    if(cell_id > CONFIG_MAX_CELL || cell_id == 0) {
                        err_cnt ++;
                        break;
                    }
                    i03t_node->alarm.alarm.cell_alarm[cell_id-1] = (uint16_t)pdata[index] << 8 | pdata[index + 1];
                    index += 2;
                }
                break; 
                
            case StatusDataId_Time:
                index += 6;
                break;
            default:
                err_cnt ++;
                break;
        }
        
        if(err_cnt > 0) {
            break;
        }
    }

	ModuleRxMsg_t *p = sys_malloc(sizeof(ModuleRxMsg_t));
    if(p == NULL) {
        return;
    }
    p->err = err;
	p->tag = TAG_Status;
	p->ser = ser;
	osStatus_t status =  osMessageQueuePut(mid_I03TComm,&p,osPriorityNormal,500);
    if(status != osOK) {
        sys_free(p);
    }
}

void i03t_module_response_flush(void) {
    while(1) {
        uint32_t msg;
        osStatus_t status = osMessageQueueGet(mid_I03TComm,&msg,NULL,0);
                
        if(status == osOK) { 
            ModuleRxMsg_t *p = (ModuleRxMsg_t *)msg;
            if(p != NULL) {
                sys_free(p);
            } 
        } else {
            break;
        }
    }

}

errStatus_t i03t_module_send_and_response(COMM_TYPE_t commType,uint8_t *sendbytes,uint16_t length,uint8_t exp_ser,uint8_t exp_tag,uint32_t timeout,uint8_t try_times) {
	
    errStatus_t err = errErr;
    
	uint8_t trytimes = try_times;
    
    if(mid_I03TComm == NULL) {
        i03t_comm_init();
    }

    i03t_module_response_flush();
    
	while(trytimes--) {
        //debug_printf("i03t_module_send_and_response : %d\r\n",trytimes);
        
		protocol_send_bytes(commType,sendbytes,length);
        
		while(1) {
            uint32_t msg;
            
            osStatus_t status = osMessageQueueGet(mid_I03TComm,&msg,NULL,timeout);
            
            if(status == osOK) { 
                
                ModuleRxMsg_t *p = (ModuleRxMsg_t *)msg;
				if((p->ser & 0x3F) == (exp_ser & 0x3F) && \
					p->tag == exp_tag) {
					err = p->err;    
				} else {
                    err = errErr;
                }
                
                if(p->ser & SER_EOF && err == errOK) {
                    sys_free(p);
                    //debug_printf("i03t_module_send_and_response get end frame\r\n");
                    break;
                } else {
                   sys_free(p);
                   continue;
                }
            } else {
				err = errErr;
                //debug_printf("i03t_module_send_and_response timeout\r\n");
				break;
			}
		}

		if(err == errOK) {
			break;
		}
	}

	return err;

}
#endif
errStatus_t i03t_module_query(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length,uint8_t trytimes,uint32_t timeout) {

	uint8_t *p = (uint8_t *)sys_malloc(128);
	if(p == NULL) {
		return errErr;
	}

	uint8_t ser_no = protocol_ser++;

	uint16_t bytes = de_protocol_master_package(p, \
			CTRL_MustAck | CTRL_Request, \
			0x00, \
			target_addr, \
			SER_SINGLE | (ser_no & 0x3F), \
			TAG_MASK_Query, \
			0, \
			p_content,\
			length,\
			NULL, \
			0);

    errStatus_t err = i03t_module_send_and_response(commType,p,bytes,ser_no,tag,timeout,trytimes);
    
    sys_free(p);
    
	return err;

}

errStatus_t i03t_module_request_upgrade(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length ) {
	uint8_t *p = sys_malloc(1088);
	if(p == NULL) {
		return errErr;
	}
	uint8_t ser_no = protocol_ser++;
	uint16_t bytes = de_protocol_master_package(p, \
			CTRL_Request, \
			0x00, \
			0xff, \
			SER_SINGLE | (ser_no & 0x3F), \
			TAG_MASK_Upgrade, \
			0, \
			p_content,\
			length,\
			NULL, \
			0);
    protocol_send_bytes(commType,p,bytes);
    
    sys_free(p);
    
	return errOK;
}

errStatus_t i03t_module_config(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length ) {
	uint8_t *p = sys_malloc(512);
	if(p == NULL) {
		return errErr;
	}
	uint8_t ser_no = protocol_ser++;
	uint16_t bytes = de_protocol_master_package(p, \
			CTRL_MustAck | CTRL_Request, \
			0x00, \
			target_addr, \
			SER_SINGLE | (ser_no & 0x3F), \
			TAG_MASK_Config, \
			0, \
			p_content,\
			length,\
			NULL, \
			0);

    errStatus_t err = i03t_module_send_and_response(commType,p,bytes,ser_no,tag,PROTOCOL_COMMU_RESP_TIME_OUT,PROTOCOL_TRY_TIMES);
    
    sys_free(p);
    
	return err;
}

errStatus_t i03t_module_control(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length,uint8_t trytimes ) {
	uint8_t *p = sys_malloc(512);
	if(p == NULL) {
		return errErr;
	}
	uint8_t ser_no = protocol_ser++;
	uint16_t bytes = de_protocol_master_package(p, \
			CTRL_MustAck | CTRL_Request, \
			0x00, \
			target_addr, \
			SER_SINGLE | (ser_no & 0x3F), \
			TAG_MASK_Control, \
			0, \
			p_content,\
			length,\
			NULL, \
			0);

    errStatus_t err = i03t_module_send_and_response(commType,p,bytes,ser_no,tag,PROTOCOL_COMMU_RESP_TIME_OUT,trytimes);
    
    sys_free(p);
    
	return err;
}


