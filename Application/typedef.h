#ifndef __TYPEDEF_H_
#define __TYPEDEF_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

#include "project_config.h"
 
#ifdef  __cplusplus
extern "C"
{
#endif

#define MODBUS_VOLT_CONV     1u//(10.0f)
#define MODBUS_CURRENT_CONV  1u//(10.0f)
#define MODBUS_SOC_CONV      1u//(10.0f)
#define MODBUS_SOH_CONV      1u//(10.0f)

#define multiple(x,y)      (x * y)
#define division(x,y)      (x / y)

#define limits(n,down,up)  ((n)>=(down) && (n) <= (up))

#define swap16(x)         ((((uint16_t)(x) & 0xff00) >> 8) | (((uint16_t)(x) & 0x00ff) << 8))

#define swap32(x)        ((((uint32_t)(x) & 0xff000000) >> 24) | \
                            (((uint32_t)(x) & 0x00ff0000) >> 8) |\
                            (((uint32_t)(x) & 0x0000ff00) << 8) |\
                            (((uint32_t)(x) & 0x000000ff) << 24))

typedef enum {
    
    COMM_TYPE_485_1 = 0,
    COMM_TYPE_485_2,
    COMM_TYPE_485_3,
    COMM_TYPE_485_4,
    
    COMM_TYPE_COM,
    COMM_TYPE_ETH,
    COMM_TYPE_USB,
}COMM_TYPE_t;


#define COMM_I03T_PORT   COMM_TYPE_485_4



typedef enum {
    errOK = 0,
    errErr,
    errFmtFilSys,

}errStatus_t;


typedef enum {
    SysCmdNone = 0,
    SysCmdReset,
    SysCmdSetBaudRate,
}SysCmd_t;

#define I03T_CREATE            1
#define I03T_DELETE            2
#define I03T_SOH_DEC           3
#define I03T_SOH_INIT          4

#define COM_DEBUG_RX_SIZE      (1600)

#define PROTOCOL_RX_SIZE       1600
#define PROTOCOL_TIME_OUT      (20)
#define PROTOCOL_RX_PULL_TIME  (5)
#define PROTOCOL_I03T_TIME_OUT (20)


#define PROTOCOL_TIME_OUT_1      (50)
#define PROTOCOL_RX_PULL_TIME_1  (10)


typedef struct {
    uint32_t tick;
    uint8_t  buffer[COM_DEBUG_RX_SIZE];
    uint16_t length;
} DebugMsg_t;

typedef struct {
    uint32_t tick;
    uint8_t  buffer[PROTOCOL_RX_SIZE];
    uint16_t length;
} ProtocolMsg_t;

typedef struct {
    uint8_t    *msg;
    uint16_t    length;
    COMM_TYPE_t comm_type;
} CommMsg_t;

typedef struct {
    SysCmd_t cmd;
    uint8_t *pdata;
    uint16_t length;
}SysMsgCmd_t;

typedef struct {
    COMM_TYPE_t commType;
    uint16_t    length;
    uint8_t    *pdata;
}ApplProto_t;

     
/* logger date */
typedef struct {
    uint32_t sec    : 6;
    uint32_t min    : 6;
    uint32_t hour   : 5;
    uint32_t day    : 5;
    uint32_t month  : 4;
    uint32_t year   : 6; 
} DATE_yymmddhhmmss_Bit;

typedef union {
    uint32_t        date;
    DATE_yymmddhhmmss_Bit bits;
} DATE_yymmddhhmmss_t;


typedef enum {
    StorageNone = 0,
    StorageParaInfor,
    StorageHistData,
    StorageAlarmData,
    StorageDischarge,
    StorageCharge,
    StorageFileIndex,

    StorageHistRead,
    StorageAlarmRead,
    StorageDischargeRead,
    
    StorageFileCheck,
    StorageFileDelete,
    StorageFileDeleteHist,
    StorageFileDeleteAlarm,
    StorageFileDeleteDischarge,
    StorageFileFlush,
    StorageFsFormat,
    
    StorageUdisk,
    StorageUDiskUpgrade,
    StorageUDiskExportAll,
    StorageUDiskExportHist,
    StorageUDiskExportAlarm,
    StorageUDiskExportDischarge,
    StorageUDiskExportSN,
    
    StorageSaveLogger,
    
    StorageParaFactory,
    
    StorageFileTest,
    
}StorageType_t;

