/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "gpio.h"
#include "tim.h"
extern unsigned int power_on;
 __IO uint8_t ubDmaTransferStatus = 2; /* Variable set into DMA interruption callback */
 __IO   uint16_t   aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE];
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TIM_HandleTypeDef htim3;
/* USER CODE END PV */
unsigned int write_eeprom_flag;
 extern int32_t AD_Value[];
 extern  unsigned int send_data_flag;
 extern  unsigned int send_end_timer;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	int delay = 500;
    int i = 0;  
  /* USER CODE BEGIN 1 */
 __disable_irq();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
    MX_GPIO_Init();
	  CPU_POWER__ON;
	//	MAINPOWER__ON ;
	  MX_DMA_Init();
	  MX_ADC1_Init();
	//	  MX_TIM14_Init();
    enable();		 
	 StopMotor();
	 	power_on=0;
	 for(i=0;i<2000;i++) 
    {
        DelayUs(delay);  
        TaskBlinLed();
    } 
	
	 EnableBattary(); 		
   InitInvert(0);  
	 InitIED();   
   GetCurretPos();
	 StopDC2ACConvert(0); 
	 InitKey();
   ActiveTaskCheckPhase(); 
 //   InitModbus();  
   
    WriteLEDInfo(0);
 //   GetChildState();   
    
//    Init_task_timer(); 
 
 MX_TIM3_Init();
	if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
  {
    /* Starting Error */
    Error_Handler();
  }
  
  /* Initialize interrupts */
//  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	  enable(); 
		send_data(0x00);
		HAL_Delay(100);
	  checkUserMode();   
	  send_data(0x00);
	//   MAINPOWER__OFF ;
	  HAL_Delay(2000);
//		  send_data(0x0f);
//		  send_data(0x10);
//		HAL_Delay(6000);
//	   	send_data(0x00);
//	    send_data(0x00);
////	    send_data_flag=1;
//			 send_end_timer=60;
//	
    SetLock(0);
		SelfTest();
 if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
 {
    /* Calibration Error */
   Error_Handler();
  }
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)AD_Value, 7);
//		if (HAL_ADC_Start_DMA(&hadc1,
//                        (uint32_t *)aADCxConvertedData,
//                        ADC_CONVERTED_DATA_BUFFER_SIZE
//                       ) != HAL_OK)
//  {
//    /* ADC conversion start error */
//    Error_Handler();
//  }  	
    /* Wait for ADC conversion and DMA transfer completion (update of variable ubDmaTransferStatus) */
		MX_IWDG_Init(); 
	  power_on=1; 
  while (1)
  {
    /* USER CODE END WHILE */
		
   TaskProc();
	if((FLASH->SR & FLASH_SR_CFGBSY) == 0x00U)
	{
		if(write_eeprom_flag==1)
			 {
		  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGAERR);
			 __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGSERR);
				  EEWriteStruct(&TimeOut,sizeof(TimeOut),0);
				 	 write_eeprom_flag=0;
		//		  EEReadStruct(&TimeOut,sizeof(TimeOut),0);
			
				
	    }		 
   
	 }
		
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}
/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1599;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}


/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
//void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
//{
//  if(htim_base->Instance==TIM3)
//  {
//  /* USER CODE BEGIN TIM2_MspInit 0 */

//  /* USER CODE END TIM2_MspInit 0 */
//    /* Peripheral clock enable */
//    __HAL_RCC_TIM3_CLK_ENABLE();
//    /* TIM2 interrupt Init */
//    HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
//    HAL_NVIC_EnableIRQ(TIM3_IRQn);
//  /* USER CODE BEGIN TIM2_MspInit 1 */

//  /* USER CODE END TIM2_MspInit 1 */
//  }

//}

