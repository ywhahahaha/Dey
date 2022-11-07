#ifndef __DE_PROTOCOL_H_
#define __DE_PROTOCOL_H_

#ifdef __cplusplus
 extern "C" {
#endif
     
#include <stdint.h>
#include "typedef.h"


#define PROTOCOL_RX_MAX_SIZE            2048
#define PROTOCOL_COMMU_RESP_TIME_OUT    500
#define PROTOCOL_COMMU_RESP_TIME_OUT_2  800
#define PROTOCOL_TRY_TIMES              3

#define  SER_EOF           (1 << 7)
#define  SER_SOF           (1 << 6)
#define  SER_SINGLE        (SER_EOF | SER_SOF)
#define  SER_MIDDLE        0x00

#define  CTRL_MustAck      (1 << 7)
     
#define  CTRL_Response     (1 << 6)
#define  CTRL_Request      (0 << 6)

#define  CTRL_WriteAck     (1 << 5)  
#define  CTRL_Encryption   (1 << 4)




typedef enum {
	TAG_NONE         = 0x00,
	TAG_Query        = 0x02,
	TAG_Control      = 0x03,
	TAG_Config       = 0x04,
	TAG_Status       = 0x05,
    TAG_Logger       = 0x06,
    TAG_Upgrade      = 0x0E,
    TAG_TransferData = 0x0F,
	
    
}TAG_t;

#define TAG_MASK_Query            (1 << TAG_Query)
#define TAG_MASK_Control          (1 << TAG_Control)
#define TAG_MASK_Config           (1 << TAG_Config)
#define TAG_MASK_Status           (1 << TAG_Status)
#define TAG_MASK_Logger           (1 << TAG_Logger)
#define TAG_MASK_Upgrade          (1 << TAG_Upgrade)
#define TAG_MASK_Transfer         (1 << TAG_TransferData)

typedef enum {

	QueryControl = 3,
	QueryConfig = 4,
	QueryStatus = 5,
}DataId_Query_t;

typedef enum {

	DataId_Upgrade_Header = 1,
	DataId_Upgrade_FileData,


}DataId_Upgrade_t;

typedef enum {
    DataId_Alarm_Header = 1,
    DataId_Alarm_Data,
    
    DataId_Hist_Header = 4,
    DataId_Hist_Data = 5,
    
    DataId_Discharge_Header = 7,
    DataId_Discharge_Data = 8,

}DataId_Logger_t;


typedef enum  { 
    ConfigDataId_BaudRate = 5,
    ConfigDataId_TotalVoltageSamplePeriod = 6,//0x06	总压电流采集周期	2	1S/bit
    ConfigDataId_CellPullPeriod,              //0x07	采集盒轮询周期	2	1S/bit
    ConfigDataId_ResSamplePeriod,             //0x08	内阻采样周期	2	0.1H/bit
    ConfigDataId_CellNumber,                  //0x09	采集盒个数	2	1个/bit
    ConfigDataId_CriticalCurrentLimits,       //0x0A	非充非放状态电流范围	2	0.1A/bit
    ConfigDataId_FastToFloat,                 //0x0B	快充转浮充	2	0.1A/bit
    ConfigDataId_FloatToFast,                 //0x0C	浮充转快充	2	0.1A/bit
    ConfigDataId_ChargeToDischarge,           //0x0D	充电转放电	2	0.1A/bit
    ConfigDataId_DrySwitch,                   //0x0E	干节点信号输出开关	1	0：关闭1：开启
    ConfigDataId_DryOutputThreshold,          //0x0F	干节点信号输出阈值	2	0.1V/bit
    ConfigDataId_DryResumeThreshold,          //0x10	干节点信号恢复阈值	2	0.1V/bit
    ConfigDataId_DryDelay,                    //0x11	干节点信号延时时间	2	1s/bit
    ConfigDataId_DryOutputMinVoltage,         //0x12	干节点信号输出最小电压值	2	0.1V/bit
    ConfigDataId_433WorkFreq,                 //0x13	工作路433的频段	1	
    ConfigDataId_433AssistFreq,               //0x14	辅助路433的频段	1	
    ConfigDataId_Protocol,                    //0x15    protocol
    ConfigDataId_PortCfg,                     //0x16    port cfg

    ConfigDataId_NominalCap = 0x30,           //0x30	标称容量	2	1=0.1AH
    ConfigDataId_NominalVolt,                 //0x31	标称电压	2	1-2V 3-6V 6-12V
    ConfigDataId_FloatVolt,                   //0x32	浮充电压	2	1=0.001V
    ConfigDataId_TempCompensate,              //0x33	温度补偿电压	2	1=0.001mV
    ConfigDataId_DischargeMethod,             //0x34	放电方法	1	0-安时放电法 1-深度学习法
    ConfigDataId_ChargeMethod,                //0x35	充电方法	1	0-固定充电效率法 1-可变充电效率法
    ConfigDataId_ChargeEfficiency,            //0x36	固定充电效率	2	1=0.001
    ConfigDataId_MaxDischargeCap,             //0x37	最大放电容量	2	最大默认为标称电池电量的1.5倍1=0.1AH
    ConfigDataId_CellMinVolt,                 //0x38	单体最小电压	2	1=0.001V
    ConfigDataId_CellMaxVolt,                 //0x39	单体最大电压	2	1=0.001V
    ConfigDataId_DischargeMaxCurrent,         //0x3A	放电最大电流	2	1=0.1A
    ConfigDataId_DischargeMinCurrent,         //0x3B	放电最小电流	2	1=0.1A
    ConfigDataId_MinCalculateTemp,            //0x3C	最低计算温度	2	1=0.1℃
    ConfigDataId_MaxCalculateTemp,            //0x3D	最高计算温度	2	1=0.1℃
    ConfigDataId_CapInitFlg,                  //0x3E	核容标志位	1	0-已核容 1-要核容
    ConfigDataId_CurrentGroupNumber,          //0x3F
    ConfigDataId_TotalVoltageThreshold = 0x40,//0x40    总电压告警阈值
    ConfigDataId_ChargeOverCurrentThreshold,  //0x41    充电过流阈值
    ConfigDataId_DischargeOverCurrentThreshold,//0x42	放电过流告警阈值
    ConfigDataId_TempOverThreshold,           //0x43	温度高告警阈值
    ConfigDataId_SocLowThreshold,             //0x44	SOC低告警阈值
    ConfigDataId_CellVoltOverThreshold,       //0x45	单体过压告警阈值
    ConfigDataId_CellVoltLowThreshold,        //0x46	单体欠压告警阈值
    ConfigDataId_CellTempOverThreshold,       //0x47	单体高温告警阈值
    ConfigDataId_CellInterResOverThreshold,   //0x48	单体内阻高告警阈值
    ConfigDataId_LeakThreshold,
    
    ConfigDataId_AlarmCorrelation = 0x4C,

    
    
    ConfigDataId_VoltageLevel = 0x50,         //0x50	电压等级
    ConfigDataId_CurrentLevel,                //0x51	电流等级
    ConfigDataId_CellSNLength,                //0x52	采集盒SN码长度
    ConfigDataId_AlarmMaxLevel,               //0x53	故障分级值
    ConfigDataId_CellWorkMode,                //0x54	采集盒工作模式
    ConfigDataId_VoltageCalib,                //0x55	电压校准值
    ConfigDataId_CurrentCalib,                //0x55	电流校准值	
    ConfigDataId_TempCalib,                   //0x56	温度校准值
    ConfigDataId_Volt5VCalib,                 //0x57	5v电压校准值
    
    
    ConfigDataId_AddSN = 0x80,                //0x80	增加SN	22 	Byte[2]:SN对应的序号
                                                                  //0:序号和SN不对应（现场部署）
                                                                  //其他:序号和SN对应(不需要部署，EXCEL导入)
                                                                  //Byte[20] :SN码，asc格式
                                                                  //响应查找指令时，需要返回序号和SN的对应关系
    ConfigDataId_DelSN,                       //0x81	删除单个SN码	20	Byte[20] :SN码，asc格式
    ConfigDataId_DelAllSN,                    //0x82	删除所有的SN码	20	Byte[20]个全FF
    ConfigDataId_NewSN,                       //0x83	替换的SN码	40	Byte[20] :被替换的SN码
                                                                      //Byte[20] :新的SN码
    ConfigDataId_Time,                        //0x84	时间	6	YYMMDDHHMMSS
    ConfigDataId_StorePeriod,                 //0x85	数据存储周期	2	秒
    ConfigDataId_Address,                     //0x86	设置地址	1	Byte[1] 需要设置的地址
    ConfigDataId_AddI03T,                     
    ConfigDataId_DelI03T,
    ConfigDataId_SetSoc,
    ConfigDataId_SetSoh,
    
    ConfigDataId_CloudInfo,
    ConfigDataId_4GModuleType,
    
    ConfigDataId_SynchI03TTime,
    
    ConfigDataId_AddSNCloudId,
    
    ConfigDataId_SetLocalIpV4,
    ConfigDataID_DNSServer,
    
    

}DATA_ID_Config_t;


typedef enum {
    StatusDataId_BatGroupStatus = 0x01,//0x01	电池组状态		0：非充放
                                                              //1：充电
                                                              //2：放电
    StatusDataId_BatGroupVoltage,      //0x02	电池组电压		0.1V/bit
    StatusDataId_BatGroupCurrent,      //0x03	电池组电流		Byte[1] 电池组序号
                                                        //充电为正，放电为负
                                                        //0.1A/bit
    StatusDataId_BatGroupSOC,          //0x04	电池组SOC		0.1%/bit
    StatusDataId_BatAvailableTime,     //0x05	备电预估时长		0.1H/bit
    StatusDataId_SOH,                  //0x06	SOH		0.1%/bit
    StatusDataId_DryNodeStatus,        //0x07	干节点信号状态		0：断开
                                                                  //1：闭合
    StatusDataId_CellNumber,           //0x08	单体个数		1个/bit
    StatusDataId_I03TNumber,           //0x09	I03T个数		
    StatusDataId_CellPeakInfo,         //0x0A	单体峰值信息		最大单体电压0.001V/bit
                                        //			最小单体电压0.001V/bit
                                        //			单体平均电压0.001V/bit
                                        //			最高单体序号
    
    StatusDataId_TempPeakInfo = 0x0E,   //0x0E	温度峰值信息
                                        //最高温度0.1℃/bit
                                        //最低温度0.1℃/bit
                                        //平均温度0.1℃/bit
                                        //最高温度序号
                                        //最低温度序号
                                        
    StatusDataId_ResPeakInfo = 0x0F,    
    
    StatusDataId_SoftVersion = 0x20,   //0x20	软件版本
    StatusDataId_HardVersion = 0x21,   //0x21	硬件版本
    StatusDataId_Time,
    
    StatusDataId_CellInfo = 0x30,//0x30	单体信息
                                //Byte[2]单体序号
                                //Byte[2]单体电压值 0.001V/bit
                                //Byte[2]单体内阻 1uΩ/bit
                                //Byte[2]单体温度0.1℃/bit
                                //Byte[2]预留
                                //Byte[2]预留
                                //Byte[2]预留
                                //Byte[2]预留
    StatusDataId_BatGroupAlarmInfo = 0x31,//0x31	电池组告警信息
                                //Byte[2]电池组序号
                                //Byte[2]告警信息1
                                //Bit0-Bit1:电池组过压告警等级
                                //Bit2-Bit3:电池组欠压告警等级
                                //Bit4-Bit5:电池组充电过流告警等级
                                //Bit6-Bit7:电池组放电过流告警等级
                                //Bit8-Bit9:环境温度高告警
                                //Bit10-Bit11:电阻组SOC低告警
                                //Bit12-Bit13:预留
                                //Bit14-Bit15:预留
                                //Byte[2]告警信息2
                                //Bit0：总压采集故障
                                //Bit1：电流1采集故障
                                //Bit2：电流2采集故障
                                //Bit3：电流3采集故障
                                //Bit4：电流4采集故障
                                //Bit5：温度采集故障
                                //Bit6：从设备1通讯故障
                                //Bit7-bit15：预留
                                //Byte[2]告警信息3

    StatusDataId_CellAlarmInfo = 0x32,//0x32	单体告警信息
                                //BYTE[2]单体编号
                                //BYTE[2]告警信息
                                //Bit0-Bit1:单体1过压告警
                                //Bit2-Bit3:单体1欠压告警
                                //Bit4-Bit5:单体1过温告警
                                //Bit6-Bit7:单体1内阻告警
                                //Bit8:单体1通讯异常告警
                                //Bit9:电压采集异常告警
                                //Bit10:温度采集异常告警
                                //Bit11-Bit15:预留
                                        
}DATA_ID_Status_t;


typedef enum {
    ControlDataId_Reset = 1,
    ControlDataId_Format,
    ControlDataId_Factory,
    ControlDataId_SampleIntRes,
    ControlDataId_ClearData,
    ControlDataId_UDiskExport,
    ControlDataId_UDiskUpgrade,
    ControlDataId_UDiskStatus,
    
}DATA_ID_Control_t;

#define I03T_GROUP1_ALARM_TEMP_HI_POS     (4)
#define I03T_GROUP1_ALARM_SOC_LOW_POS     (6)

#define I03T_GROUP2_ALARM_TEMP_ERR_POS    (5)
#define I03T_GROUP2_ALARM_COMM_ERR_POS    (6)



typedef enum {
    Result_OK = 0,
    Result_MsgErr,
    Result_NotSupport,
    Result_HandleFailed,
	Result_SerErr,
	Result_FileCheckErr,
	Result_FileTooLarge,
	Result_VersionErr,
	Result_WriteErr,
	Result_Busy,
}CodeResult_t;

typedef struct {
	uint8_t      DataId;
    uint8_t      transfer;
	CodeResult_t result;
	uint8_t      extLength;
	uint8_t      extData[4];
}HandleResult;





typedef struct {
	uint8_t  tag;
	uint8_t  lh;
	uint8_t  ll;
	uint8_t  *pv;
}tlv_t;



typedef struct {
	uint32_t rx_tick;
	uint16_t length;
	uint8_t  buffer[PROTOCOL_RX_MAX_SIZE];
}protocol_msg_t;

typedef struct {
	TAG_t       tag;
	uint8_t     ser;
	errStatus_t err;
	union {
		void     *p;
		uint32_t  v;
	}arg;
}ModuleRxMsg_t;

typedef struct {
    uint8_t *pdata;
    uint16_t length;
}ModuleTxMsg_t;

typedef struct {
    uint8_t logger_type;
    uint8_t alarm_type;
    uint8_t alarm_lvl;
    uint32_t logger_start_time;
    uint32_t logger_end_time;
}alarm_infor_t;


typedef struct {
    uint8_t  logger_type;
    uint8_t  para8_1;
    uint8_t  para8_2;
    uint8_t  para8_3;
 
    uint32_t para32_1;
    uint32_t para32_2;
    uint32_t para32_3;
}Logger_Msg_t;


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
						             uint8_t        resultcnt);

