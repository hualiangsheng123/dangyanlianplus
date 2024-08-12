#include "EEprom.h" 
/****************************************************************************************************************************************** 
* 函数名称:	FLASH_Read()
* 功能说明:	FLASH数据读取
* 输    入: uint32_t addr		  要读取的数据在Flash中的地址，字对齐
*			      uint32_t buff[]		读取到的数据存入buff指向的内存
*			      uint32_t cnt		  要读取的数据的个数，以字为单位
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(volatile unsigned long*)faddr; 
}
void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)	
{ 
	FLASH_EraseInitTypeDef FlashEraseInit;
	HAL_StatusTypeDef FlashStatus=HAL_OK;
	u32 PageError=0;
	u32 addrx=0;
	u32 endaddr=0;	
	if(WriteAddr<FLASH_BASE ||WriteAddr%4)return;//
    
	HAL_FLASH_Unlock();             //
	addrx=WriteAddr;				//
	endaddr=WriteAddr+NumToWrite*8;	//
	if(addrx<0X08010000)
	{
		while(addrx<endaddr)		//
		{
			 if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)	//
			 {   
				FlashEraseInit.TypeErase=FLASH_TYPEERASE_PAGES;    // 
//				FlashEraseInit.Banks=4;
				FlashEraseInit.Page=31;   						   //
				FlashEraseInit.NbPages=1;                          //
				if(HAL_FLASHEx_Erase(&FlashEraseInit,&PageError)==HAL_OK) 
				{
					break;	
				}
				else
				{
					send_data(0x00);
					return;
				}
			}
			 else addrx+=4;
			FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE );            //
		}
	FlashStatus=FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE );        //
	if(FlashStatus==HAL_OK)
	{
		 while(WriteAddr<endaddr)									//???
		 {
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,WriteAddr,*(uint64_t*) pBuffer)!=HAL_OK)//????
			{ 
				break;												//????
			}
			WriteAddr+=8;
			pBuffer+=2;
		}  
	}
}
	HAL_FLASH_Lock();           									//??
} 

void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)   		//????
{
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);	//??4???.
		ReadAddr+=4;												//??4???.	
	}
}
void  EEReadStruct(void* pBuf,u32 cLen,u32  Addr)
{
 //   UINT i;
	  u32 Addr1;
    u32 *pcBuf = (u32 *)pBuf; 
     Addr1=Addr+STARTADDR;
    if(cLen%4!=0){
       // printf("不整除");
        return;
    }
    
    STMFLASH_Read(Addr1,(u32*)pcBuf,cLen/4);   
}
 void EEWriteStruct(void* pBuf,u32 cLen,u32 iAddr)     
{    
    u32 Addr1;
    u32 *pcBuf = (u32 *)pBuf; 
     Addr1=iAddr+STARTADDR;	
    STMFLASH_Write(Addr1,pcBuf,cLen);
}
