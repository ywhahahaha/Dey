#include "main.h"
#include "de_protocol.h"
#include "appl_alarm.h"

void appl_alarm_clear_all(I03T_Info_t *i03t,AlarmType_t type) {
    if(type >= AlarmTypeMax) {
        return;
    }
    
    if(i03t == NULL) {
        return;
    }
    
    for(uint8_t i=0;i<AlarmLevel_Max;i++) {
        i03t->alarm_msg[type][i].delay = 0;
        i03t->alarm_msg[type][i].status = AlarmReset;
    }
}


void appl_alarm_clear_below(I03T_Info_t *i03t,AlarmType_t type,AlarmLevel_t level) {
    if(type >= AlarmTypeMax || level >= AlarmLevel_Max) {
        return;
    }
    
    if(i03t == NULL) {
        return;
    }
    
    for(uint8_t i=0;i<level;i++) {
        i03t->alarm_msg[type][i].delay = 0;
        i03t->alarm_msg[type][i].status = AlarmReset;
    }
    
}

void appl_alarm_clear_level(I03T_Info_t *i03t,AlarmType_t type,AlarmLevel_t level) {
    if(type >= AlarmTypeMax || level >= AlarmLevel_Max) {
        return;
    }
    
    if(i03t == NULL) {
        return;
    }
    

    i03t->alarm_msg[type][level].delay = 0;
    i03t->alarm_msg[type][level].status = AlarmReset;

}


AlarmLevel_t appl_alarm_get_status(I03T_Info_t *i03t,AlarmType_t type) {
    for(int16_t i=AlarmLevel_Max-1; i > AlarmLevel_0; i--) {
        if(i03t->alarm_msg[type][i].status != AlarmReset) {
            return (AlarmLevel_t)i;
        }
    }
    
    return AlarmLevel_0;
}

//10 20 30
//15 25 35
void appl_alarm_check_low1(I03T_Info_t *i03t,AlarmType_t type,uint16_t value,ThresholdLevel_t *threshold) {
    if(i03t == NULL) {
        return;
    } 
    
    if(i03t->i03t_addr == 0 || i03t->i03t_addr > CONFIG_MAX_I03T) {
        return;
    }

    if(value <= threshold->level_3) {
        if(i03t->alarm_msg[type][AlarmLevel_3].delay < threshold->level_3_delay) {
            i03t->alarm_msg[type][AlarmLevel_3].status = AlarmReset;
            i03t->alarm_msg[type][AlarmLevel_3].delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_3].status = AlarmSet;
            appl_alarm_clear_level(i03t,type,AlarmLevel_2);
            appl_alarm_clear_level(i03t,type,AlarmLevel_1);
            appl_alarm_clear_level(i03t,type,AlarmLevel_0);
        }
    } else if(value <= threshold->level_2) {
        if(i03t->alarm_msg[type][AlarmLevel_3].status != AlarmSet) {
            if(i03t->alarm_msg[type][AlarmLevel_2].delay < threshold->level_2_delay) {
                i03t->alarm_msg[type][AlarmLevel_2].status = AlarmReset;
                i03t->alarm_msg[type][AlarmLevel_2].delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_2].status = AlarmSet;
                appl_alarm_clear_level(i03t,type,AlarmLevel_0);
                appl_alarm_clear_level(i03t,type,AlarmLevel_1);
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
            }
        }
    } else if(value <= threshold->level_1) {
        if(i03t->alarm_msg[type][AlarmLevel_2].status != AlarmSet) { 
            if(i03t->alarm_msg[type][AlarmLevel_1].delay < threshold->level_1_delay) {
                i03t->alarm_msg[type][AlarmLevel_1].status = AlarmReset;
                i03t->alarm_msg[type][AlarmLevel_1].delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_1].status = AlarmSet;
                appl_alarm_clear_level(i03t,type,AlarmLevel_0);
                appl_alarm_clear_level(i03t,type,AlarmLevel_2);
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
            }
        }
    } 
    
    if(value >= threshold->level_1_resume) {  
        if(i03t->alarm_msg[type][AlarmLevel_1].resume_delay < threshold->level_1_delay) {
            i03t->alarm_msg[type][AlarmLevel_1].resume_delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
            
            i03t->alarm_msg[type][AlarmLevel_0].status = AlarmReset;
            appl_alarm_clear_level(i03t,type,AlarmLevel_1);
            appl_alarm_clear_level(i03t,type,AlarmLevel_2);
            appl_alarm_clear_level(i03t,type,AlarmLevel_3);
        }
    } else if(value >= threshold->level_2_resume) {
        if(i03t->alarm_msg[type][AlarmLevel_2].resume_delay < threshold->level_2_delay) {
            i03t->alarm_msg[type][AlarmLevel_2].resume_delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
            
            appl_alarm_clear_level(i03t,type,AlarmLevel_2);
            appl_alarm_clear_level(i03t,type,AlarmLevel_3);
        }
    } else if(value >= threshold->level_3_resume) {
        if(i03t->alarm_msg[type][AlarmLevel_3].resume_delay < threshold->level_3_delay) {
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
            
            appl_alarm_clear_level(i03t,type,AlarmLevel_3);
        }   
    }
}


