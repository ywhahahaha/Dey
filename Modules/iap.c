/*
 * iap.c
 *
 *  Created on: Sep 6, 2021
 *      Author: weiou
 */
#include <string.h>
#include <stdbool.h>
#include "crc_check.h"
#include "main.h"
#include "iap.h"
#include "bsp_flash.h"
#include "storage.h"
/*
00:接收正常
01:帧序号错误
02:帧校验错误
03:文件校验错误
04:升级包过大
05:数据包过大
06:写文件错误
*/
typedef enum {
	IAP_ERR_OK = 0,
	IAP_ERR_SER,
	IAP_ERR_CRC,
	IAP_ERR_FILE_CRC,
	IAP_ERR_FILE_SIZE_ERR,
	IAP_ERR_PACKAGE_ERR,
	IAP_ERR_WRITE_ERR,

}IAP_ERRCODE_t;

typedef struct {
	uint32_t       file_crc32; //文件CRC32校验
	uint32_t       file_length;//文件长度
	uint32_t       file_sum;   //文件校验和
	uint16_t       total_frame;//总帧数
	uint16_t       frame_size; //帧大小

	uint16_t       frame_count;
	uint32_t       file_sum_count;
	uint32_t       file_length_count;
    
    bool           upgrading;
	
	IAP_ERRCODE_t  iap_err_code;//错误代码
}IAP_Infor_t;

typedef struct {
	uint8_t  result;
	uint16_t version;
}IAP_DATA_t;



IAP_Infor_t iap_infor = {0};


IAP_ERRCODE_t IAP_GetErr(void) {
	return iap_infor.iap_err_code;
}

void IAP_SetErr(IAP_ERRCODE_t code) {
	iap_infor.iap_err_code = code;
}

void IAP_EraseAppBack(void) {
    storage_erasesector(ADDR_FLASH_SECTOR_7);
    storage_erasesector(ADDR_FLASH_SECTOR_8);
    storage_erasesector(ADDR_FLASH_SECTOR_9);
}

bool IAP_UpgradeOK(void) {
	if(iap_infor.frame_count == 0) {
		return false;
	}
	return iap_infor.frame_count == iap_infor.total_frame;
}



FILE_CHECK_t file_check = {0};

uint8_t *pFileRdBuf = NULL;

int IAP_GetPackSize(void) {
    return iap_infor.frame_size;
}

errStatus_t IAP_Init(uint32_t file_size,uint32_t file_sum,uint16_t pack_size,uint8_t erase) {

    if(file_size > APP_SIZE) {
        return errErr;
    }
    
    if(pack_size > FRAME_BUFF_SIZE) {
        return errErr;
    }

    if(pFileRdBuf == NULL) {
        pFileRdBuf = sys_malloc(FRAME_BUFF_SIZE);
        if(pFileRdBuf == NULL) {
            return errErr;
        }
    }

    memset(&file_check,0,sizeof(FILE_CHECK_t));
    memset(&iap_infor,0,sizeof(IAP_Infor_t));

    iap_infor.file_length = file_size;
    iap_infor.file_sum = file_sum;
    iap_infor.frame_size = pack_size;

    if(erase) {
        IAP_EraseAppBack();
    }
    

    return errOK;

}

void IAP_Deinit(void) {
    if(pFileRdBuf != NULL) {
        sys_free(pFileRdBuf);
        pFileRdBuf = NULL;
    }
    memset(&file_check,0,sizeof(FILE_CHECK_t));
}

bool IAP_GetStatus(void) {
    return iap_infor.upgrading;
}

void IAP_SetStatus(void) {
    iap_infor.upgrading = true;
}

void IAP_ClearStatus(void) {
    iap_infor.upgrading = false;
}

bool IAP_isAppValid(void) {

    uint32_t readStartAddress = APP_ADDRESS;
    uint32_t readLength = FRAME_BUFF_SIZE;
    int32_t remainLength = iap_infor.file_length;
    uint32_t checkSum = 0;
    
   

    do {
        memset(pFileRdBuf,0,FRAME_BUFF_SIZE);
        readLength = (remainLength > FRAME_BUFF_SIZE) ? FRAME_BUFF_SIZE : remainLength;
        //hal_flash_read_bytes(readStartAddress,pFileRdBuf,readLength);
        readStartAddress += readLength;
        for(uint16_t i=0;i<readLength;i++) {
            checkSum += pFileRdBuf[i];
        }
        remainLength -= readLength;

    } while(remainLength > 0);

    return checkSum == iap_infor.file_sum;
}

