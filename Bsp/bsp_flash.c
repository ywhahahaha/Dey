/*
*********************************************************************************************************
*
*	模块名称 : stm32内部FLASH 读写模块
*	文件名称 : stm32_flash.c
*	版    本 : V1.0
*	修改记录 :
*		版本号  日期        作者     说明

*********************************************************************************************************
*/
#include <stdbool.h>
#include "bsp_flash.h"
#include "stm32f4xx.h"


/*************************************************
Function: Bsp_FlashReadWord
Description: 读取指定地址的数据(32位字节) 
Input: 	faddr:读地址 
Return: uint32_t 数据
Others:
*************************************************/
uint32_t Bsp_FlashReadWord(uint32_t faddr)
{
	return *(volatile uint32_t*)faddr; 
}


/*************************************************
Function: STMFLASH_GetFlashSector
Description: 获取某个地址所在的flash扇区
Input: 	addr:flash地址
Return: 返回值:0~11,addr所在的扇区
Others:
*************************************************/
uint8_t Bsp_GetFlashSector(uint32_t addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_SECTOR_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_SECTOR_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_SECTOR_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_SECTOR_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_SECTOR_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_SECTOR_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_SECTOR_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_SECTOR_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_SECTOR_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_SECTOR_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_SECTOR_10;   
	return FLASH_SECTOR_11;	
}

/*************************************************
Function: STM32_FLASH_Read
Description: 	从指定地址开始读出指定长度的数据
Input: 	ReadAddr:起始地址
				pBuffer:数据指针
				NumToRead:长度
Return: 
Others:
*************************************************/
void Bsp_FlashRead(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t lengthOfByte)   	
{
	uint32_t i;
	for(i=0;i<lengthOfByte;i+=4)
	{
		*pBuffer++ = Bsp_FlashReadWord(ReadAddr);//读取4个字节.
		ReadAddr += 4;//偏移4个字节.	
	}
}

bool Bsp_FlashEraseSector(uint32_t address)
{
    uint32_t SectorError = 0;
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef FlashEraseInit;
    
    //HAL_FLASH_Unlock(); 
    
  
    
    __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;       		//擦除类型，扇区擦除 
    FlashEraseInit.Sector= Bsp_GetFlashSector(address);             //要擦除的扇区
    FlashEraseInit.NbSectors=1;                             		//一次只擦除一个扇区
    FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;      		//电压范围，VCC=2.7~3.6V之间!!
    status = HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError);
    FLASH_WaitForLastOperation(FLASH_WAITETIME);                    //等待上次操作完成
    //HAL_FLASH_Lock();  
    
    return status == HAL_OK;
}

bool Bsp_FlashProgram(uint32_t address,uint32_t *pBuffer,uint16_t lengthOfByte) {
    
    bool ret = false;
    
    //HAL_FLASH_Unlock(); 

    if(FLASH_WaitForLastOperation(FLASH_WAITETIME) == HAL_OK)            //等待上次操作完成
	{
        uint32_t programAddress = address;
        
        uint32_t endAddress = address + lengthOfByte;
        while(1) {
            
            if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,programAddress,*pBuffer) != HAL_OK){ 
                    
                __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
				ret = false;
                break;	//写入异常
			}
            
            programAddress += 4;
            pBuffer++;
            
            if(programAddress >= endAddress) {
                ret = true;
                break;
            }
                
        }
	}
    
    //HAL_FLASH_Lock();  
    
    return ret;
}


