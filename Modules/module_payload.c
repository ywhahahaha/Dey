#include <stdint.h>
#include <time.h>

#include "main.h"
#include "module_payload.h"
#include "cJSON.h"
#include "module_4g.h"
#include "module_sn.h"

/*
v1/gateway/telemetry
{
    "c428e635b5ac464f9f182061c13afada": 
    [
        {
            "ts": 1644025767526,
            "values": 
            {
                "TolVolt": "5161.22",
                "Soc":1000,
                "TolCur":-1.7,
                "Status":3,
                
                "TolTs":1644025767526,
                "TolVer":"V1.0.7",
                
                "CellVer":"21",
                "CellTs":1644025767526,
                
                "DebugDetail":"1",
                "DebugTs":1644025767526,
                "DebugType":4
            }
        }
    ],
*/

/*
c:1
d:2
f:3
*/
char *module_payload_make_i03t(uint8_t i03t_addr) {
    cJSON *root = NULL;
    cJSON *array = NULL;
    cJSON *key = NULL;
    cJSON *values = NULL;
    char *pJsonBytes = NULL;
    
    I03T_Info_t *i03t_node = i03t_node_find(i03t_addr);
    if(i03t_node == NULL) {
        return NULL;
    } 
    
    if(i03t_node->mount || pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id[0] == '\0') {
        return NULL;
    }
    
    
    root = cJSON_CreateObject();
    if(root == NULL) {
        goto EXIT;
    }

    //for(uint8_t i03t_addr = 1;i03t_addr <= CONFIG_MAX_I03T; i03t_addr++) 
    {
        //if(i03t_node != NULL && !i03t_node->mount && pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id[0] != '\0') 
        {
            
            uint8_t cloud_id[CONFIG_CLOUD_ID_LENGTH+1] = {0};
            memcpy(cloud_id,pDEApp->device_config.i03t_nodes[i03t_addr-1].base_para.cloud_id,CONFIG_CLOUD_ID_LENGTH);
            array = cJSON_CreateArray(); if (array == NULL) {goto EXIT;}
            cJSON_AddItemToObject(root, (const char *)cloud_id, array);
            
            key = cJSON_CreateObject(); if (key == NULL) {goto EXIT;}
            cJSON_AddItemToArray(array, key);
            
            uint64_t ms;
            struct tm time1;     
            
            /*
                sample time.
            */
            /*
            4G Module signal.
            */
            time1.tm_year = pDEApp->now.bits.year + 2000 - 1900;
            time1.tm_mon = pDEApp->now.bits.month - 1;
            time1.tm_mday = pDEApp->now.bits.day;
            time1.tm_hour = pDEApp->now.bits.hour;
            time1.tm_min = pDEApp->now.bits.min;
            time1.tm_sec = pDEApp->now.bits.sec;
            time1.tm_isdst = 0;
            uint64_t sec = (uint64_t)mktime(&time1) - 8 * 3600ul;
            ms = (uint64_t)sec * (uint64_t)1000;   
            
            cJSON_AddNumberToObject(key, "ts", ms); //
            values = cJSON_CreateObject();if (values == NULL) {goto EXIT;}
            cJSON_AddItemToObject(key, "values", values);
            
            char _str[12] = {0};

#ifdef MODULE_4G_DEBUG_DATA
            i03t_node->discharge.voltage = rand();
            i03t_node->discharge.current[0] = rand();
            i03t_node->discharge.soc = rand()%100;
#endif
            
            cJSON_AddNumberToObject(values, "TolVolt", i03t_node->discharge.voltage);
            cJSON_AddNumberToObject(values, "TolCur",  i03t_node->discharge.current[0]);
            cJSON_AddNumberToObject(values, "Soc", i03t_node->discharge.soc);
            cJSON_AddNumberToObject(values, "Status", (i03t_node->discharge.status == 0) ? 3 : i03t_node->discharge.status);
            
            cJSON_AddNumberToObject(values, "CellTs", ms);
            sprintf(_str, "%d", pModule4GInfor->rssi);
            cJSON_AddStringToObject(values, "CellVer", _str); 
            

            time1.tm_year = 2022 - 1900;
            time1.tm_mon = 1;
            time1.tm_mday = 1;
            time1.tm_hour = I03M_SOFT_VERSION_X;
            time1.tm_min = I03M_SOFT_VERSION_Y;
            time1.tm_sec = I03M_SOFT_VERSION_Z;
            
            time1.tm_isdst = 0;
            sec = (uint64_t)mktime(&time1) - 8 * 3600ul;
            ms = (uint64_t)sec * (uint64_t)1000; 
            cJSON_AddNumberToObject(values, "TolTs", ms);
            sprintf(_str, "V%d.%d.%d", VERSION_X(i03t_node->soft_version),
                                       VERSION_Y(i03t_node->soft_version),
                                       VERSION_Z(i03t_node->soft_version));
            cJSON_AddStringToObject(values, "TolVer", _str);
            

            time1.tm_year = i03t_node->sample_time.bits.year + 2000 - 1900;
            time1.tm_mon = i03t_node->sample_time.bits.month - 1;
            time1.tm_mday = i03t_node->sample_time.bits.day;
            time1.tm_hour = i03t_node->sample_time.bits.hour;
            time1.tm_min = i03t_node->sample_time.bits.min;
            time1.tm_sec = i03t_node->sample_time.bits.sec;
            time1.tm_isdst = 0;
            sec = (uint64_t)mktime(&time1) - 8 * 3600ul;
            ms = (uint64_t)sec * (uint64_t)1000;    
            cJSON_AddNumberToObject(values, "DebugTs", ms);
            cJSON_AddNumberToObject(values, "DebugType", 4);
            sprintf(_str, "%d", pDEApp->device_config.i03t_nodes[i03t_addr-1].wireless.work_freq);  
            cJSON_AddStringToObject(values, "DebugDetail", _str);

        } 
    }
    
    pJsonBytes = cJSON_PrintUnformatted(root);
    
EXIT:
    if(root) {
        cJSON_Delete(root);
    }
    return pJsonBytes;
}

