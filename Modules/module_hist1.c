#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "cmsis_os2.h"
#include "rl_fs.h"
#include "main.h"
#include "module_hist.h"
#include "crc_check.h"
#include "i03t_list.h"
#include "de_protocol.h"

void storage_msg_put(uint8_t i03t_addr,uint8_t *pdata,uint16_t length,StorageType_t storage_type);
errStatus_t module_hist_store(I03T_Info_t *i03t) {
    FILE *fp;
    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));
    
    uint16_t write_index = (i03t->hist.index % FILE_HIST_DATA_MAX_CNT);
    uint32_t write_addr = write_index * sizeof(CellHistStorage_t);
    
    sprintf(path,"hist%d.txt",i03t->i03t_addr);
    fp = fopen(path,"r+");
    if(fp == NULL) {
        sys_free(cell);
        storage_msg_put(i03t->i03t_addr,NULL,0,StorageFileCheck);
        return errErr;
    }
    
    memcpy(cell,&i03t->hist,sizeof(CellHistStorage_t));
 
    fseek(fp,write_addr,SEEK_SET); 

    Bsp_RtcGetTime((DATE_yymmddhhmmss_t *)&cell->time);    
  
    cell->crc_chk = CRC_Get32((uint8_t *)(cell) + 4,sizeof(CellHistStorage_t) - 4);
    
    size_t size = fwrite(cell,sizeof(CellHistStorage_t),1,fp);
  
    fclose(fp);
    
    i03t->hist.index++;
    
    sys_free(cell);

    return (size == 1) ? errOK : errErr;

}
void debug_printf(const char *fmt, ...);
errStatus_t module_hist_load(I03T_Info_t *i03t) {
    FILE *fp;
    
    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"hist%d.txt",i03t->i03t_addr);
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(CellHistStorage_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(cell);
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 
    
    uint32_t tick = osKernelGetTickCount() + 250;
    uint8_t ff_cnt = 0;
    uint8_t find = false;
    while(1) {
        
        feed_watchdog(); 
        
        memset(cell,0,sizeof(CellHistStorage_t));
        
        size_t size = fread(cell,sizeof(CellHistStorage_t),1,fp);
        
        if(size == 1 ) {
            uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(CellHistStorage_t) - 4);
            if(check == cell->crc_chk ) {
                if(cell->index > i03t->hist.index) {
                    memcpy(&i03t->hist,cell,sizeof(CellHistStorage_t));
                }
                find = true;
                ff_cnt = 0;
            } else {
                if(cell->crc_chk == 0xffffffff) {
                    ff_cnt++;
                    if(ff_cnt > 3) {
                        break;
                    }
                }
            }
        } else {
            break;
        }
        
        if(osKernelGetTickCount() > tick) {
            tick = osKernelGetTickCount() + 250;
            osDelay(5);
        }
    }
   
    fclose(fp);   

    sys_free(cell);
    
    if(find) {
        i03t->hist.index++;
    }
    
    return find ? errOK : errErr;

}



int32_t module_hist_get_index(FILE *fp,uint32_t file_pos,CellHistStorage_t *cell) {
    
    memset(cell,0,sizeof(CellHistStorage_t));

	if(fseek(fp,file_pos,SEEK_SET)) {
		return -1;
	}
        
    size_t size = fread(cell,sizeof(CellHistStorage_t),1,fp);
    
    if(size == 1 ) {
        uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(CellHistStorage_t) - 4);
        if(check == cell->crc_chk ) {
            return cell->index;
        } 
    }
    
    return -1;
}


errStatus_t module_hist_index_check(I03T_Info_t *i03t) {
    FILE *fp;

    char path[16];
    
    if(i03t == NULL) {
        return errErr;
    }
    
    if(i03t->hist.index == 0) {
        return errErr;
    }

    uint16_t _index = ((i03t->hist.index-1) % FILE_HIST_DATA_MAX_CNT);
    int32_t file_pos = _index * sizeof(CellHistStorage_t);
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"hist%d.txt",i03t->i03t_addr);
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(CellHistStorage_t));

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(cell);
        return errErr;
    }
    
    int32_t index = module_hist_get_index(fp,file_pos,cell);
   
    fclose(fp);   

    sys_free(cell);
    
    if(index < 0) {
        return errErr;
    }

    return i03t->hist.index >= index ? errOK : errErr;

}

