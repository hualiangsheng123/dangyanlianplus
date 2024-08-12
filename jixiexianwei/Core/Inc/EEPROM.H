#ifndef _EEPROM_H_
#define _EEPROM_H_
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#define STARTADDR               (FLASH_BASE + (31*0x00000800U))   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (STARTADDR + 24)   /* End @ of user Flash area */ 
#define FLASH_USER_PageAddr     0x001f                                     //31页
/////////////////////////////////////////////////////////////////////////
unsigned int STMFLASH_ReadWord(unsigned int faddr);
void STMFLASH_Write(unsigned int WriteAddr,unsigned int *pBuffer,unsigned int NumToWrite);
void STMFLASH_Read(unsigned int ReadAddr,unsigned int *pBuffer,unsigned int NumToRead);
void  EEReadStruct(void* pBuf,unsigned int cLen,uint32_t Addr);         //读取结构体
void EEWriteStruct(void* pBuf,unsigned int cLen,unsigned int iAddr);
#ifdef __cplusplus
}
#endif
#endif 

