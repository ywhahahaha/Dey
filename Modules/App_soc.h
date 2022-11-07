/*
 * @Descripttion: 
 * @version: 
 * @Author: czh
 * @Date: 2022-02-21 17:08:43
 * @LastEditors: czh
 * @LastEditTime: 2022-04-22 11:53:36
 */
#ifndef __APP_SOC_H
#define __APP_SOC_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "rtwtypes.h"
#include <math.h>
//#include <emmintrin.h>
#include <stddef.h>
#include <string.h>
/******************************************模型仿真模块参数START******************************************/
/* Block states (default storage) for system 'Model_simulation' */
typedef struct {
  real_T Delay_DSTATE[6];                 /* 'Delay' (':184') */
  real_T Memory1_PreviousInput[6];        /* 'Memory1' (':172') */
  real_T Memory_PreviousInput[2][6];      /* 'Memory' (':153') */
  uint32_T RandomSource_STATE_DWORK[2];/* 'Random Source' (':259:3') */
  uint32_T method;                     /* '真实状态' (':152') */
  uint32_T state[2];                   /* '真实状态' (':152') */
  uint32_T method_n;                   /* '真实状态' (':152') */
  uint32_T state_c;                    /* '真实状态' (':152') */
  uint32_T state_d[2];                 /* '真实状态' (':152') */
  uint32_T state_k[625];               /* '真实状态' (':152') */
}  DW_Model_simulation_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T I_real;                       /* 'I_real' (':369') */
} ExtU_Model_simulation_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T SOC_real;                     /* 'SOC_real' (':370') */
  real_T UL_obs;                       /* 'UL_obs' (':371') */
} ExtY_Model_simulation_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Pooled Parameter (Mixed Expressions)
   * Referenced by:
   *   'Delay' (':184')
   *   'Random Source' (':259:3')
   */
  real_T pooled1;

  /* Pooled Parameter (Mixed Expressions)
   * Referenced by:
   *   'tr' (':143')
   *   'Memory1' (':172')
   *   'Random Source' (':259:3')
   */
  real_T pooled2;

  /* Expression: [1.715,1.355	,1.158,0.858,0.787,0.852,0.835]
   * Referenced by: '1-D Lookup Table' (':341')
   */
  real_T uDLookupTable_tableData[7];

  /* Pooled Parameter (Expression: [0.2:0.1:0.8])
   * Referenced by:
   *   '1-D Lookup Table' (':341')
   *   '1-D Lookup Table1' (':342')
   *   '1-D Lookup Table3' (':344')
   */
  real_T pooled3[7];

  /* Expression: [3.623,4.678,4.566,4.180,3.804,5.026,4.181]
   * Referenced by: '1-D Lookup Table1' (':342')
   */
  real_T uDLookupTable1_tableData[7];

  /* Expression: [1.927,	1.954,	1.981,	2.008,	2.035,	2.062,	2.089,	2.116,	2.143,	2.17
     ]
   * Referenced by: '1-D Lookup Table2' (':343')
   */
  real_T uDLookupTable2_tableData[10];

  /* Expression: [0.1:0.1:1]
   * Referenced by: '1-D Lookup Table2' (':343')
   */
  real_T uDLookupTable2_bp01Data[10];

  /* Expression: [1.07,0.95,0.85,0.83,0.82,0.76,0.79]
   * Referenced by: '1-D Lookup Table3' (':344')
   */
  real_T uDLookupTable3_tableData[7];
} ConstP_Model_simulation_T;
/******************************************模型仿真模块参数END******************************************/

/********************************************EKF算法参数START******************************************/

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real_T UnitDelay2_DSTATE[4];         /* '<S1>/Unit Delay2' */
  real_T UnitDelay_DSTATE;             /* '<S1>/Unit Delay' */
  real_T Delay1_DSTATE;                /* '<S1>/Delay1' */
  real_T Delay_DSTATE;                 /* '<S1>/Delay' */
  real_T UnitDelay1_DSTATE;            /* '<S1>/Unit Delay1' */
} DW_SOC_EKF_T;