void appl_alarm_check_low(I03T_Info_t *i03t,AlarmType_t type,uint16_t value,ThresholdLevel_t *threshold) {
    if(i03t == NULL) {
        return;
    } 
    
    if(i03t->i03t_addr == 0 || i03t->i03t_addr > CONFIG_MAX_I03T) {
        return;
    }

    if(value <= threshold->level_3) {
        if(i03t->alarm_msg[type][AlarmLevel_3].delay < threshold->level_3_delay) {
            i03t->alarm_msg[type][AlarmLevel_3].status = AlarmReset;
            i03t->alarm_msg[type][AlarmLevel_3].delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_3].status = AlarmSet;
            appl_alarm_clear_level(i03t,type,AlarmLevel_2);
            appl_alarm_clear_level(i03t,type,AlarmLevel_1);
            appl_alarm_clear_level(i03t,type,AlarmLevel_0);
            
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
            
            
        }
    } else if(value <= threshold->level_2) {
        if(i03t->alarm_msg[type][AlarmLevel_3].status != AlarmSet) {
            if(i03t->alarm_msg[type][AlarmLevel_2].delay < threshold->level_2_delay) {
                i03t->alarm_msg[type][AlarmLevel_2].status = AlarmReset;
                i03t->alarm_msg[type][AlarmLevel_2].delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_2].status = AlarmSet;
                appl_alarm_clear_level(i03t,type,AlarmLevel_0);
                appl_alarm_clear_level(i03t,type,AlarmLevel_1);
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
                
                 i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
            }
        } else {
            if(value >= threshold->level_3_resume) {
                if(i03t->alarm_msg[type][AlarmLevel_2].delay < threshold->level_2_delay) {
                    i03t->alarm_msg[type][AlarmLevel_2].status = AlarmReset;
                    i03t->alarm_msg[type][AlarmLevel_2].delay++;
                } else {
                    i03t->alarm_msg[type][AlarmLevel_2].status = AlarmSet;
                    appl_alarm_clear_level(i03t,type,AlarmLevel_0);
                    appl_alarm_clear_level(i03t,type,AlarmLevel_1);
                    appl_alarm_clear_level(i03t,type,AlarmLevel_3);
                }
            }
        }
    } else if(value <= threshold->level_1) {
        if(i03t->alarm_msg[type][AlarmLevel_2].status != AlarmSet) { 
            if(i03t->alarm_msg[type][AlarmLevel_1].delay < threshold->level_1_delay) {
                i03t->alarm_msg[type][AlarmLevel_1].status = AlarmReset;
                i03t->alarm_msg[type][AlarmLevel_1].delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_1].status = AlarmSet;
                appl_alarm_clear_level(i03t,type,AlarmLevel_0);
                appl_alarm_clear_level(i03t,type,AlarmLevel_2);
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
                
                i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
            }
        } else {
            if(value >= threshold->level_2_resume) {
                if(i03t->alarm_msg[type][AlarmLevel_1].delay < threshold->level_1_delay) {
                    i03t->alarm_msg[type][AlarmLevel_1].status = AlarmReset;
                    i03t->alarm_msg[type][AlarmLevel_1].delay++;
                } else {
                    i03t->alarm_msg[type][AlarmLevel_1].status = AlarmSet;
                    appl_alarm_clear_level(i03t,type,AlarmLevel_0);
                    appl_alarm_clear_level(i03t,type,AlarmLevel_2);
                    appl_alarm_clear_level(i03t,type,AlarmLevel_3);
                }
            }
        }
    } 
    
    if(value >= threshold->level_1_resume) {  
        if(i03t->alarm_msg[type][AlarmLevel_1].resume_delay < threshold->level_1_delay) {
            i03t->alarm_msg[type][AlarmLevel_1].resume_delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
            
            i03t->alarm_msg[type][AlarmLevel_0].status = AlarmReset;
            appl_alarm_clear_level(i03t,type,AlarmLevel_1);
            appl_alarm_clear_level(i03t,type,AlarmLevel_2);
            appl_alarm_clear_level(i03t,type,AlarmLevel_3);
        }
    } else if(value >= threshold->level_2_resume) {
        if(i03t->alarm_msg[type][AlarmLevel_2].resume_delay < threshold->level_2_delay) {
            i03t->alarm_msg[type][AlarmLevel_2].resume_delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
            
            appl_alarm_clear_level(i03t,type,AlarmLevel_2);
            appl_alarm_clear_level(i03t,type,AlarmLevel_3);
        }
    } else if(value >= threshold->level_3_resume) {
        if(i03t->alarm_msg[type][AlarmLevel_3].resume_delay < threshold->level_3_delay) {
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
            i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
            
            appl_alarm_clear_level(i03t,type,AlarmLevel_3);
        }   
    }
}


