#ifndef __DE_MODBUS_PROTOCOL_H_
#define __DE_MODBUS_PROTOCOL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "typedef.h"

#define FUNCTION_CONTINUES_R_MAX_REGS    125
#define FUNCTION_CONTINUES_W_MAX_REGS    120

#define FUNCTION_READ_RW_REGS            0x03
#define FUNCTION_READ_RO_REGS            0x04
#define FUNCTION_WRTIE_REG               0x06
#define FUNCTION_WRTIE_REGS              0x10


//-------------regs read only-----------------
typedef enum {
    ROReg_DeviceNum = 0x01,
    ROReg_TotalCellNum,
    
    ROReg_SoftVersionI03M = 0x10,
    ROReg_HardVersionI03M,
    ROReg_YearI03M,
    ROReg_MonthI03M,
    ROReg_DayI03M,
    ROReg_HourI03M,
    ROReg_MinI03M,
    ROReg_SecI03M,

    
}MODBUS_RO_REGS_I03M_t;

typedef enum {
    RWReg_Addr = 0x01,
    
    RWReg_YearI03M = 0x07,
    RWReg_MonthI03M,
    RWReg_DayI03M,
    RWReg_HourI03M,
    RWReg_MinI03M,
    RWReg_SecI03M,
    RWReg_SecH,
    RWReg_SecL,
    
    RW_StorePeriod = 0x10,
  
}MODBUS_RW_REGS_I03M_t;

typedef enum {

    ROReg_BatGroupStatus = 0x01,                                                    
    ROReg_BatGroupVoltage, 
    ROReg_BatGroupCurrent_1,
    ROReg_BatSOC,    
    ROReg_BatAvailableTime,     
    ROReg_SOH,                  
    ROReg_DryNodeStatus,                                     
    ROReg_CellNumber,  
    ROReg_CellMaxVoltage,
    ROReg_CellMinVoltage,
    ROReg_CellAverageVoltage,
    ROReg_CellDiffVolt,
    ROReg_CellMaxVoltageId,
    ROReg_CellMinVoltageId,
    ROReg_CellMaxTemp,
    ROReg_CellMinTemp,
    ROReg_CellAverageTemp,
    ROReg_CellDiffTemp,
    ROReg_CellMaxTempId,
    ROReg_CellMinTempId,
    ROReg_CellMaxRes,
    ROReg_CellMinRes,
    ROReg_CellAverageRes,
    ROReg_CellDiffRes,
    ROReg_CellMaxResId,
    ROReg_CellMinResId,
    ROReg_BatGroupCurrent_2,
    ROReg_BatGroupCurrent_3,
    ROReg_BatGroupCurrent_4,
    ROReg_BatGroupSOC_2,
    ROReg_BatGroupSOC_3,
    ROReg_BatGroupSOC_4,
    ROReg_BatAvailableTime_2,
    ROReg_BatAvailableTime_3,
    ROReg_BatAvailableTime_4,
    ROReg_SOH_2,
    ROReg_SOH_3,
    ROReg_SOH_4,
    ROReg_BatGroupStatus_2,
    ROReg_BatGroupStatus_3,
    ROReg_BatGroupStatus_4,
    
    ROReg_SoftVersion = 0x2E,
    ROReg_HardVersion = 0x2F,

    ROReg_BatGroupRegMax = 0x2F,
    
    ROReg_CellVoltage_1 = 0x30,
    ROReg_CellIntRes_1,
    ROReg_CellTemp_1,
    ROReg_CellSpare_1_1,
    ROReg_CellSpare_1_2,
    
    
    ROReg_CellVoltage_Max,
    ROReg_CellIntRes_Max,
    ROReg_CellTemp_Max,
    ROReg_CellSpare_Max_1,
    ROReg_CellSpare_MAX_2 = 0x60B,
    
    
    ROReg_AlarmBatGroup1 = 0x60C,
    ROReg_AlarmBatGroup2,
    ROReg_AlarmBatGroup3,
    
    ROReg_AlarmCell_1 = 0x610,
    //...
    ROReg_AlarmCell_MAX = 0x73B,
    
    ROReg_SampleTaskStatus = 0x800,
    ROReg_SamplingNo,
    ROReg_SampledNo,
    ROReg_BroadcastTotalNo,


} MODBUS_RO_REGS_t;