errStatus_t module_hist_load_protocol(uint8_t i03t_addr,LoggerQuery_t *query) {
    FILE *fp;
    
    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"hist%d.txt",i03t_addr);
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(CellHistStorage_t));
    
    uint8_t *proto_tx_buf = sys_malloc(PROTOCOL_RX_MAX_SIZE);
    if(proto_tx_buf == NULL) {
        sys_free(cell);
        return errErr;
    }
    
    uint8_t *p = sys_malloc(PROTOCOL_RX_MAX_SIZE);
    if(p == NULL) {
        sys_free(cell);
        sys_free(proto_tx_buf);
        return errErr;
    }
    
    fp = fopen(path,"r");
    if(fp == NULL) {
        sys_free(cell);
        sys_free(p);
        sys_free(proto_tx_buf);
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 
    
    uint32_t tick = osKernelGetTickCount() + 250;
    
    uint8_t find = false;
    uint16_t index = 0;
    uint8_t ff_cnt = 0;
    while(1) {
        
        feed_watchdog(); 
        
        memset(cell,0,sizeof(CellHistStorage_t));
        
        size_t size = fread(cell,sizeof(CellHistStorage_t),1,fp);
        
        if(size == 1 ) {
            uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(CellHistStorage_t) - 4);
            if(check == cell->crc_chk ) {
                if(query->print){
                    DATE_yymmddhhmmss_t date;
                    date.date = cell->time;
                    debug_printf("%04d-%02d-%02d %02d:%02d:%02d Volt:%05d,Res:%05d,Temp:%04d,SOC:%04d,SOH:%04d\r\n",\
                                                        date.bits.year + 2000,date.bits.month,date.bits.day,\
                                                        date.bits.hour,date.bits.min,date.bits.sec,\
                                                        cell->total_info.discharge.voltage,\
                                                        cell->total_info.discharge.current,\
                                                        cell->total_info.discharge.temp,\
                                                        cell->total_info.discharge.soc,\
                                                        cell->total_info.discharge.soh);
                    for(uint16_t i = 0;i<CONFIG_MAX_CELL;i++) {
                        debug_printf("Cell<%d> Volt:%05d,Res:%05d,Temp:%04d\r\n",i+1,\
                                                        cell->cells[i].voltage,\
                                                        cell->cells[i].inter_res,\
                                                        cell->cells[i].temperature);
                    }
                }  else { 
                    DATE_yymmddhhmmss_t date;
                    date.date = cell->time;
                    index = 0;
                    p[index++] = 0x00;
                    p[index++] = i03t_addr;
                
                    p[index++] = DataId_Hist_Data;
                
                    p[index++] = date.bits.year;
                    p[index++] = date.bits.month;
                    p[index++] = date.bits.day;
                    p[index++] = date.bits.hour;
                    p[index++] = date.bits.min;
                    p[index++] = date.bits.sec;
                
                    p[index++] = cell->total_info.discharge.voltage >> 8;
                    p[index++] = cell->total_info.discharge.voltage;
                
                    p[index++] = cell->total_info.discharge.current >> 8;
                    p[index++] = cell->total_info.discharge.current;
                
                    p[index++] = cell->total_info.discharge.temp >> 8;
                    p[index++] = cell->total_info.discharge.temp;
                    
                    p[index++] = cell->total_info.discharge.soc >> 8;
                    p[index++] = cell->total_info.discharge.soc;
                    
                    p[index++] = cell->total_info.discharge.soh >> 8;
                    p[index++] = cell->total_info.discharge.soh;
                    
                    p[index++] = cell->total_info.discharge.available_time >> 8;
                    p[index++] = cell->total_info.discharge.available_time;
                    
                    p[index++] = cell->total_info.discharge.status;
                    p[index++] = cell->total_info.dry_node_status;
                    
                    for(uint16_t i=0;i<CONFIG_MAX_CELL;i++) {
                        p[index++] = cell->cells[i].voltage >> 8;
                        p[index++] = cell->cells[i].voltage;
                        
                        p[index++] = cell->cells[i].inter_res >> 8;
                        p[index++] = cell->cells[i].inter_res;
                        
                        p[index++] = cell->cells[i].temperature >> 8;
                        p[index++] = cell->cells[i].temperature;
                    }
                    
                    uint8_t ser_flg = find ? 0 : SER_SOF;
                    
                    uint16_t send_length = de_protocol_slave_package_fill(proto_tx_buf,
                                            CTRL_Response,
                                            pDEApp->device_config.i03m.i03m_addr,
                                            query->dest,
                                            ser_flg |(query->requestMsgId & 0x3F),
                                            TAG_MASK_Logger,
                                            p,
                                            index);
                    
                    protocol_send_bytes(query->commType,proto_tx_buf,send_length);
                    osDelay(10);
                }
                
                ff_cnt = 0;
                find = true;
            } else {
                if(cell->crc_chk == 0xffffffff) {
                    ff_cnt++;
                    if(ff_cnt > 5) {
                        break;
                    }
                }
            }
        } else {
            break;
        }
    }
   
    fclose(fp); 

    if(!query->print) {
        uint16_t send_length = de_protocol_slave_package_fill(proto_tx_buf,
                                                    CTRL_Response,
                                                    pDEApp->device_config.i03m.i03m_addr,
                                                    query->dest,
                                                    SER_EOF |(query->requestMsgId & 0x3F),
                                                    TAG_MASK_Logger,
                                                    NULL,
                                                    0);
                            
        protocol_send_bytes(query->commType,proto_tx_buf,send_length);
    }

    sys_free(cell);
    sys_free(p);
    sys_free(proto_tx_buf);
    
    return errOK;
}