typedef struct {
    StorageType_t storageType;
    uint8_t       i03t_addr;
    uint16_t      length;
    uint8_t      *pdata;
}StorageMsg_t;

typedef struct {
    uint16_t id;          //id
    uint16_t voltage;     //mV
    uint16_t inter_res;   //1u omu
    int16_t  temperature; //0.1 degree
}CellInfo_t;

typedef struct {
    DATE_yymmddhhmmss_t start;
    DATE_yymmddhhmmss_t end;
    COMM_TYPE_t         commType;
    uint8_t             requestMsgId;
    uint8_t             dest;
    uint8_t             print;
}LoggerQuery_t;

typedef enum  {
    IDLE_STATE = 0,
    CHARGE_STATE = 1,
    DISCHARGE_STATE,
    FLOAT_CHARGE_STATE,
} ChargeStatus_t;

typedef union {
    uint32_t u;
    int32_t  i;
    float    f;
}Convt_t;

#if 1
typedef struct {
    uint32_t       crc_chk;
    uint32_t       time;
    uint32_t       index;

    int16_t        voltage;
    int16_t        temp;
    int16_t        current[CONFIG_MAX_CURRENT_CH];
    
    uint16_t       soc;
    uint16_t       soh;
    uint16_t       available_time;
    uint8_t        status;
    uint8_t        spare8;
    uint16_t       spare16;
    float          deep_discharge_cycle;
    

}Discharge_t;

typedef struct {
    uint32_t       crc_chk;
    uint32_t       time;
    uint32_t       index;

    int16_t        voltage;
    int16_t        temp;
    int16_t        current;
    uint16_t       soc;
    uint16_t       soh;
    uint16_t       available_time;
}Charge_t;
#else

typedef struct {
    uint32_t       crc_chk;
    uint32_t       time;
    uint32_t       index;

    int16_t        voltage;
    int16_t        temp;
    
    uint8_t        status[CONFIG_MAX_CURRENT_CH];
    int16_t        current[CONFIG_MAX_CURRENT_CH];
    uint16_t       soc[CONFIG_MAX_CURRENT_CH];
    uint16_t       soh[CONFIG_MAX_CURRENT_CH];
    uint16_t       available_time[CONFIG_MAX_CURRENT_CH];
    float          deep_discharge_cycle[CONFIG_MAX_CURRENT_CH];
}Discharge_t;

typedef struct {
    uint32_t       crc_chk;
    uint32_t       time;
    uint32_t       index;

    int16_t        voltage;
    int16_t        temp;
    int16_t        current[CONFIG_MAX_CURRENT_CH];
    uint16_t       soc[CONFIG_MAX_CURRENT_CH];
    uint16_t       soh[CONFIG_MAX_CURRENT_CH];
    uint16_t       available_time[CONFIG_MAX_CURRENT_CH];
}Charge_t;
#endif



typedef struct {
    Discharge_t     discharge;
    uint8_t         dry_node_status;
}TotalInfo_t;

typedef struct {
    uint32_t index_hist;
    uint32_t index_alarm;
    uint32_t index_discharge;
    uint32_t index_charge;
    DATE_yymmddhhmmss_t date;
    uint16_t soc;
    uint16_t soh;
    float    deep_discharge_cycle;
}IndexInfo_t;

typedef struct {
    uint32_t    crc_chk;
    uint32_t    index;
    uint32_t    logger_index;
    IndexInfo_t index_info[CONFIG_MAX_I03T];
}FileIndexStore_t;

typedef struct {
    int16_t   max;
    int16_t   min;
    int16_t   average;
    uint16_t  max_id;
    uint16_t  min_id;
    uint16_t  diff;
}PeakInfo_t;


typedef struct {
    uint32_t crc_check;
    uint32_t index;
    uint32_t count;
    uint32_t max_size;
}FileHead_t;

typedef struct {
	int16_t critical_current;  		       //非充非放状态电流范围
	int16_t fast_to_float;				   //快充转浮充   0.1A/bit
	int16_t float_to_fast;				   //浮充转快充
	int16_t charge_to_discharge;		   //充电转放电
}ChargeStatusThreshold_t;

