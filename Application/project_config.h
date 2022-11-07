#ifndef __PROJECT_CONFIG_H_
#define __PROJECT_CONFIG_H_

#include <stdint.h>
#include <string.h>
 
#define I03M_SOFT_VERSION_X  1u  
#define I03M_SOFT_VERSION_Y  3u
#define I03M_SOFT_VERSION_Z  0u

#define I03M_HARD_VERSION_X  2u  
#define I03M_HARD_VERSION_Y  1u
#define I03M_HARD_VERSION_Z  1u
 
#define COMPILE_DATE         __DATE__
#define COMPILE_TIME         __TIME__

#define I03M_SOFT_VERSION  (I03M_SOFT_VERSION_X << 12 | I03M_SOFT_VERSION_Y << 4 | I03M_SOFT_VERSION_Z)
#define I03M_HARD_VERSION  (I03M_HARD_VERSION_X << 12 | I03M_HARD_VERSION_Y << 4 | I03M_HARD_VERSION_Z)

#define VERSION_X(x)  (x >> 12)
#define VERSION_Y(x)  ((x & 0x0FF0) >> 4)
#define VERSION_Z(x)  (x & 0x0F)

#define CONFIG_MAX_CELL               300u //collector box size
#define CONFIG_MAX_I03T               6u   //
#define CONFIG_SN_LENGTH              20u  //
#define CONFIG_CLOUD_ID_LENGTH        32u
#define CONFIG_MAX_IO3T_MODBUS_ADDR   6u
#define CONFIG_MAX_CURRENT_CH         4u

#define CONFIG_MAX_485_PORT           4u

#define CONFIG_DEEP_DISCHARGE_CYCLE   1500u

#define CONFIG_MAX_TOTAL_CELL         (CONFIG_MAX_CELL * CONFIG_MAX_I03T) 

#define DRVIER_NAME                "N0:"
#define DRVIER_UNAME               "U0:"

/*
#define DRVIER_UPGRADE_PATH        "U0:DEY\\UPGRADE\\"
#define DRVIER_LOGGER_PATH         "U0:DEY\\LOGGER\\"
#define DRVIER_I03M_DEV_INFO_PATH  "U0:DEY\\PARA\\i03m_dev_info.csv"
#define DRVIER_I03M_TOKEN_PATH     "U0:DEY\\PARA\\i03m_token.csv"
#define DRVIER_ACCESS_PATH         "U0:DEY\\DEY.ini"
#define DRVIER_ACCESS_PARA_PATH    "U0:DEY\\I03M_PARA.ini"
*/
#define DRVIER_UPGRADE_PATH        "U0:\\"
#define DRVIER_LOGGER_PATH         "U0:\\"
#define DRVIER_I03M_DEV_INFO_PATH  "U0:\\device.csv"
#define DRVIER_I03M_TOKEN_PATH     "U0:\\token.csv"
#define DRVIER_ACCESS_PATH         "U0:\\DEY.ini"
#define DRVIER_ACCESS_PARA_PATH    "U0:\\PARA.ini"


#define FILE_HIST_DATA_1          "hist1.txt"
#define FILE_HIST_DATA_2          "hist2.txt"
#define FILE_HIST_DATA_3          "hist3.txt"
#define FILE_HIST_DATA_4          "hist4.txt"
#define FILE_HIST_DATA_5          "hist5.txt"
#define FILE_HIST_DATA_6          "hist6.txt"

#define FILE_ALARM_DATA_1         "alarm1.txt"
#define FILE_ALARM_DATA_2         "alarm2.txt"  
#define FILE_ALARM_DATA_3         "alarm3.txt"  
#define FILE_ALARM_DATA_4         "alarm4.txt"  
#define FILE_ALARM_DATA_5         "alarm5.txt"  
#define FILE_ALARM_DATA_6         "alarm6.txt"  

#define FILE_DISCHARGE_ENABLE     1
#define FILE_DISCHARGE_STORE_TIME (60000u) //1 min.
 

#define FILE_DISCHARGE_DATA_1     "dis1.txt"
#define FILE_DISCHARGE_DATA_2     "dis2.txt"
#define FILE_DISCHARGE_DATA_3     "dis3.txt"
#define FILE_DISCHARGE_DATA_4     "dis4.txt"
#define FILE_DISCHARGE_DATA_5     "dis5.txt"
#define FILE_DISCHARGE_DATA_6     "dis6.txt"

#define FILE_CHARGE_DATA_1     "charge1.txt"
#define FILE_CHARGE_DATA_2     "charge2.txt"
#define FILE_CHARGE_DATA_3     "charge3.txt"
#define FILE_CHARGE_DATA_4     "charge4.txt"
#define FILE_CHARGE_DATA_5     "charge5.txt"
#define FILE_CHARGE_DATA_6     "charge6.txt"

#define FILE_LOGGER_PATH          "logger.log"

#define FILE_SN                   "sn"
#define SN_SYNCH_DELAY            15000

#define FILE_LOGGER_MAX_CNT         (10000ul)
#define FILE_HIST_DATA_MAX_CNT      (10000ul)
#define FILE_ALARM_DATA_MAX_CNT     (10000ul)
#define FILE_DISCHARGE_DATA_MAX_CNT (20000ul) 
#define FILE_SN_MAX_CNT             (300ul)

#define FILE_DELAY                  (100u)
#define FILL_FILE                    0
#define FILE_DELETE                  0
#define FILE_CHECKERROR              0

#define CONFIG_MAX_FILES             64

typedef enum {
    PROTOCOL_TYPE_DE_MODBUS = 0,
    PROTOCOL_TYPE_SIFANG,
    PROTOCOL_TYPE_DE,
    PROTOCOL_TYPE_MAX,
}PROTOCOL_TYPE_t;

typedef enum {
    PORT_ETH = 5,
    PORT_4G,
    PORT_485,
    PORT_MAX,
} PORT_TYPE_t;

typedef enum {
    MODULE_TYPE_NONE = 0,
    MODULE_4G_SELF,
    MODULE_4G_TAS,
    MODULE_4G_ZLG,
    MODULE_ETH_TAS,
    MODULE_4G_TYPE_MAX,
}MODULE_4G_TYPE_t;

#define MODULE_4G_RESET_TIME     (10u * 60 * 1000ul)
#define MODULE_4G_CELL_TX_CYCLE  (5u * 1000)
#define MODULE_4G_I03T_TX_CYCLE  (60u * 1000)


#define MQTT_PUB_TOPIC_1           "v1/gateway/telemetry"

//#define MODULE_4G_DEBUG_DATA       1
//#define CONFIG_KB_ENABLE           0

#define OS_RTX5        0
#define OS_FREERTOS    1
//#define OS_TYPE        OS_FREERTOS






#endif