/*
    "a357a93bb2e04161a74ecdcdd7587415":
    [
        {
            "ts":1636618423339,
            "values":
            {
                "Volt":2002,
                "Res":0,
                "Temp":229
            }
        }
    ]
*/

char *module_payload_make_cells(I03T_Info_t *i03t_node,uint16_t cell_id_start,uint16_t num) {

    cJSON *root = NULL;
    cJSON *array = NULL;
    cJSON *key = NULL;
    cJSON *values = NULL;
    char *pJsonBytes = NULL;
    
    uint16_t temp_cell_id;

    uint8_t count = 0;
    
    root = cJSON_CreateObject();
    if(root == NULL) {
        goto EXIT;
    }


    temp_cell_id = cell_id_start;
    
    for(uint16_t i=0;i<num && temp_cell_id <= CONFIG_MAX_CELL;i++,temp_cell_id++) {
        AlarmCell_t cell_alarm = {0};
        cell_alarm.all = i03t_node->alarm.alarm.cell_alarm[temp_cell_id-1];
        
        if(i03t_node->hist.cells[temp_cell_id-1].voltage == 0 ||\
           i03t_node->flag.bits.i03t_comm_err ) {
#ifndef MODULE_4G_DEBUG_DATA
            continue;
#endif
        }   
        SNStore_t *sn = module_sn_get_by_index(i03t_node->i03t_addr,temp_cell_id);
        if(sn == NULL) {
            break;
        }
        
        if(sn->cloud_id[0] == 0x00) {
            sys_free(sn);
            continue;
        }
        
        uint8_t cloud_id[CONFIG_CLOUD_ID_LENGTH+1] = {0};
        memcpy(cloud_id,sn->cloud_id,CONFIG_CLOUD_ID_LENGTH);
        array = cJSON_CreateArray(); if (array == NULL) {goto EXIT;}
        cJSON_AddItemToObject(root, (const char *)cloud_id, array);
        
        key = cJSON_CreateObject(); if (key == NULL) {goto EXIT;}
        cJSON_AddItemToArray(array, key);
        
        uint64_t ms;
        struct tm time1;   
 
        time1.tm_year = i03t_node->sample_time.bits.year + 2000 - 1900;
        time1.tm_mon = i03t_node->sample_time.bits.month - 1;
        time1.tm_mday = i03t_node->sample_time.bits.day;
        time1.tm_hour = i03t_node->sample_time.bits.hour;
        time1.tm_min = i03t_node->sample_time.bits.min;
        time1.tm_sec = i03t_node->sample_time.bits.sec;
        time1.tm_isdst = 0;
        
        uint64_t sec = (uint64_t)mktime(&time1) - 8 * 3600ul;
        ms = (uint64_t)sec * (uint64_t)1000;            
        cJSON_AddNumberToObject(key, "ts", ms); //
        
        values = cJSON_CreateObject();if (values == NULL) {goto EXIT;}
        cJSON_AddItemToObject(key, "values", values);
        
#ifdef MODULE_4G_DEBUG_DATA
        i03t_node->hist.cells[temp_cell_id-1].voltage =  i03t_node->i03t_addr*1000 + temp_cell_id;
        i03t_node->hist.cells[temp_cell_id-1].inter_res = i03t_node->sample_time.bits.hour;
        i03t_node->hist.cells[temp_cell_id-1].temperature = i03t_node->sample_time.bits.min;
#endif

        cJSON_AddNumberToObject(values, "Volt", i03t_node->hist.cells[temp_cell_id-1].voltage);
        cJSON_AddNumberToObject(values, "Res",  i03t_node->hist.cells[temp_cell_id-1].inter_res);
        cJSON_AddNumberToObject(values, "Temp", i03t_node->hist.cells[temp_cell_id-1].temperature);
        //indicate alarm.
        cJSON_AddNumberToObject(values, "Alarm", cell_alarm.all);
        
        sys_free(sn);
        
        count++;
    }
    
    if(count > 0) {
        pJsonBytes = cJSON_PrintUnformatted(root);
    }
    
    
EXIT:
    if(root) {
        cJSON_Delete(root);
    }
    return pJsonBytes;
}