void appl_alarm_check_over(I03T_Info_t *i03t,AlarmType_t type,uint16_t value,ThresholdLevel_t *threshold) {
    if(i03t == NULL) {
        return;
    } 
    
    if(i03t->i03t_addr == 0 || i03t->i03t_addr > CONFIG_MAX_I03T) {
        return;
    }

    if(value >= threshold->level_3) {
        if(i03t->alarm_msg[type][AlarmLevel_3].delay < threshold->level_3_delay) {
            i03t->alarm_msg[type][AlarmLevel_3].status = AlarmReset;
            i03t->alarm_msg[type][AlarmLevel_3].delay++;
        } else {
            i03t->alarm_msg[type][AlarmLevel_3].status = AlarmSet;
            appl_alarm_clear_level(i03t,type,AlarmLevel_2);
            appl_alarm_clear_level(i03t,type,AlarmLevel_1);
            appl_alarm_clear_level(i03t,type,AlarmLevel_0);
        }
    } else if(value >= threshold->level_2) {
        if(i03t->alarm_msg[type][AlarmLevel_3].status != AlarmSet) {
            if(i03t->alarm_msg[type][AlarmLevel_2].delay < threshold->level_2_delay) {
                i03t->alarm_msg[type][AlarmLevel_2].status = AlarmReset;
                i03t->alarm_msg[type][AlarmLevel_2].delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_2].status = AlarmSet;
                appl_alarm_clear_level(i03t,type,AlarmLevel_0);
                appl_alarm_clear_level(i03t,type,AlarmLevel_1);
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
            }
        }
    } else if(value >= threshold->level_1) {
        if(i03t->alarm_msg[type][AlarmLevel_2].status != AlarmSet) { 
            if(i03t->alarm_msg[type][AlarmLevel_1].delay < threshold->level_1_delay) {
                i03t->alarm_msg[type][AlarmLevel_1].status = AlarmReset;
                i03t->alarm_msg[type][AlarmLevel_1].delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_1].status = AlarmSet;
                appl_alarm_clear_level(i03t,type,AlarmLevel_0);
                appl_alarm_clear_level(i03t,type,AlarmLevel_2);
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
            }
        }
    } 
    
    if(value < threshold->level_1_resume) {
        if(i03t->alarm_msg[type][AlarmLevel_1].status != AlarmReset) {
            if(i03t->alarm_msg[type][AlarmLevel_1].resume_delay < threshold->level_1_delay) {
                i03t->alarm_msg[type][AlarmLevel_1].resume_delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
                
                i03t->alarm_msg[type][AlarmLevel_0].status = AlarmReset;
                appl_alarm_clear_level(i03t,type,AlarmLevel_1);
                appl_alarm_clear_level(i03t,type,AlarmLevel_2);
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
            }
        }
    } else if(value < threshold->level_2_resume) {
        if(i03t->alarm_msg[type][AlarmLevel_2].status != AlarmReset) {
            if(i03t->alarm_msg[type][AlarmLevel_2].resume_delay < threshold->level_2_delay) {
                i03t->alarm_msg[type][AlarmLevel_2].resume_delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
                
                appl_alarm_clear_level(i03t,type,AlarmLevel_2);
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
            }
        }
    } else if(value < threshold->level_3_resume) {
        if(i03t->alarm_msg[type][AlarmLevel_3].status != AlarmReset) {
            if(i03t->alarm_msg[type][AlarmLevel_3].resume_delay < threshold->level_3_delay) {
                i03t->alarm_msg[type][AlarmLevel_3].resume_delay++;
            } else {
                i03t->alarm_msg[type][AlarmLevel_0].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_1].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_2].resume_delay = 0;
                i03t->alarm_msg[type][AlarmLevel_3].resume_delay = 0;
                
                appl_alarm_clear_level(i03t,type,AlarmLevel_3);
            }   
        }

    }
}