typedef struct {
	uint16_t period_total_volt;             //总压电流采集周期
    uint16_t period_cell_poll;              //采集盒轮询周期
    uint16_t period_int_res;                //内阻采样周期
    uint16_t spare;
}SamplePeriod_t;

/*
Bit0:电池组过压告警等级
Bit1:电池组欠压告警等级
Bit2:电池组充电过流告警等级
Bit3:电池组放电过流告警等级
Bit4:环境温度高告警
Bit5:电阻组SOC低告警
Bit6：总压采集故障
Bit7：电流1采集故障
Bit8：电流2采集故障
Bit9：电流3采集故障
Bit10：电流4采集故障
Bit11：温度采集故障
Bit12：从设备通讯故障
Bit13~BIT15 预留
*/
typedef struct {
    volatile uint16_t over_volt       : 1;
    volatile uint16_t low_volt        : 1;
    volatile uint16_t over_charge     : 1;
    volatile uint16_t over_discharge  : 1;
    volatile uint16_t over_temp       : 1;
    volatile uint16_t low_soc         : 1;
    volatile uint16_t total_v_sample  : 1;
    volatile uint16_t current1_sample : 1;
    volatile uint16_t current2_sample : 1;
    volatile uint16_t current3_sample : 1;
    volatile uint16_t current4_sample : 1;
    volatile uint16_t temp_sample     : 1;
    volatile uint16_t comm_err        : 1;
    volatile uint16_t leak            : 1;
    volatile uint16_t spare           : 2;
}AlarmCorrelationGroupField_t;

typedef union {
    uint16_t                     all;
    AlarmCorrelationGroupField_t bit;
}AlarmCorrelationGroup_t;


typedef struct {
    volatile uint16_t over_volt   : 1;
    volatile uint16_t low_volt    : 1;
    volatile uint16_t over_temp   : 1;
    volatile uint16_t over_res    : 1;
    volatile uint16_t comm_err    : 1;
    volatile uint16_t volt_sample : 1;
    volatile uint16_t temp_sample : 1;
    volatile uint16_t spare       : 9;
}AlarmCorrelationCellsField_t;

typedef union {
    uint16_t all;
    AlarmCorrelationCellsField_t bit;
}AlarmCorrelationCells_t;


typedef struct {
    uint16_t enable       : 1;
    uint16_t relationship : 1;
    uint16_t spare        : 14;
}AlarmCorrelationField_t;

typedef union {
    uint16_t                all;
    AlarmCorrelationField_t bit;
}AlarmCorrelation_t;

typedef struct {
    uint16_t alarm_correlation;
    uint16_t alarm_correlation_group;
    uint16_t alarm_correlation_cells;
    uint16_t spare;
}AlarmCorrelationSettings_t;

typedef struct {
	uint8_t  dry_switch;
    uint8_t  spare; 
	uint16_t out_threshold;
	uint16_t output_resume_threshold;
	uint16_t delay;
	uint16_t output_min_volt;
    AlarmCorrelationSettings_t alarm_correlation;
}DryNodeInfo_t;


typedef struct {
	uint16_t work_freq;
	uint16_t assist_freq;
}Wireless433Infor_t;

typedef struct {
	uint16_t nominal_cap;             //1=0.1AH
	uint16_t nominal_volt;            //1-2V 3-6V 6-12V
	uint16_t float_volt;              //1=0.001V
    uint16_t temp_compensate;         //1=0.001mV
    uint8_t  discharge_method;        //
    uint8_t  charge_method;
    uint16_t charge_efficiency;       //
    uint16_t max_discharge_cap;       //1=0.1AH
    uint16_t cell_min_voltage;
    uint16_t cell_max_voltage;
    int16_t discharge_max_current;
    int16_t discharge_min_current;
    int16_t  min_calculate_temp;
    int16_t  max_calculate_temp;
    uint8_t  cap_init;
    uint8_t  current_group_number;
    uint32_t spare;
}CapPara_t;