uint16_t de_protocol_slave_package_fill(uint8_t  *p,
							  uint8_t        ctrl,
		                      uint8_t        src,
					          uint8_t        dest,
					          uint8_t        ser,
					          uint16_t       tag_mask,
                              uint8_t        *pdata,
                              uint16_t       length);
                              
uint16_t de_protocol_slave_package( COMM_TYPE_t commType,
                                    uint8_t  *p,
                                    uint8_t        ctrl,
                                    uint8_t        src,
                                    uint8_t        dest,
                                    uint8_t        ser,
                                    uint16_t       tag_mask,
                                    HandleResult  *result,
                                    uint8_t        resultcnt,
                                    uint8_t       *remain);

void Bsp_Rs485SendBytes(uint8_t *pdata,uint16_t length);
void Bsp_Rs485Uplink1SendBytes(uint8_t*,uint16_t);
//errStatus_t rs485_2_sendData(uint8_t *buffer, int bytes);
//#define protocol_send_bytes  Bsp_Rs485SendBytes
void protocol_send_bytes(COMM_TYPE_t type,uint8_t *pdata,uint16_t length);
void de_protocol_master_process(COMM_TYPE_t type,uint8_t *pdate,uint16_t length);
errStatus_t i03t_module_control(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length,uint8_t trytimes);
errStatus_t i03t_module_config(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length);
//errStatus_t i03t_module_query(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length,uint8_t trytimes);
errStatus_t i03t_module_query(COMM_TYPE_t commType,uint8_t target_addr,uint8_t tag,uint8_t *p_content,uint16_t length,uint8_t trytimes,uint32_t timeout);
#ifdef __cplusplus
}
#endif

#endif