//-------------regs read and write -----------
typedef enum {
    RWReg_ModbusAddr = 1,
    RWReg_BaudRate,
    RWReg_DataBits,
    RWReg_StopBits,
    RWReg_ParityBits,    
    RWReg_TotalVoltageSamplePeriod = 6,//0x06
    RWReg_CellPullPeriod,              //0x07
    RWReg_ResSamplePeriod,             //0x08
    RWReg_CellNumber,                  //0x09	
    RWReg_CriticalCurrentLimits,       //0x0A	
    RWReg_FastToFloat,                 //0x0B	
    RWReg_FloatToFast,                 //0x0C	
    RWReg_ChargeToDischarge,           //0x0D	
    RWReg_DrySwitch,                   //0x0E	
    RWReg_DryOutputThreshold,          //0x0F	
    RWReg_DryResumeThreshold,          //0x10	
    RWReg_DryDelay,                    //0x11	
    RWReg_DryOutputMinVoltage,         //0x12	
    RWReg_433WorkFreq,                 //0x13	
    RWReg_433AssistFreq,               //0x14	
    RWReg_SOC,
    RWReg_SOH,
    
    RWReg_NominalCap = 0x30,           //0x30	标称容量	2	1=0.1AH
    RWReg_NominalVolt,                 //0x31	标称电压	2	1-2V 3-6V 6-12V
    RWReg_FloatVolt,                   //0x32	浮充电压	2	1=0.001V
    RWReg_TempCompensate,              //0x33	温度补偿电压	2	1=0.001mV
    RWReg_DischargeMethod,             //0x34	放电方法	1	0-安时放电法 1-深度学习法
    RWReg_ChargeMethod,                //0x35	充电方法	1	0-固定充电效率法 1-可变充电效率法
    RWReg_ChargeEfficiency,            //0x36	固定充电效率	2	1=0.001
    RWReg_MaxDischargeCap,             //0x37	最大放电容量	2	最大默认为标称电池电量的1.5倍1=0.1AH
    RWReg_CellMinVolt,                 //0x38	单体最小电压	2	1=0.001V
    RWReg_CellMaxVolt,                 //0x39	单体最大电压	2	1=0.001V
    RWReg_DischargeMaxCurrent,         //0x3A	放电最大电流	2	1=0.1A
    RWReg_DischargeMinCurrent,         //0x3B	放电最小电流	2	1=0.1A
    RWReg_MinCalculateTemp,            //0x3C	最低计算温度	2	1=0.1℃
    RWReg_MaxCalculateTemp,            //0x3D	最高计算温度	2	1=0.1℃
    RWReg_CapInitFlg,                  //0x3E	核容标志位	1	0-已核容 1-要核容


    RWReg_TotalVoltageOverThreshold_Level_1 = 0x40,
	RWReg_TotalVoltageOverThreshold_Level_2,
	RWReg_TotalVoltageOverThreshold_Level_3,
	RWReg_TotalVoltageOverThreshold_ResumeLevel_1,
	RWReg_TotalVoltageOverThreshold_ResumeLevel_2,
	RWReg_TotalVoltageOverThreshold_ResumeLevel_3,
	RWReg_TotalVoltageOverThreshold_DelayLevel_1,
	RWReg_TotalVoltageOverThreshold_DelayLevel_2,
	RWReg_TotalVoltageOverThreshold_DelayLevel_3,

	RWReg_TotalVoltageLowThreshold_Level_1,
	RWReg_TotalVoltageLowThreshold_Level_2,
	RWReg_TotalVoltageLowThreshold_Level_3,
	RWReg_TotalVoltageLowThreshold_ResumeLevel_1,
	RWReg_TotalVoltageLowThreshold_ResumeLevel_2,
	RWReg_TotalVoltageLowThreshold_ResumeLevel_3,
	RWReg_TotalVoltageLowThreshold_DelayLevel_1,
	RWReg_TotalVoltageLowThreshold_DelayLevel_2,
	RWReg_TotalVoltageLowThreshold_DelayLevel_3,
	
	RWReg_ChargeOverCurrentThreshold_Level_1,
	RWReg_ChargeOverCurrentThreshold_Level_2,
	RWReg_ChargeOverCurrentThreshold_Level_3,
	RWReg_ChargeOverCurrentThreshold_ResumeLevel_1,
	RWReg_ChargeOverCurrentThreshold_ResumeLevel_2,
	RWReg_ChargeOverCurrentThreshold_ResumeLevel_3,
	RWReg_ChargeOverCurrentThreshold_DelayLevel_1,
	RWReg_ChargeOverCurrentThreshold_DelayLevel_2,
	RWReg_ChargeOverCurrentThreshold_DelayLevel_3,
	
	RWReg_DischargeOverCurrentThreshold_Level_1,
	RWReg_DischargeOverCurrentThreshold_Level_2,
	RWReg_DischargeOverCurrentThreshold_Level_3,
	RWReg_DischargeOverCurrentThreshold_ResumeLevel_1,
	RWReg_DischargeOverCurrentThreshold_ResumeLevel_2,
	RWReg_DischargeOverCurrentThreshold_ResumeLevel_3,
	RWReg_DischargeOverCurrentThreshold_DelayLevel_1,
	RWReg_DischargeOverCurrentThreshold_DelayLevel_2,
	RWReg_DischargeOverCurrentThreshold_DelayLevel_3,
	
	RWReg_TempOverThreshold_Level_1,
	RWReg_TempOverThreshold_Level_2,
	RWReg_TempOverThreshold_Level_3,
	RWReg_TempOverThreshold_ResumeLevel_1,
	RWReg_TempOverThreshold_ResumeLevel_2,
	RWReg_TempOverThreshold_ResumeLevel_3,
	RWReg_TempOverThreshold_DelayLevel_1,
	RWReg_TempOverThreshold_DelayLevel_2,
	RWReg_TempOverThreshold_DelayLevel_3,

    RWReg_SocLowThreshold_Level_1,
	RWReg_SocLowThreshold_Level_2,
	RWReg_SocLowThreshold_Level_3,
	RWReg_SocLowThreshold_ResumeLevel_1,
	RWReg_SocLowThreshold_ResumeLevel_2,
	RWReg_SocLowThreshold_ResumeLevel_3,
	RWReg_SocLowThreshold_DelayLevel_1,
	RWReg_SocLowThreshold_DelayLevel_2,
	RWReg_SocLowThreshold_DelayLevel_3,
	
    RWReg_CellVoltOverThreshold_Level_1,
	RWReg_CellVoltOverThreshold_Level_2,
	RWReg_CellVoltOverThreshold_Level_3,
	RWReg_CellVoltOverThreshold_ResumeLevel_1,
	RWReg_CellVoltOverThreshold_ResumeLevel_2,
	RWReg_CellVoltOverThreshold_ResumeLevel_3,
	RWReg_CellVoltOverThreshold_DelayLevel_1,
	RWReg_CellVoltOverThreshold_DelayLevel_2,
	RWReg_CellVoltOverThreshold_DelayLevel_3,
	
    RWReg_CellVoltLowThreshold_Level_1,
	RWReg_CellVoltLowThreshold_Level_2,
	RWReg_CellVoltLowThreshold_Level_3,
	RWReg_CellVoltLowThreshold_ResumeLevel_1,
	RWReg_CellVoltLowThreshold_ResumeLevel_2,
	RWReg_CellVoltLowThreshold_ResumeLevel_3,
	RWReg_CellVoltLowThreshold_DelayLevel_1,
	RWReg_CellVoltLowThreshold_DelayLevel_2,
	RWReg_CellVoltLowThreshold_DelayLevel_3,
	
    RWReg_CellTempOverThreshold_Level_1,
	RWReg_CellTempOverThreshold_Level_2,
	RWReg_CellTempOverThreshold_Level_3,
	RWReg_CellTempOverThreshold_ResumeLevel_1,
	RWReg_CellTempOverThreshold_ResumeLevel_2,
	RWReg_CellTempOverThreshold_ResumeLevel_3,
	RWReg_CellTempOverThreshold_DelayLevel_1,
	RWReg_CellTempOverThreshold_DelayLevel_2,
	RWReg_CellTempOverThreshold_DelayLevel_3,
	
    RWReg_CellInterResOverThreshold_Level_1,
	RWReg_CellInterResOverThreshold_Level_2,
	RWReg_CellInterResOverThreshold_Level_3,
	RWReg_CellInterResOverThreshold_ResumeLevel_1,
	RWReg_CellInterResOverThreshold_ResumeLevel_2,
	RWReg_CellInterResOverThreshold_ResumeLevel_3,
	RWReg_CellInterResOverThreshold_DelayLevel_1,
	RWReg_CellInterResOverThreshold_DelayLevel_2,
	RWReg_CellInterResOverThreshold_DelayLevel_3,
    
    RWReg_AlarmCorrelation = 0xB0,
    RWReg_AlarmCorrelationGroup,
    RWReg_AlarmCorrelationCells,

	RWReg_ResetI03M = 0xF0,
	RWReg_SampleIntRes,
	RWReg_ClearHist,
	RWReg_Format,
    
	
    RWReg_VoltageLevel = 0x100,     
    RWReg_CurrentLevel,
    RWReg_CellSNLength,              
    RWReg_AlarmMaxLevel,             
    RWReg_CellWorkMode,
    
    RWReg_VoltageCalibZeroOffset,
    RWReg_VoltageCalibKH,
    RWReg_VoltageCalibKL,
    RWReg_VoltageCalibBH,
    RWReg_VoltageCalibBL,
    
    RWReg_Current1CalibZeroOffset,
    RWReg_Current1CalibKH,
    RWReg_Current1CalibKL,
    RWReg_Current1CalibBH,
    RWReg_Current1CalibBL,
    
    RWReg_TempCalibKH,
    RWReg_TempCalibKL,
    RWReg_TempCalibBH,
    RWReg_TempCalibBL,
    
    RWReg_5VCalibKH,
    RWReg_5VCalibKL,
    RWReg_5VCalibBH,
    RWReg_5VCalibBL,
    
    RWReg_CellReportPeriod = 0x118,
    
    RWRegExtCfg_LeakThreshold_Level_1 = 0x201,
	RWRegExtCfg_LeakThreshold_Level_2,
	RWRegExtCfg_LeakThreshold_Level_3,
	RWRegExtCfg_LeakThreshold_ResumeLevel_1,
	RWRegExtCfg_LeakThreshold_ResumeLevel_2,
	RWRegExtCfg_LeakThreshold_ResumeLevel_3,
	RWRegExtCfg_LeakThreshold_DelayLevel_1,
	RWRegExtCfg_LeakThreshold_DelayLevel_2,
	RWRegExtCfg_LeakThreshold_DelayLevel_3,
               

} MODBUS_RW_REGS_t;