typedef struct {
    int16_t level_1; //轻微
    int16_t level_2; //一般
    int16_t level_3; //严重
    int16_t level_1_resume; //轻微
    int16_t level_2_resume; //一般
    int16_t level_3_resume; //严重
	uint16_t level_1_delay; //轻微
    uint16_t level_2_delay; //一般
    uint16_t level_3_delay; //严重
    uint16_t spare;
}ThresholdLevel_t;

typedef struct {
	ThresholdLevel_t over;
	ThresholdLevel_t low;
}ThresholdVoltage_t;

typedef struct {
	ThresholdLevel_t over_charge;
	ThresholdLevel_t over_discharge;
}ThresholdCurrent_t;

typedef struct {
	ThresholdLevel_t over;
}ThresholdTemp_t;

typedef struct {
	ThresholdLevel_t over;
}ThresholdRes_t;


typedef struct {
	ThresholdLevel_t low;
}ThresholdSOC_t;

typedef struct {
    ThresholdLevel_t leak;
}ThresholdLeak_t;

typedef struct {
	ThresholdVoltage_t voltage;
	ThresholdTemp_t    temp;
	ThresholdRes_t     res;
    
}ThresholdCell_t;

typedef struct {
	ThresholdVoltage_t total_voltage;    
	ThresholdCurrent_t total_current;
    ThresholdSOC_t     soc;
	ThresholdTemp_t    temp;
	ThresholdCell_t    cell;
    ThresholdLeak_t    leak;
}AlarmThreshold_t;

typedef struct {
    uint16_t over_volt   : 2;
    uint16_t low_volt    : 2;
    uint16_t over_temp   : 2;
    uint16_t over_res    : 2;
    uint16_t comm_err    : 1;
    uint16_t volt_sample : 1;
    uint16_t temp_sample : 1;
    uint16_t spare       : 5;
}AlarmCellBit_t;

typedef union {
    AlarmCellBit_t bit;
    uint16_t       all;
}AlarmCell_t;


typedef struct {
    uint16_t over_volt      : 2;
    uint16_t low_volt       : 2;
    uint16_t over_temp      : 2;
    uint16_t low_soc        : 2;
    uint16_t spare          : 8;
}AlarmGroup1Bit_t;

typedef union {
    AlarmGroup1Bit_t bit;
    uint16_t         all;
}AlarmGroup1_t;

typedef struct {
    uint16_t total_volt_sample : 1;
    uint16_t current1_sample   : 1;
    uint16_t current2_sample   : 1;
    uint16_t current3_sample   : 1;
    uint16_t current4_sample   : 1;
    uint16_t temp_sample       : 1;
    uint16_t comm_err          : 1;
    uint16_t leak              : 1;
    uint16_t spare             : 8;
}AlarmGroup2Bit_t;

typedef union {
    AlarmGroup2Bit_t bit;
    uint16_t         all;
}AlarmGroup2_t;

typedef struct {
    uint16_t current1_over_charge    : 2;
    uint16_t current1_over_discharge : 2;
    uint16_t current2_over_charge    : 2;
    uint16_t current2_over_discharge : 2;
    uint16_t current3_over_charge    : 2;
    uint16_t current3_over_discharge : 2;
    uint16_t current4_over_charge    : 2;
    uint16_t current4_over_discharge : 2;
}AlarmGroup3Bit_t;

typedef union {
    uint16_t all;
    AlarmGroup3Bit_t bit;
}AlarmGroup3_t;


typedef struct {
    uint16_t    bat_group_alarm1;
    uint16_t    bat_group_alarm2;
    uint16_t    bat_group_alarm3;
    uint16_t    cell_alarm[CONFIG_MAX_CELL];
}AlarmInfo_t;


typedef enum {
    AlarmTypeSocLow = 0,
#if 0
    AlarmTypeVoltageOver = 0,
    AlarmTypeVoltageLow,
    AlarmTypeChargeCurrentOver,
    AlarmTypeDischargeCurrentOver,
    AlarmTypeTempOver,
    
    
    AlarmTypeVoltageSampleErr,
    AlarmTypeCurrent1SampleErr,
    AlarmTypeCurrent2SampleErr,
    AlarmTypeCurrent3SampleErr,
    AlarmTypeCurrent4SampleErr,
    AlarmTypeTempSampleErr,
    ///AlarmTypeCommErr,
#endif
    
    AlarmTypeMax,
}AlarmType_t;