/* Invariant block signals (default storage) */
typedef struct {
  const real_T Fcn1;                   /* '<S3>/Fcn1' */
} ConstB_SOC_EKF_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: [1.715,1.355	,1.158,0.858,0.787,0.852,0.835]
   * Referenced by: '<S1>/1-D Lookup Table'
   */
  real_T uDLookupTable_tableData[7];

  /* Pooled Parameter (Expression: [0.2:0.1:0.8])
   * Referenced by:
   *   '<S1>/1-D Lookup Table'
   *   '<S1>/1-D Lookup Table1'
   *   '<S1>/1-D Lookup Table3'
   */
  real_T pooled2[7];

  /* Expression: [3.623,4.678,4.566,4.180,3.804,5.026,4.181]
   * Referenced by: '<S1>/1-D Lookup Table1'
   */
  real_T uDLookupTable1_tableData[7];

  /* Expression: [1.927,	1.954,	1.981,	2.008,	2.035,	2.062,	2.089,	2.116,	2.143,	2.17
     ]
   * Referenced by: '<S1>/1-D Lookup Table4'
   */
  real_T uDLookupTable4_tableData[10];

  /* Expression: [0.1:0.1:1]
   * Referenced by: '<S1>/1-D Lookup Table4'
   */
  real_T uDLookupTable4_bp01Data[10];

  /* Expression: [1.07,0.95,0.85,0.83,0.82,0.76,0.79]
   * Referenced by: '<S1>/1-D Lookup Table3'
   */
  real_T uDLookupTable3_tableData[7];
} ConstP_SOC_EKF_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T u;                            /* '<Root>/U_obs' */
  real_T u_f;                          /* '<Root>/I_obs' */
} ExtU_SOC_EKF_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T SOC_upd;                      /* '<Root>/SOC_upd' */
} ExtY_SOC_EKF_T;

/********************************************EKF算法参数END******************************************/

/******************************************信号添加噪声参数START****************************************/

/* Block states (default storage) for system '<Root>' */
typedef struct {
  uint32_T RandomSource_STATE_DWORK[2];/* '<S1>/Random Source' */
} DW_Noise_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: MeanVal
   * Referenced by: '<S1>/Random Source'
   */
  real_T RandomSource_MeanRTP;

  /* Computed Parameter: RandomSource_VarianceRTP
   * Referenced by: '<S1>/Random Source'
   */
  real_T RandomSource_VarianceRTP;
} ConstP_Noise_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T In1;                          /* '<Root>/In1' */
  real_T Variance;                     /* '<Root>/Variance' */
} ExtU_Noise_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T Out1;                         /* '<Root>/Out1' */
} ExtY_Noise_T;
/******************************************信号添加噪声参数END****************************************/

/******************************************初始容量参数START**************************************/
/* Constant parameters (default storage) */
typedef struct {
  real_T cap_tableData[24];

  real_T temp_Data[8];

  real_T curcoeff_Data[3];

  uint32_T maxIndex[2];
} ConstP_Cap_init_T;
/******************************************初始容量参数END****************************************/

/******************************************外部调用函数START****************************************/
//#define SOC_PRINTF

#ifdef SOC_PRINTF
#define vs_printf(...)   printf(__VA_ARGS__)
#endif
extern unsigned char App_deep_cycle_time(float *deep_cycle_time,int real_cur,int real_temp,unsigned char bat_state,unsigned char bat_pack_label);
extern void App_soc_intial(float initial_cap,unsigned int initial_soc,float real_vol, float float_charge_vol,float max_cycle_use_vol,float mix_cycle_use_vol,unsigned char bat_state,unsigned char bat_pack_label,unsigned char soc_retain_flag);
extern unsigned int App_soc_process(float real_vol,float real_cur,float temp,float clk_in_sec,unsigned char bat_state,unsigned char bat_pack_label);
extern unsigned int App_standby_time(unsigned char bat_pack_label,float real_cur,unsigned char bat_state);
extern void App_soh_intial(unsigned int life,unsigned int intial_soh,float deep_cycle_time,unsigned char bat_pack_label);
extern unsigned int App_soh_process(unsigned char bat_pack_label);
void App_soc_terminate(unsigned char bat_pack_label);
/******************************************外部调用函数END****************************************/

#ifdef __cplusplus
}
#endif
#endif