#define TOPTOMID   1000 
void InitIED(void)
{
    //0x3E000
//    char *pChar;
//    int i;            
    #ifdef _DEBUG
    printf("读取参数 \n");
    #endif                                     
    IED.MidWaitTimerID = -1;
    IED.MotorMoveDelayTimerID = -1;
     /*  
    pChar = (char*)&TimeOut;
    for(i=0;i<sizeof(TimeOut);i++)
    {
        pChar[i]=i;    
    } 
    EEWriteStruct(&TimeOut,sizeof(TimeOut),0);
    for(i=0;i<sizeof(TimeOut);i++)
    {
        pChar[i]=0;    
    } 
    */ 
    //测试写入代码
    IED.TimerS          = TOPTOMID +1;             //电机不知道在什么位置,一般会到顶
    IED.TimeOutTimer    = 8000;                    //超时定时器
    IED.Err             |= ErrPhase;       
    IED.Motor_Cmd       = POS_NONE;
    IED.cPos            = POS_UNKNOWN;
    IED.OverRollTime    =  0x4FFF;         
    //开机没有超时.只有到位传感器正常,才会恢复超时设置
//    STMFLASH_Read(STARTADDR,&TimeOut,sizeof(TimeOut));//
     EEReadStruct(&TimeOut,sizeof(TimeOut),0);
//		  EEWriteStruct(&TimeOut,sizeof(TimeOut),0); 
//			 EEReadStruct(&TimeOut,sizeof(TimeOut),0);
//			  TimeOut.TimeTopToMid = 100; //50X50=2.5S
//        TimeOut.TimeTopToMid2 = TimeOut.TimeTopToMid+10 ;
//        TimeOut.TimeTopToBot = 0x7FFF ; 
//        //TimeOut.TimeBotToTop = TimeOut.TimeTopToBot*14/10 ;
//        TimeOut.MidTimeWait  = 1500  ;        //30S 
//        TimeOut.SlideTimeOutTimer = 3;    
//        IED.MidTimeWait  = 1500;          //30S       
//        
//        EEWriteStruct(&TimeOut,sizeof(TimeOut),0);
//			  EEReadStruct(&TimeOut,sizeof(TimeOut),0);
//				 EEReadStruct(&TimeOut,sizeof(TimeOut),0);
    IED.SlideTimeOutTimer = 0;        //超时定时器
  
    if((TimeOut.TimeTopToMid!=0xFFFFFFFF)
        &&(TimeOut.TimeTopToMid!=0)
        &&(TimeOut.TimeTopToMid!=0))
    {
        #ifdef _DEBUG
        printf("成功\n");
        #endif
    }  
    
    else
    {
        #ifdef _DEBUG
        printf("失败，使用默认参数\n  ");
        #endif

        TimeOut.TimeTopToMid = 50; //50X50=2.5S
        TimeOut.TimeTopToMid2 = TimeOut.TimeTopToMid+10 ;
        TimeOut.TimeTopToBot = 0x7FFF ; 
        //TimeOut.TimeBotToTop = TimeOut.TimeTopToBot*14/10 ;
        TimeOut.MidTimeWait  = 1500  ;        //30S 
        TimeOut.SlideTimeOutTimer = 3;    
        IED.MidTimeWait  = 1500;          //30S        
        
        EEWriteStruct(&TimeOut,sizeof(TimeOut),0);
    }
    
            
        IED.TimeTopToMid2 = TimeOut.TimeTopToMid2 ;
        IED.TimeTopToMid = TimeOut.TimeTopToMid ;
        IED.TimeTopToBot = TimeOut.TimeTopToBot ; 
        //IED.TimeBotToTop = TimeOut.TimeBotToTop ;
        IED.MidTimeWait  = TimeOut.MidTimeWait  ;     
        
        IED.SlideTimeTopToMid = TimeOut.SlideTimeOutTimer ;    ;
    
    #ifdef _DEBUG
        printf(("顶部到中部时间%ld,\n 暂停时间%ld,\n "),IED.TimeTopToMid,IED.MidTimeWait );
        printf(("顶部到底时间%ld\n"),IED.TimeTopToBot);   
        
        printf(("顶部到中部滑行时间%ld,\n "),TimeOut.SlideTimeOutTimer );
    #endif
}



/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  /* Update status variable of DMA transfer */
  ubDmaTransferStatus = 1;  

  /* Set LED depending on DMA transfer status */
  /* - Turn-on if DMA transfer is completed */
  /* - Turn-off if DMA transfer is not completed */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
extern unsigned char MAIN_POWER_SOUND;
extern unsigned char BAT_SOUND;
extern unsigned char MOTOR_LINE_SOUND;
extern unsigned char COIL_SOUND;
extern unsigned char TOP_OVERROLL_SOUND;
extern unsigned char SMOKE_TEMP_SOUND;
extern unsigned int send_data_flag;
extern unsigned int send_end_timer;
extern unsigned char anjian_flag;
extern unsigned char sound_feed_on;
uint16_t m,n,p;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  	m++;

//	if(halfdown_life==1)
//	{
//	  p++;
//		if(p>=60)
//		{
//			halfdown_life=0;
//			p=0;
//		}
//	}
//	 if((IED.cMoving==1)&&(IED.Err&(MAIN_POWER_ERROR|ErrPhase))==1)
//	 {
//		 run_time++;
//		 if(run_time>4000)
//		 {
//			  StopMotor();
//		 }
//	 }
//	 else{run_time=0;}
	 
	if(sound_feed_on==1)
	{
			p++;
		if(p>=2000)
		{
			p=0;
   	sound_feed_on=0;
		}
	}
	if(anjian_flag==1)
	{
		n++;
		if(n>=30)
		{
			n=0;
			anjian_flag=0;
		}
	}
	if(m>=1000)
	{
		m=0;
	 MAIN_POWER_SOUND=0;
   BAT_SOUND=0;
   MOTOR_LINE_SOUND=0;
   COIL_SOUND=0;
   TOP_OVERROLL_SOUND=0;
   SMOKE_TEMP_SOUND=0;
	}
	if(send_data_flag==1)
	{
		send_end_timer--;
		if(send_end_timer<=0)
		{
			send_data_flag=0;
		}
	}
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