bool IAP_FlashCheck(void) {
    if(iap_infor.file_length == 0 || iap_infor.file_length > APP_SIZE ||
        iap_infor.file_sum == 0xffffffff) {
        return false;
    }
        
    if(pFileRdBuf == NULL) {
        pFileRdBuf = sys_malloc(FRAME_BUFF_SIZE);
        if(pFileRdBuf == NULL) {
            return false;
        }
    }
    
    IAP_Infor_t temp_infor = {0}; 
    
    storage_readbytes(APP_BACK_ADDRESS,pFileRdBuf,FRAME_BUFF_SIZE);
    
     temp_infor.file_length =  (uint32_t)pFileRdBuf[24] << 24 \
                                |(uint32_t)pFileRdBuf[25] << 16 \
                                |(uint32_t)pFileRdBuf[26] << 8 \
                                |(uint32_t)pFileRdBuf[27];
                    
     temp_infor.file_sum = (uint32_t)pFileRdBuf[28] << 24 \
                                |(uint32_t)pFileRdBuf[29] << 16 \
                                |(uint32_t)pFileRdBuf[30] << 8 \
                                |(uint32_t)pFileRdBuf[31];
    
    if(temp_infor.file_length != iap_infor.file_length) {
        return false;
    }
    
    if(temp_infor.file_sum != iap_infor.file_sum) {
        return false;
    }
                    
    uint32_t readStartAddress = APP_BACK_ADDRESS + 1024;
    uint32_t readLength = FRAME_BUFF_SIZE;
    int32_t remainLength = iap_infor.file_length;
    uint32_t checkSum = 0;

    do {
        memset(pFileRdBuf,0,FRAME_BUFF_SIZE);
        readLength = (remainLength > FRAME_BUFF_SIZE) ? FRAME_BUFF_SIZE : remainLength;
        storage_readbytes(readStartAddress,pFileRdBuf,readLength);
        readStartAddress += readLength;
        for(uint16_t i=0;i<readLength;i++) {
            checkSum += pFileRdBuf[i];
        }
        remainLength -= readLength;

    } while(remainLength > 0);

    return checkSum == iap_infor.file_sum;
}

bool IAP_WriteCheckInfor(void) {
    
	memset(&file_check,0,sizeof(FILE_CHECK_t));
	
	file_check.magic = 0x55AA050A;

	file_check.file_checksum1 = iap_infor.file_sum;
	file_check.file_checksum2 = iap_infor.file_sum;
	file_check.file_length1 = iap_infor.file_length;
	file_check.file_length2 = iap_infor.file_length;

	file_check.crc32 = CRC_Get32((uint8_t *)&file_check.magic,sizeof(FILE_CHECK_t)-4);
	
	//hal_eeprom_erase(ADDR_UPGRADE_INFOR, FLASH_EEPROM_SECTOR_SIZE);
	//hal_eeprom_write_bytes(ADDR_UPGRADE_INFOR,(uint8_t *)&file_check,sizeof(FILE_CHECK_t));

	return true;
}

void command_msg_put(SysCmd_t command,uint8_t *pdata,uint16_t length);
errStatus_t IAP_Process(uint8_t *pdata,uint16_t length,uint16_t pack) {

    if(pack < iap_infor.frame_count) {
        return errOK;
    }
    
    if(pack > iap_infor.frame_count) {
        return errErr;
    }

    if(pack > 0) {
        for(uint16_t index =0; index < length; index++) {
            iap_infor.file_sum_count += pdata[index];
        }
        
        iap_infor.file_length_count += length;
    }
    
    if(iap_infor.file_length_count > APP_SIZE) {
        return errErr;
    }
    
    uint32_t write_addr = APP_BACK_ADDRESS + iap_infor.frame_count * iap_infor.frame_size;

    storage_writebytes(write_addr,pdata,length);
    
    iap_infor.frame_count++;

    if(iap_infor.file_length_count >= iap_infor.file_length) {

        if(iap_infor.file_sum == iap_infor.file_sum_count) {
            if(IAP_FlashCheck()) {
                
                command_msg_put(SysCmdReset,NULL,0);
                
                return errOK;
            } 
        }

        return errErr;
    }

    return errOK;
}