osTimerId_t appl_alarm_id = NULL;

//uint16_t soc_debug = 1000;
void appl_alarm_check(void *parameter) {
 
    bool alarm_group_status = false;
    bool alarm_cell_status = false;
    bool alarm_comm_err = false;
    bool alarm_synch_err = false;
    
    for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR; i03t_addr++) {
        
        I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);

        if(i03t_node != NULL) {
            AlarmLevel_t level;
            uint16_t temp = 0;
            if(i03t_node->mount == 0) {
                if(i03t_node->discharge.status == CHARGE_STATE) {
                    appl_alarm_clear_level(i03t_node,AlarmTypeSocLow,AlarmLevel_0);
                    appl_alarm_clear_level(i03t_node,AlarmTypeSocLow,AlarmLevel_1);
                    appl_alarm_clear_level(i03t_node,AlarmTypeSocLow,AlarmLevel_2);
                    appl_alarm_clear_level(i03t_node,AlarmTypeSocLow,AlarmLevel_3);
                    level = AlarmLevel_0;
                } else {
                    
                    appl_alarm_check_low(i03t_node,AlarmTypeSocLow,i03t_node->discharge.soc,&pDEApp->device_config.i03t_nodes[i03t_addr-1].alarm_threshold.soc.low);
                    level = appl_alarm_get_status(i03t_node,AlarmTypeSocLow);
                    if(level > pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.alarm_max_level) {
                        level = (AlarmLevel_t)pDEApp->device_config.i03t_nodes[i03t_addr-1].sys_para.alarm_max_level;
                    }
                }
                
                
                temp = i03t_node->alarm.alarm.bat_group_alarm1;
                
                temp &=  ~(3u << I03T_GROUP1_ALARM_SOC_LOW_POS);
                    
                if(level) {
                    temp |= level << I03T_GROUP1_ALARM_SOC_LOW_POS;
                }  
                
                i03t_node->alarm.alarm.bat_group_alarm1 = temp;
                
                //temp = (i03t_node->alarm.alarm.bat_group_alarm2 & ~(1u << I03T_GROUP2_ALARM_COMM_ERR_POS));
            } 
            
            if(i03t_node->alarm.alarm.bat_group_alarm1 \
                || i03t_node->alarm.alarm.bat_group_alarm2\
                || i03t_node->alarm.alarm.bat_group_alarm3 ) {
                alarm_group_status = true;
            }

            if( i03t_node->flag.bits.cell_alarm) {
                alarm_cell_status = true;
            }
                
            if(i03t_node->flag.bits.i03t_comm_err) {
                alarm_comm_err = true;
            }
            
            //if(!i03t_node->flag.bits.i03t_comm_err) 
            {
                if(!i03t_node->flag.bits.synch_config || i03t_node->flag.bits.synch_sn_need) {
                    alarm_synch_err = true;
                }
            }
        }
    }
    
    pDEApp->Flag.bits.alarm_group_flag = alarm_group_status;
    pDEApp->Flag.bits.alarm_cell_flag = alarm_cell_status;
    pDEApp->Flag.bits.comm_err = alarm_comm_err;
    pDEApp->Flag.bits.synch_err = alarm_synch_err;
}

void appl_alarm_check_init(void) {
    const osTimerAttr_t timer_attr = {
        .name = "alarm_check",
    };
    
    appl_alarm_id = osTimerNew(appl_alarm_check,osTimerPeriodic,NULL,&timer_attr);
    osTimerStart(appl_alarm_id, 1000ul);

}