#if 0
errStatus_t module_hist_load_bs(I03T_Info_t *i03t) {
    FILE *fp;
    
    char path[16];
    
    memset(path,0,sizeof(path));
    
    sprintf(path,"hist%d.txt",i03t->i03t_addr);
    
    fsFileInfo info;
    info.fileID = 0; 
    if(ffind (path, &info) != fsOK) {
        return errErr;
    }
    
    CellHistStorage_t *cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        return errErr;
    }
    memset(cell,0,sizeof(CellHistStorage_t));
    
    
	int index1;
	int index2;
	int index3;

	int file_pos1 = 0;
	int file_pos2 = sizeof(CellHistStorage_t) / 2 * sizeof(CellHistStorage_t);
	int file_pos3 = (info.size / sizeof(CellHistStorage_t) - 1) * sizeof(CellHistStorage_t);

    
    fp = fopen(path,"r");
    if(fp == NULL) {
        return errErr;
    }
    fseek(fp,0,SEEK_SET); 

	int temp_index;
	while(1) {
		temp_index = module_hist_find_index_big(fp, cell, file_pos1, file_pos2);
		if(temp_index > 0) {
			index1 = temp_index;
		} else {
			if(temp_index == -1) {
				index1 = index2;
			} 

			index1 = index1;
		}

		temp_index = module_hist_find_index_big(fp, cell, file_pos2, file_pos3);
		if(temp_index > 0) {
			index2 = temp_index;
		} else {
			if(temp_index == -1) {
				index2 = index3;
			} 
		}
		
	}


   
    fclose(fp);   

    sys_free(cell);
    
    return errOK;

}
#endif