typedef enum {
    AlarmReset = 0,
    AlarmSet,
}AlarmStatus_t;

typedef enum {
    AlarmLevel_0 = 0,
    AlarmLevel_1,
    AlarmLevel_2,
    AlarmLevel_3,
    AlarmLevel_Max,
}AlarmLevel_t;

typedef struct {
    AlarmStatus_t  status;
    uint16_t       delay;
    uint16_t       resume_delay;
}AlarmMsg_t;

typedef struct {
	double k;
	double b;
}KB_t;

typedef struct {
	KB_t    kb;
	int16_t zero_offset;
}Calib_t;

typedef struct {
	Calib_t total_voltage;
	Calib_t current[CONFIG_MAX_CURRENT_CH];
	Calib_t temp;
	Calib_t voltage_5v;
}CalibPara_t;


typedef struct {
    uint8_t  sn_length;
	uint8_t  voltage_level;
	uint8_t  current_level;
	uint8_t  alarm_max_level;
	uint8_t  cell_mode;
    uint8_t  spare1;
    uint16_t cell_number;                     //采集盒个数
    uint8_t  protocol_type;
    uint8_t  spare2;
    uint16_t spare16;
    uint32_t spare32;
}SystemPara_t;

typedef struct {
    uint32_t  crc_check;
    uint16_t  i03t_addr;
    uint16_t  cell_id;
    uint8_t   sn[CONFIG_SN_LENGTH];
    uint8_t   cloud_id[CONFIG_CLOUD_ID_LENGTH];
    uint8_t   cell_on_current_group;
}SNStore_t;

typedef struct {
    uint8_t                  i03t_addr;                       //地址
    uint8_t                  mount;                           //挂载 
    uint8_t                  bat_life;                        //bat life years
    uint8_t                  spare8;                          //spare
    uint32_t                 spare32;
    uint8_t                  cloud_id[CONFIG_CLOUD_ID_LENGTH];//cloud id
}I03T_BasePara_t;

typedef struct {
    I03T_BasePara_t          base_para;                       //Base Info
    SystemPara_t             sys_para;                        //系统参数
	SamplePeriod_t           sample_period;                   //采样周期
    ChargeStatusThreshold_t  charge_status_threshold;         //充放电转换
	DryNodeInfo_t            dry_node;                        //干接点
	Wireless433Infor_t       wireless;                        //无线
    CapPara_t                cap;                             //容量
    AlarmThreshold_t         alarm_threshold;                 //阈值
    CalibPara_t              calib;                           //校准参数
}I03T_Para_t;

#define BAUDRATE_4800     0
#define BAUDRATE_9600     1
#define BAUDRATE_19200    2
#define BAUDRATE_38400    3
#define BAUDRATE_57600    4
#define BAUDRATE_115200   5

#define DATA_BITS_8       0
#define DATA_BITS_9       1

#define CHECK_BIT_NONE    0
#define CHECK_BIT_ODD     1
#define CHECK_BIT_EVEN    2

#define STOP_BITS_1       0
#define STOP_BITS_2       1

typedef struct {
    /*
    0:4800,1:9600,2:19200,3:38400,4:57600,5:115200
    */
    uint32_t baudrate   : 4;
    /* 0:8,1:9*/
    uint8_t  data_bits  : 2;
    /*
    0:None,1:Odd,2:Even
    */
    uint32_t check_bits : 2;
    /* 0:1,1:2*/
    uint32_t stop_bits  : 2;
    uint32_t spare      : 22;
}__BaudRate_t;

typedef union {
    volatile __BaudRate_t bits;
    volatile uint32_t     all;
}BaudRate_t;


typedef struct {
    uint8_t   spare;
    uint8_t   cloud_enable;
    uint16_t  mqtt_port;
    uint16_t  cells_report_interval;
    uint16_t  i03t_report_interval;
    uint8_t   user_name[64];
    uint8_t   pass_word[32];
    uint8_t   mqtt_addr[64];
    //uint8_t   topic[64];
}MqttConfig_t;

typedef struct {
    uint8_t ip[16];
    uint8_t mask[16];
    uint8_t gate[16];
}LocalIpV4Setting_t;

