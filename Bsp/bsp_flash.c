/*
*********************************************************************************************************
*
*	ģ������ : stm32�ڲ�FLASH ��дģ��
*	�ļ����� : stm32_flash.c
*	��    �� : V1.0
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��

*********************************************************************************************************
*/
#include <stdbool.h>
#include "bsp_flash.h"
#include "stm32f4xx.h"


/*************************************************
Function: Bsp_FlashReadWord
Description: ��ȡָ����ַ������(32λ�ֽ�) 
Input: 	faddr:����ַ 
Return: uint32_t ����
Others:
*************************************************/
uint32_t Bsp_FlashReadWord(uint32_t faddr)
{
	return *(volatile uint32_t*)faddr; 
}


/*************************************************
Function: STMFLASH_GetFlashSector
Description: ��ȡĳ����ַ���ڵ�flash����
Input: 	addr:flash��ַ
Return: ����ֵ:0~11,addr���ڵ�����
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
Description: 	��ָ����ַ��ʼ����ָ�����ȵ�����
Input: 	ReadAddr:��ʼ��ַ
				pBuffer:����ָ��
				NumToRead:����
Return: 
Others:
*************************************************/
void Bsp_FlashRead(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t lengthOfByte)   	
{
	uint32_t i;
	for(i=0;i<lengthOfByte;i+=4)
	{
		*pBuffer++ = Bsp_FlashReadWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr += 4;//ƫ��4���ֽ�.	
	}
}

bool Bsp_FlashEraseSector(uint32_t address)
{
    uint32_t SectorError = 0;
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef FlashEraseInit;
    
    //HAL_FLASH_Unlock(); 
    
  
    
    __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;       		//�������ͣ��������� 
    FlashEraseInit.Sector= Bsp_GetFlashSector(address);             //Ҫ����������
    FlashEraseInit.NbSectors=1;                             		//һ��ֻ����һ������
    FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;      		//��ѹ��Χ��VCC=2.7~3.6V֮��!!
    status = HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError);
    FLASH_WaitForLastOperation(FLASH_WAITETIME);                    //�ȴ��ϴβ������
    //HAL_FLASH_Lock();  
    
    return status == HAL_OK;
}

bool Bsp_FlashProgram(uint32_t address,uint32_t *pBuffer,uint16_t lengthOfByte) {
    
    bool ret = false;
    
    //HAL_FLASH_Unlock(); 

    if(FLASH_WaitForLastOperation(FLASH_WAITETIME) == HAL_OK)            //�ȴ��ϴβ������
	{
        uint32_t programAddress = address;
        
        uint32_t endAddress = address + lengthOfByte;
        while(1) {
            
            if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,programAddress,*pBuffer) != HAL_OK){ 
                    
                __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
				ret = false;
                break;	//д���쳣
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