//const char *hist_head_str = "时间戳,总电压,总电流,温度,SOC,SOH,状态,可用时间,单体1电压,单体1内阻,单体1温度,单体2电压,单体2内阻,单体2温度,单体3电压,单体3内阻,单体3温度,单体4电压,单体4内阻,单体4温度,单体5电压,单体5内阻,单体5温度,单体6电压,单体6内阻,单体6温度,单体7电压,单体7内阻,单体7温度,单体8电压,单体8内阻,单体8温度,单体9电压,单体9内阻,单体9温度,单体10电压,单体10内阻,单体10温度,单体11电压,单体11内阻,单体11温度,单体12电压,单体12内阻,单体12温度,单体13电压,单体13内阻,单体13温度,单体14电压,单体14内阻,单体14温度,单体15电压,单体15内阻,单体15温度,单体16电压,单体16内阻,单体16温度,单体17电压,单体17内阻,单体17温度,单体18电压,单体18内阻,单体18温度,单体19电压,单体19内阻,单体19温度,单体20电压,单体20内阻,单体20温度,单体21电压,单体21内阻,单体21温度,单体22电压,单体22内阻,单体22温度,单体23电压,单体23内阻,单体23温度,单体24电压,单体24内阻,单体24温度,单体25电压,单体25内阻,单体25温度,单体26电压,单体26内阻,单体26温度,单体27电压,单体27内阻,单体27温度,单体28电压,单体28内阻,单体28温度,单体29电压,单体29内阻,单体29温度,单体30电压,单体30内阻,单体30温度,单体31电压,单体31内阻,单体31温度,单体32电压,单体32内阻,单体32温度,单体33电压,单体33内阻,单体33温度,单体34电压,单体34内阻,单体34温度,单体35电压,单体35内阻,单体35温度,单体36电压,单体36内阻,单体36温度,单体37电压,单体37内阻,单体37温度,单体38电压,单体38内阻,单体38温度,单体39电压,单体39内阻,单体39温度,单体40电压,单体40内阻,单体40温度,单体41电压,单体41内阻,单体41温度,单体42电压,单体42内阻,单体42温度,单体43电压,单体43内阻,单体43温度,单体44电压,单体44内阻,单体44温度,单体45电压,单体45内阻,单体45温度,单体46电压,单体46内阻,单体46温度,单体47电压,单体47内阻,单体47温度,单体48电压,单体48内阻,单体48温度,单体49电压,单体49内阻,单体49温度,单体50电压,单体50内阻,单体50温度,单体51电压,单体51内阻,单体51温度,单体52电压,单体52内阻,单体52温度,单体53电压,单体53内阻,单体53温度,单体54电压,单体54内阻,单体54温度,单体55电压,单体55内阻,单体55温度,单体56电压,单体56内阻,单体56温度,单体57电压,单体57内阻,单体57温度,单体58电压,单体58内阻,单体58温度,单体59电压,单体59内阻,单体59温度,单体60电压,单体60内阻,单体60温度,单体61电压,单体61内阻,单体61温度,单体62电压,单体62内阻,单体62温度,单体63电压,单体63内阻,单体63温度,单体64电压,单体64内阻,单体64温度,单体65电压,单体65内阻,单体65温度,单体66电压,单体66内阻,单体66温度,单体67电压,单体67内阻,单体67温度,单体68电压,单体68内阻,单体68温度,单体69电压,单体69内阻,单体69温度,单体70电压,单体70内阻,单体70温度,单体71电压,单体71内阻,单体71温度,单体72电压,单体72内阻,单体72温度,单体73电压,单体73内阻,单体73温度,单体74电压,单体74内阻,单体74温度,单体75电压,单体75内阻,单体75温度,单体76电压,单体76内阻,单体76温度,单体77电压,单体77内阻,单体77温度,单体78电压,单体78内阻,单体78温度,单体79电压,单体79内阻,单体79温度,单体80电压,单体80内阻,单体80温度,单体81电压,单体81内阻,单体81温度,单体82电压,单体82内阻,单体82温度,单体83电压,单体83内阻,单体83温度,单体84电压,单体84内阻,单体84温度,单体85电压,单体85内阻,单体85温度,单体86电压,单体86内阻,单体86温度,单体87电压,单体87内阻,单体87温度,单体88电压,单体88内阻,单体88温度,单体89电压,单体89内阻,单体89温度,单体90电压,单体90内阻,单体90温度,单体91电压,单体91内阻,单体91温度,单体92电压,单体92内阻,单体92温度,单体93电压,单体93内阻,单体93温度,单体94电压,单体94内阻,单体94温度,单体95电压,单体95内阻,单体95温度,单体96电压,单体96内阻,单体96温度,单体97电压,单体97内阻,单体97温度,单体98电压,单体98内阻,单体98温度,单体99电压,单体99内阻,单体99温度,单体100电压,单体100内阻,单体100温度,单体101电压,单体101内阻,单体101温度,单体102电压,单体102内阻,单体102温度,单体103电压,单体103内阻,单体103温度,单体104电压,单体104内阻,单体104温度,单体105电压,单体105内阻,单体105温度,单体106电压,单体106内阻,单体106温度,单体107电压,单体107内阻,单体107温度,单体108电压,单体108内阻,单体108温度,单体109电压,单体109内阻,单体109温度,单体110电压,单体110内阻,单体110温度,单体111电压,单体111内阻,单体111温度,单体112电压,单体112内阻,单体112温度,单体113电压,单体113内阻,单体113温度,单体114电压,单体114内阻,单体114温度,单体115电压,单体115内阻,单体115温度,单体116电压,单体116内阻,单体116温度,单体117电压,单体117内阻,单体117温度,单体118电压,单体118内阻,单体118温度,单体119电压,单体119内阻,单体119温度,单体120电压,单体120内阻,单体120温度,单体121电压,单体121内阻,单体121温度,单体122电压,单体122内阻,单体122温度,单体123电压,单体123内阻,单体123温度,单体124电压,单体124内阻,单体124温度,单体125电压,单体125内阻,单体125温度,单体126电压,单体126内阻,单体126温度,单体127电压,单体127内阻,单体127温度,单体128电压,单体128内阻,单体128温度,单体129电压,单体129内阻,单体129温度,单体130电压,单体130内阻,单体130温度,单体131电压,单体131内阻,单体131温度,单体132电压,单体132内阻,单体132温度,单体133电压,单体133内阻,单体133温度,单体134电压,单体134内阻,单体134温度,单体135电压,单体135内阻,单体135温度,单体136电压,单体136内阻,单体136温度,单体137电压,单体137内阻,单体137温度,单体138电压,单体138内阻,单体138温度,单体139电压,单体139内阻,单体139温度,单体140电压,单体140内阻,单体140温度,单体141电压,单体141内阻,单体141温度,单体142电压,单体142内阻,单体142温度,单体143电压,单体143内阻,单体143温度,单体144电压,单体144内阻,单体144温度,单体145电压,单体145内阻,单体145温度,单体146电压,单体146内阻,单体146温度,单体147电压,单体147内阻,单体147温度,单体148电压,单体148内阻,单体148温度,单体149电压,单体149内阻,单体149温度,单体150电压,单体150内阻,单体150温度,单体151电压,单体151内阻,单体151温度,单体152电压,单体152内阻,单体152温度,单体153电压,单体153内阻,单体153温度,单体154电压,单体154内阻,单体154温度,单体155电压,单体155内阻,单体155温度,单体156电压,单体156内阻,单体156温度,单体157电压,单体157内阻,单体157温度,单体158电压,单体158内阻,单体158温度,单体159电压,单体159内阻,单体159温度,单体160电压,单体160内阻,单体160温度,单体161电压,单体161内阻,单体161温度,单体162电压,单体162内阻,单体162温度,单体163电压,单体163内阻,单体163温度,单体164电压,单体164内阻,单体164温度,单体165电压,单体165内阻,单体165温度,单体166电压,单体166内阻,单体166温度,单体167电压,单体167内阻,单体167温度,单体168电压,单体168内阻,单体168温度,单体169电压,单体169内阻,单体169温度,单体170电压,单体170内阻,单体170温度,单体171电压,单体171内阻,单体171温度,单体172电压,单体172内阻,单体172温度,单体173电压,单体173内阻,单体173温度,单体174电压,单体174内阻,单体174温度,单体175电压,单体175内阻,单体175温度,单体176电压,单体176内阻,单体176温度,单体177电压,单体177内阻,单体177温度,单体178电压,单体178内阻,单体178温度,单体179电压,单体179内阻,单体179温度,单体180电压,单体180内阻,单体180温度,单体181电压,单体181内阻,单体181温度,单体182电压,单体182内阻,单体182温度,单体183电压,单体183内阻,单体183温度,单体184电压,单体184内阻,单体184温度,单体185电压,单体185内阻,单体185温度,单体186电压,单体186内阻,单体186温度,单体187电压,单体187内阻,单体187温度,单体188电压,单体188内阻,单体188温度,单体189电压,单体189内阻,单体189温度,单体190电压,单体190内阻,单体190温度,单体191电压,单体191内阻,单体191温度,单体192电压,单体192内阻,单体192温度,单体193电压,单体193内阻,单体193温度,单体194电压,单体194内阻,单体194温度,单体195电压,单体195内阻,单体195温度,单体196电压,单体196内阻,单体196温度,单体197电压,单体197内阻,单体197温度,单体198电压,单体198内阻,单体198温度,单体199电压,单体199内阻,单体199温度,单体200电压,单体200内阻,单体200温度,单体201电压,单体201内阻,单体201温度,单体202电压,单体202内阻,单体202温度,单体203电压,单体203内阻,单体203温度,单体204电压,单体204内阻,单体204温度,单体205电压,单体205内阻,单体205温度,单体206电压,单体206内阻,单体206温度,单体207电压,单体207内阻,单体207温度,单体208电压,单体208内阻,单体208温度,单体209电压,单体209内阻,单体209温度,单体210电压,单体210内阻,单体210温度,单体211电压,单体211内阻,单体211温度,单体212电压,单体212内阻,单体212温度,单体213电压,单体213内阻,单体213温度,单体214电压,单体214内阻,单体214温度,单体215电压,单体215内阻,单体215温度,单体216电压,单体216内阻,单体216温度,单体217电压,单体217内阻,单体217温度,单体218电压,单体218内阻,单体218温度,单体219电压,单体219内阻,单体219温度,单体220电压,单体220内阻,单体220温度,单体221电压,单体221内阻,单体221温度,单体222电压,单体222内阻,单体222温度,单体223电压,单体223内阻,单体223温度,单体224电压,单体224内阻,单体224温度,单体225电压,单体225内阻,单体225温度,单体226电压,单体226内阻,单体226温度,单体227电压,单体227内阻,单体227温度,单体228电压,单体228内阻,单体228温度,单体229电压,单体229内阻,单体229温度,单体230电压,单体230内阻,单体230温度,单体231电压,单体231内阻,单体231温度,单体232电压,单体232内阻,单体232温度,单体233电压,单体233内阻,单体233温度,单体234电压,单体234内阻,单体234温度,单体235电压,单体235内阻,单体235温度,单体236电压,单体236内阻,单体236温度,单体237电压,单体237内阻,单体237温度,单体238电压,单体238内阻,单体238温度,单体239电压,单体239内阻,单体239温度,单体240电压,单体240内阻,单体240温度,单体241电压,单体241内阻,单体241温度,单体242电压,单体242内阻,单体242温度,单体243电压,单体243内阻,单体243温度,单体244电压,单体244内阻,单体244温度,单体245电压,单体245内阻,单体245温度,单体246电压,单体246内阻,单体246温度,单体247电压,单体247内阻,单体247温度,单体248电压,单体248内阻,单体248温度,单体249电压,单体249内阻,单体249温度,单体250电压,单体250内阻,单体250温度,单体251电压,单体251内阻,单体251温度,单体252电压,单体252内阻,单体252温度,单体253电压,单体253内阻,单体253温度,单体254电压,单体254内阻,单体254温度,单体255电压,单体255内阻,单体255温度,单体256电压,单体256内阻,单体256温度,单体257电压,单体257内阻,单体257温度,单体258电压,单体258内阻,单体258温度,单体259电压,单体259内阻,单体259温度,单体260电压,单体260内阻,单体260温度,单体261电压,单体261内阻,单体261温度,单体262电压,单体262内阻,单体262温度,单体263电压,单体263内阻,单体263温度,单体264电压,单体264内阻,单体264温度,单体265电压,单体265内阻,单体265温度,单体266电压,单体266内阻,单体266温度,单体267电压,单体267内阻,单体267温度,单体268电压,单体268内阻,单体268温度,单体269电压,单体269内阻,单体269温度,单体270电压,单体270内阻,单体270温度,单体271电压,单体271内阻,单体271温度,单体272电压,单体272内阻,单体272温度,单体273电压,单体273内阻,单体273温度,单体274电压,单体274内阻,单体274温度,单体275电压,单体275内阻,单体275温度,单体276电压,单体276内阻,单体276温度,单体277电压,单体277内阻,单体277温度,单体278电压,单体278内阻,单体278温度,单体279电压,单体279内阻,单体279温度,单体280电压,单体280内阻,单体280温度,单体281电压,单体281内阻,单体281温度,单体282电压,单体282内阻,单体282温度,单体283电压,单体283内阻,单体283温度,单体284电压,单体284内阻,单体284温度,单体285电压,单体285内阻,单体285温度,单体286电压,单体286内阻,单体286温度,单体287电压,单体287内阻,单体287温度,单体288电压,单体288内阻,单体288温度,单体289电压,单体289内阻,单体289温度,单体290电压,单体290内阻,单体290温度,单体291电压,单体291内阻,单体291温度,单体292电压,单体292内阻,单体292温度,单体293电压,单体293内阻,单体293温度,单体294电压,单体294内阻,单体294温度,单体295电压,单体295内阻,单体295温度,单体296电压,单体296内阻,单体296温度,单体297电压,单体297内阻,单体297温度,单体298电压,单体298内阻,单体298温度,单体299电压,单体299内阻,单体299温度,单体300电压,单体300内阻,单体300温度,\r\n";
const char *hist_head_str1 = "时间戳,总电压,总电流,温度,SOC,SOH,状态,可用时间,";