typedef enum {
    MODBUS_ERR_illegal_Fun = 0x01,
    MODBUS_ERR_illegal_Addr,
    MODBUS_ERR_illegal_Data,
    MODBUS_ERR_SubStation,
    MODBUS_ERR_Ack_Frame,
    MODBUS_ERR_SubStationBusy,
}ModbusErrCode_t;

#define MODBUS_BUFF_MAX_SIZE      1024

typedef errStatus_t (*de_modbus_read)(uint16_t reg_start,uint16_t reg_cnt,uint16_t *value);
typedef errStatus_t (*de_modbus_write)(uint16_t reg_start,uint16_t reg_cnt,uint16_t *value);

typedef struct {
    uint16_t        reg;
    de_modbus_read  read_handle;
    de_modbus_write write_handle; 
}DE_MODBUS_t;

typedef struct {
	uint8_t address;
	uint8_t function;
	uint8_t reg_addr_hi;
	uint8_t reg_addr_lo;
	uint8_t regs_num_hi;
	uint8_t regs_num_lo;
	uint8_t crc16_hi;
	uint8_t crc16_lo;
}modbus_request_t;

typedef struct {
    uint8_t address;
	uint8_t function;
	uint8_t reg_addr_hi;
	uint8_t reg_addr_lo;
	uint8_t regs_num_hi;
	uint8_t regs_num_lo;
	uint8_t bytes;        //data length.
	uint8_t data[];
}modbus_request_conti_write_t;

typedef struct {
	uint8_t address;
	uint8_t fuction;
	uint8_t buff[MODBUS_BUFF_MAX_SIZE];
}modbus_response_t;

void Bsp_Rs485Uplink2SendBytes(uint8_t *pdata,uint16_t length);

void modbus_protocol_send_bytes(COMM_TYPE_t type,uint8_t *pdata,uint16_t length);
void modbus_handler(COMM_TYPE_t commType,uint8_t *pdata,uint16_t length);
uint16_t modbus_response_err(modbus_request_t *request,uint8_t *buffer,ModbusErrCode_t err_code);

#ifdef  __cplusplus
}
#endif


#endif
