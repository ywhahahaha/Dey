/**
  ******************************************************************************
  * File Name          : FMC.c
  * Description        : This file provides code for the configuration
  *                      of the FMC peripheral.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp_fmc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

NAND_HandleTypeDef hnand2;

/* FMC initialization function */
void MX_FMC_Init(void)
{
  FMC_NAND_PCC_TimingTypeDef ComSpaceTiming;
  FMC_NAND_PCC_TimingTypeDef AttSpaceTiming;

  /** Perform the NAND2 memory initialization sequence
  */
  hnand2.Instance = FMC_NAND_DEVICE;
  /* hnand2.Init */
  hnand2.Init.NandBank = FMC_NAND_BANK2;
  hnand2.Init.Waitfeature = FMC_NAND_PCC_WAIT_FEATURE_DISABLE;
  hnand2.Init.MemoryDataWidth = FMC_NAND_PCC_MEM_BUS_WIDTH_8;
  hnand2.Init.EccComputation = FMC_NAND_ECC_DISABLE;
  hnand2.Init.ECCPageSize = FMC_NAND_ECC_PAGE_SIZE_512BYTE;
  hnand2.Init.TCLRSetupTime = 9;
  hnand2.Init.TARSetupTime = 9;
  /* hnand2.Config */
  hnand2.Config.PageSize = 2048;
  hnand2.Config.SpareAreaSize = 64;
  hnand2.Config.BlockSize = 64;
  hnand2.Config.BlockNbr = 2048;
  hnand2.Config.PlaneNbr = 1;
  hnand2.Config.PlaneSize = 2048;
  hnand2.Config.ExtraCommandEnable = DISABLE;
  /* ComSpaceTiming */
  ComSpaceTiming.SetupTime = 9;
  ComSpaceTiming.WaitSetupTime = 9;
  ComSpaceTiming.HoldSetupTime = 10;
  ComSpaceTiming.HiZSetupTime = 9;
  /* AttSpaceTiming */
  AttSpaceTiming.SetupTime = 9;
  AttSpaceTiming.WaitSetupTime = 9;
  AttSpaceTiming.HoldSetupTime = 10;
  AttSpaceTiming.HiZSetupTime = 9;

  if (HAL_NAND_Init(&hnand2, &ComSpaceTiming, &AttSpaceTiming) != HAL_OK)
  {
    Error_Handler( );
  }

}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void){
  /* USER CODE BEGIN FMC_MspInit 0 */

  /* USER CODE END FMC_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct;
  if (FMC_Initialized) {
    return;
  }
  FMC_Initialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_ENABLE();
  
  /** FMC GPIO Configuration  
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PD11   ------> FMC_CLE
  PD12   ------> FMC_ALE
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD6   ------> FMC_NWAIT
  PD7   ------> FMC_NCE2
  */
  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15 
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN FMC_MspInit 1 */

  /* USER CODE END FMC_MspInit 1 */
}

void HAL_NAND_MspInit(NAND_HandleTypeDef* nandHandle){
  /* USER CODE BEGIN NAND_MspInit 0 */

  /* USER CODE END NAND_MspInit 0 */
  HAL_FMC_MspInit();
  /* USER CODE BEGIN NAND_MspInit 1 */

  /* USER CODE END NAND_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void){
  /* USER CODE BEGIN FMC_MspDeInit 0 */

  /* USER CODE END FMC_MspDeInit 0 */
  if (FMC_DeInitialized) {
    return;
  }
  FMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();
  
  /** FMC GPIO Configuration  
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PD11   ------> FMC_CLE
  PD12   ------> FMC_ALE
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD6   ------> FMC_NWAIT
  PD7   ------> FMC_NCE2
  */

  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15 
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7);

  /* USER CODE BEGIN FMC_MspDeInit 1 */

  /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_NAND_MspDeInit(NAND_HandleTypeDef* nandHandle){
  /* USER CODE BEGIN NAND_MspDeInit 0 */

  /* USER CODE END NAND_MspDeInit 0 */
  HAL_FMC_MspDeInit();
  /* USER CODE BEGIN NAND_MspDeInit 1 */

  /* USER CODE END NAND_MspDeInit 1 */
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