errStatus_t module_hist_copy(void) {
    
    #define HIST_LOG_SIZE 6000
   
    FILE *f_dest;
    FILE *f_src;
    
    char path_dest[64];
    char path_src[24];
    
    I03T_Info_t *i03t = NULL;
    CellHistStorage_t *cell = NULL;
    
    char *write_buf = sys_malloc(HIST_LOG_SIZE);
    cell = sys_malloc(sizeof(CellHistStorage_t));
    if(cell == NULL) {
        goto EXIT;
    }
    memset(cell,0,sizeof(CellHistStorage_t));    
    
    if(write_buf == NULL) {
        goto EXIT;
    }

    for(uint8_t i03t_addr=1;i03t_addr<=CONFIG_MAX_I03T;i03t_addr++) {
        
        i03t = i03t_node_find(i03t_addr);
        
        uint32_t tick = osKernelGetTickCount() + 250;
        uint8_t ff_cnt = 0;
        uint16_t index = 0;
        
        if(i03t != NULL) {
            DATE_yymmddhhmmss_t time;
            memset(path_src,0,sizeof(path_src));
            sprintf(path_src,"N0:hist%d.txt",i03t->i03t_addr);
            
            memset(path_dest,0,sizeof(path_dest));
            Bsp_RtcGetTime(&time);
#if 0
            sprintf(path_dest,DRVIER_UPATH"hist%d_%d_%04d-%02d-%02d %02d.%02d.%02d.txt",i03t->i03t_addr,\
                                                            pDEApp->device_config.i03m.i03m_modbus_addr,\
                                                            time.bits.year + 2000,\
                                                            time.bits.month,\
                                                            time.bits.day,\
                                                            time.bits.hour,\
                                                            time.bits.min,\
                                                            time.bits.sec);
#endif
            FILE     *f;        
             f = fopen ("U0:Test.txt", "w");    // Open/create file for writing
                if (f == NULL) {
                    continue;                     // Handle file opening/creation failure
                }
                
            //strcpy(path_dest,"U0:abc.csv"); 
            strcpy(path_dest,"U0:abc.txt"); 
            f_dest = fopen("U0:abc.txt","w");
            if(f_dest == NULL) {
                goto EXIT;
            }
            
            f_src = fopen(path_src,"r");
            if(f_src == NULL) {
                goto EXIT;
            }
            fseek(f_src,0,SEEK_SET); 
            
           
            
            
            //write header.
            fwrite(hist_head_str1,strlen(hist_head_str1),1,f_dest);
            index = 0;
            memset(write_buf,0,HIST_LOG_SIZE);
            for(uint16_t i=0;i<pDEApp->device_config.i03t_nodes[i03t_addr-1].cell_number;i++) {
                sprintf(write_buf + index,"单体%d电压,单体%d内阻,单体%d温度,",i+1,i+1,i+1);
                index += strlen(write_buf);
            }         
            write_buf[index++] = 0x0d;
            write_buf[index++] = 0x0a;
            fwrite(write_buf,index,1,f_dest);
            
            
            while(1) {
        
                feed_watchdog(); 
                
                memset(cell,0,sizeof(CellHistStorage_t));
                
                size_t size = fread(cell,sizeof(CellHistStorage_t),1,f_src);
                
                if(size == 1 ) {
                    uint32_t check = CRC_Get32(((uint8_t *)(cell) + 4),sizeof(CellHistStorage_t) - 4);
                    if(check == cell->crc_chk ) {
                        index = 0;
                        memset(write_buf,0,HIST_LOG_SIZE);

                        time.date = cell->time;
                        sprintf(write_buf + index,"%04d-%02d-%02d %02d:%02d:%02d,",time.bits.year + 2000,\
                                                                                time.bits.month,\
                                                                                time.bits.day,\
                                                                                time.bits.hour,\
                                                                                time.bits.min,\
                                                                                time.bits.sec);
                        index += strlen(write_buf);
                        
                        //volt,current,temp,soc,soh,status
                        sprintf(write_buf + index,"%d,%d,%d,%d,%d,%d,%d",cell->total_info.discharge.voltage,\
                                                                 cell->total_info.discharge.current,\
                                                                 cell->total_info.discharge.temp,\
                                                                 cell->total_info.discharge.soc,\
                                                                 cell->total_info.discharge.soh,\
                                                                 cell->total_info.discharge.status,
                                                                 cell->total_info.discharge.available_time);
                        index += strlen(write_buf);
                        
                        
                        for(uint16_t i=0;i<pDEApp->device_config.i03t_nodes[i03t_addr-1].cell_number;i++) {
                            sprintf(write_buf + index,"%d,%d,%d,",cell->cells[i].voltage,\
                                                                  cell->cells[i].inter_res,\
                                                                  cell->cells[i].temperature);
                            index += strlen(write_buf);
                        }
                        
                        write_buf[index++] = 0x0d;
                        write_buf[index++] = 0x0a;
                        
                        size_t size = fwrite(write_buf,index,1,f_dest);
                        if(size != 1) {
                            goto EXIT;
                        }
                        
                        ff_cnt = 0;
                        
                    } else {
                        if(cell->crc_chk == 0xffffffff) {
                            ff_cnt++;
                            if(ff_cnt > 3) {
                                break;
                            }
                        }
                    }
                } else {
                    break;
                }
                
                if(osKernelGetTickCount() > tick) {
                    tick = osKernelGetTickCount() + 250;
                    osDelay(5);
                }
            }
            
            if(f_dest != NULL) {
                fclose(f_dest);
                f_dest = NULL;
            }
            if(f_src != NULL) {
                fclose(f_src);
                f_src = NULL;
            }
        }
    }

EXIT:
    if(f_dest != NULL) {
        fclose(f_dest);
    }
    if(f_src != NULL) {
        fclose(f_src);
    }
    
    if(write_buf != NULL) {
        sys_free(write_buf);
    }
    
    if(cell != NULL) {
        sys_free(cell);
    }
    
    return errOK;
}