typedef struct {
    LocalIpV4Setting_t ipv4;
    uint8_t            dns[40];
}NetConfig_t;

typedef struct {
    uint8_t    port_type;
    uint8_t    protocol;
    BaudRate_t baudrate;
}RS485CFG_t;

typedef struct {
    //BaudRate_t      baudrate;
    RS485CFG_t      rs485_cfg[CONFIG_MAX_485_PORT];
    uint16_t        storage_period;
    uint16_t        cell_numbers;
    uint8_t         i03m_addr;
    uint8_t         i03t_time_synch;
    uint8_t         i03t_number;
    uint8_t         module_type;
    uint8_t         protocol_type;
    uint8_t         spare8;
    uint16_t        spare16;
    DryNodeInfo_t   dry_node; 
    MqttConfig_t    mqtt_config; 
    NetConfig_t     net_config;
    uint32_t        spare32;
             
}I03M_Para_t;

typedef struct {
    uint32_t        check_value;
    I03M_Para_t     i03m;
    I03T_Para_t     i03t_nodes[CONFIG_MAX_I03T];
}DeviceConfig_t;

typedef struct {
    uint32_t synch_err          :  1;
    uint32_t alarm_group_flag   :  1;
    uint32_t alarm_cell_flag    :  1;
    uint32_t comm_err           :  1;
    uint32_t sys_init_finish    :  1;
    uint32_t query_logger_flag  :  1;
    uint32_t sys_busy           :  1;
    uint32_t usb_busy           :  1;
    uint32_t print_flag         :  1;
    uint32_t usb_con            :  1;
    uint32_t soc_trace          :  1;
    uint32_t g4_trace           :  1;
}__SysFlag_t;

typedef union {
    volatile __SysFlag_t bits;
    volatile uint32_t    all;
}SysFlag_t;

typedef struct {
    uint32_t sn_synch_time;
    uint16_t sn_cnt;
    uint16_t sn_err_cnt;
}SNRequest_t;

typedef enum {
    ACTION_TYPE_NONE = 0,
    ACTION_TYPE_REGISTER,
    ACTION_TYPE_TX_MSG1,
    ACTION_TYPE_TX_MSG2,
    ACTION_TYPE_TX_MSG3,
    
} ACTION_TYPE_t;

typedef struct {
    ACTION_TYPE_t   action;
    uint8_t         tryTimes;
	COMM_TYPE_t     commType;
    uint16_t        length;
    uint8_t        *pdata;
} LinkerMsg_t;


typedef struct {
    DeviceConfig_t      device_config;
    AlarmInfo_t         alarm[CONFIG_MAX_I03T];
    FileIndexStore_t    index;
    SysFlag_t           Flag;
    DATE_yymmddhhmmss_t now;
}DEApp_t;

typedef struct {
    uint32_t check;
    uint32_t magic;
    uint8_t  file_test_err_cnt;
    uint8_t  cpu_stat;
    uint16_t usb_init_err_cnt;
    uint32_t file_err_cnt;
    uint32_t cloud_connect_err_cnt;
    uint32_t reset_cnt;

    
}ApplNoInit_t;

typedef struct {
    uint8_t  Tindex;
}TPageNature_t;

typedef struct {
    uint8_t  Tindex;
    uint16_t CellPageNum;
}CellPageNature_t;

typedef struct {
    uint8_t  Tindex;
    uint16_t AlarmPageNum;
}AlarmPageNature_t;

typedef struct {
    uint8_t             CurCusor;
    uint8_t             NvbFlag;
    uint8_t             NvbMutex;
    uint8_t             OldTNumber;
    TPageNature_t       TPageNature;
    CellPageNature_t    CellPageNature;
    AlarmPageNature_t  AlarmPageNature;
}NvbNature_t;

extern DEApp_t *pDEApp;
extern DebugMsg_t comm_debug_msg;
extern ProtocolMsg_t protocolRxMsg;
extern ProtocolMsg_t RxMsg485_1;
extern ProtocolMsg_t RxMsg485_2;
extern ProtocolMsg_t RxMsg485_3;

#ifdef  __cplusplus
}
#endif

#endif

