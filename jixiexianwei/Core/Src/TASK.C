/*********************************************************************************************************
    FILE:       TASK.C

*********************************************************************************************************/

#include "task.h" 
//任务数据库
TASK_DATA TaskDb[]={                                                                
        {1, TASK_MODE_PERIOD, 10L,  10L,    TASK_TIMER,     TaskTimer} ,     
        {1, TASK_MODE_PERIOD, 100L, 100L,   TASK_BLINK,     TaskBlinLed},  
        
//        {0, TASK_MODE_PERIOD, 10L,  10L,    TASK_BEEP,      TaskBeep},      
        {1, TASK_MODE_PERIOD, 10L,   300L,     TASK_ANJIAN,   Task_Anjian_Check},   
        {1, TASK_MODE_PERIOD, 10L,  10L,    TASK_KEY,       TaskScanKey},
        
        {0, TASK_MODE_PERIOD, 50L,  50L,    TASK_WARN,      TaskWarn},
        {0, TASK_MODE_PERIOD, 50L,  50L,    TASK_STOPMOTOR, TaskStopMotor},      
        {1, TASK_MODE_PERIOD, 10L,  10L,    TASK_STATE,     TaskStateCheck},   
//        {1, TASK_MODE_PERIOD, 1000L,1000L,  TASK_CHECK_MOTOR,Task_Check_Motor},    
//        {1, TASK_MODE_PERIOD, 2L,   2L,     TASK_FREQ2,     TaskFREQMotor},
        {0, TASK_MODE_PERIOD, 1000L,1000L,  TASK_FEED,      Feed}, 
        
        {1, TASK_MODE_PERIOD, 1000L,1000L,  TASK_POWER_CHECK,Task_Power_Check},     
        {1, TASK_MODE_PERIOD, 100L, 100L,   TASK_CMMD,      TaskCommand},     
        {1, TASK_MODE_PERIOD, 100L, 100L,   TASK_CHARGE,    TaskChargeBat},
        {1, TASK_MODE_PERIOD, 10L,  10L,    TASK_CHECK_COIL,TaskCheckBrakeCoil},        
        {1, TASK_MODE_PERIOD, 500L, 500L,   TASK_SENSOR,    TaskCheckSensor},           
        {1, TASK_MODE_PERIOD, 500L, 500L,   TASK_WDT,       Task_WDT}, 
        
//        {1, TASK_MODE_PERIOD, DC_DN_PR,DC_DN_PR,TASK_BREAK, Task_DC_Brake_down},   
        //增加一个扫描IO口的任务用来锁定输出
        };
TIMER_DATA TimerData[MAX_TIMER];
float CPU_Usage = 0;

const char LeapDays[14]={0,31,29,31,30,31,30,31,31,30,31,30,31,31};
const char NormDays[14]={0,31,28,31,30,31,30,31,31,30,31,30,31,31};
///////////////////////////////////////////////////////////////////////////////////////////////////////////

 
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/**********************************************************************************************************
    任务调度,在主循环中调用。
    由于是模拟多任务，所以要求每个任务的处理时间尽可能短。
***********************************************************************************************************/

unsigned long RunTimer =0  ;
void idle(void) 
{
    /*
    __NOP();        
    __NOP();
    //while((0x1<<16)&FLASH->STAT);
    SYS->SLEEP |= (1 << SYS_SLEEP_SLEEP_Pos);      //进入睡眠模式  
    __NOP();
    __NOP();
    //while((SYS->TWKCR & SYS_TWKCR_ST_Msk) == 0);    //等待唤醒条件
    //SYS->TWKCR |= (1 << SYS_TWKCR_ST_Msk);          //清除唤醒状态 
    __NOP();
    __NOP();
    */
}     

void TaskProc(void)
{
    unsigned char i;

    if(!gc1ms)  //保证1ms扫描一次
    {
        idle();
    }
    else
    {
        if(TMP1msCount>0)
        {
            for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
            {
                TaskDb[i].lLastTime-=TMP1msCount;
            }
        }
        gc1ms=0;
        TMP1msCount = 0;
        for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
        {
            if(!TaskDb[i].cActive)
                continue;

            if(--TaskDb[i].lLastTime<=0)
            {
                TaskDb[i].lLastTime += TaskDb[i].lPeriod;
                TaskDb[i].TaskProc();                                 //执行任务
            }
        }
        //计数器的值/重装值=CPU占用率
        CPU_Usage =   (SysTick->VAL*100.0f)/SysTick->LOAD; ; 
    }
}
/**********************************************************************************************************/
void ActiveTask(UCHAR cTaskType, ULONG lPeriod)
{
    unsigned int i;

    for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
    {
        if(TaskDb[i].cType==cTaskType)
        {
            if(TaskDb[i].cActive==0)
            {
                TaskDb[i].cActive=1;
                TaskDb[i].lPeriod=lPeriod;
                TaskDb[i].lLastTime=lPeriod;
            }
            break;
        }
    }
}
/***********************************************************************************************************/
void SuspendTask(UCHAR cTaskType)
{
    unsigned int i;
    for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
    {
        if(TaskDb[i].cType==cTaskType)
        {
            TaskDb[i].cActive=0;
            break;
        }
    }
}


void StartTimer2(int8_t *TimerID,UCHAR cMode,ULONG lPeriod,void (* routine)(void *   Param1),void *   Param1)
{
    *TimerID = StartTimer(cMode,lPeriod,routine,Param1); 
    if(*TimerID >=0)
        TimerData[*TimerID].TimerID = TimerID   ;
}

/*********************************************************************************************************
    FUNCTION:   StartTimer
    PURPOSE:    start a timer ok then return timer number ; =-1 failed;
**********************************************************************************************************/
SCHAR StartTimer(UCHAR cMode,ULONG lPeriod,void (* routine)(void *   Param1),void *   Param1)
{
    unsigned char   i;
    TIMER_DATA *pt;
    pt=&TimerData[0];
    for(i=0;i<MAX_TIMER;i++,pt++)
    {
        if(!pt->cUse)
        {
            switch(cMode)
            {
                case TIMER_MODE_TIMER:              // timer only
                case TIMER_MODE_ONCEROUTINE:        // timely routine
                case TIMER_MODE_CYCROUTINE:         // cyclely routine
                    break;
                default:
                    pt->cUse = 0;
                    return -1;                      // error
            }
            pt->TimerID     = 0;
            pt->lPeriod     = lPeriod;              // set timer period
            pt->lLastTime   = lPeriod;                // get current system time as start timer
            pt->routine     = routine;              // user timer's routine
            pt->cMode       = cMode;                // timer's work mode
            pt->cUse        = 1;
            pt->Param1      = Param1;                
            #ifdef _DEBUG
            //printf("定时器%d申请成功\n",i);
            #endif
            return (i);                             // return timer number
        }
    }
    //if(i== MAX_TIMER);
    #ifdef _DEBUG
    printf("定时器资源不足,申请不成功");
    #endif
    return (-1);                                    // no timer available
}				
/**********************************************************************************************************
    FUNCTION:   StopTimer
    PURPOSE:    stop timer count
***********************************************************************************************************/
signed char StopTimer ( signed char * pcTimerNumber )
{
    if(*pcTimerNumber>=MAX_TIMER || *pcTimerNumber==-1)
        return -1;                            // timer number error

    TimerData[*pcTimerNumber].cUse=0;    
    #ifdef _DEBUG
    //printf("定时器%d停止成功\n",*pcTimerNumber);
    #endif
    *pcTimerNumber = -1;
    return 0;
}
/**********************************************************************************************************
    FUNCTION:   TaskTimer
    PURPOSE:    timer service routine.
***********************************************************************************************************/
void TaskTimer(void)
{
    TIMER_DATA *pt;
    unsigned char i;

    pt=&TimerData[0];                                         // pointer to the first timer
    for(i=0;i<MAX_TIMER;i++,pt++)
    {
        if(!pt->cUse)
            continue;
        if((--(pt->lLastTime))<=0)
        {
                
            switch(pt->cMode)
            {
                case TIMER_MODE_TIMER:                      // timer only
                     pt->cUse=0;
                    break;
                case TIMER_MODE_CYCROUTINE:                   // cyclely routine
                     pt->lLastTime+=pt->lPeriod;
                     pt->routine(pt->Param1);                          // start user's routine
                    break;
                case TIMER_MODE_ONCEROUTINE:                // timer routine once
                     pt->cUse=0;                            //防止在routine里stoptimer后启动timer造成错误停止   
                    if(pt->TimerID!=NULL)
                    {
                        *pt->TimerID = -1;
                    }
                     pt->routine(pt->Param1);               // start user routine
                     break;
                default:
                    pt->cUse=0;
                    break;
            }
            
        }
    }
}
void Start_OneLine(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	 DelayUs(10000);  
}
void SendByte_Oneline(unsigned char addr)
{
	unsigned int i;
	for(i=0;i<8;i++)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
 	if(addr&0X01)
{
       DelayUs(1800);                        /*>2400us*/
     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
       DelayUs(600);                           /*>800us*/
}
else
{
      DelayUs(600);                         /*>800us*/
 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
DelayUs(1800);                          /*>2400us*/
}	
 addr=addr>>1;                             /*地址值右移一位*/
}
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

void send_data(unsigned char addr)
{
	unsigned int i;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_Delay(5);  
for(i=0;i<8;i++)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
 	if(addr&0X01)
{
        HAL_Delay(3);                        /*>2400us*/
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
        HAL_Delay(1);                          /*>800us*/
}
else
{
        HAL_Delay(1);                         /*>800us*/
 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_Delay(3);                          /*>2400us*/
}	
 addr=addr>>1;                             /*地址值右移一位*/
}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	
}
extern IWDG_HandleTypeDef hiwdg;
void TaskBlinLed(void)
{   
	 
    static unsigned char count =0;  
//   static int16_t num=0	;
//	  num++;
	    if(HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
    {
      /* Refresh Error */
      Error_Handler();
    }
    count=(++count)%10;    
    if(count==0)
    {  
       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
		}
    else
    {  
       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
    }

	
	//    PIN_LED = count>0?1:0;     
//    FeedRun = !PIN_LED;
    //sinvol(0); 
}
int TaskState(UCHAR cTaskType)
{
    unsigned int i; 
    for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
    {
        if(TaskDb[i].cType==cTaskType)
        {   
            return  TaskDb[i].cActive;
        //    break;
        }
    } 
    return 0;
}

void SetTaskLastTime(UCHAR cTaskType,ULONG lLastTime)
{
    unsigned int i; 
    for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
    {
        if(TaskDb[i].cType==cTaskType)
        {  
            TaskDb[i].lLastTime=lLastTime; 
            break;
        }
    }
}

void ActiveTaskNow(UCHAR cTaskType, ULONG lPeriod)
{
    unsigned int i;

    for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
    {
        if(TaskDb[i].cType==cTaskType)
        {
            if(TaskDb[i].cActive==0)
            {
                TaskDb[i].cActive=1;
                TaskDb[i].lPeriod=lPeriod;
                TaskDb[i].lLastTime=0;
            }
            break;
        }
    }
}
 extern int32_t AD_Value[];
#define ANJIAN              AD_Value[2]
#define YUANCHENG           AD_Value[3]
	  extern CHILD_IED_STRUCT CHILD[3];
 void Task_Anjian_Check(void)
 {
//	  static unsigned char lock_state;
//		if(LOCK_CHECK==1)
//		{
//			if(lock_state==0)
//			{
//				lock_state=1;
//				send_data(0x10);
//			}
//		}
//		else
//		{
//			if(lock_state==1)
//			{
//				lock_state=0;
//				send_data(0x0f);
//			}
//			
//		}
		if(((AD_Value[2]>3305)&&(AD_Value[2]<3425))||((AD_Value[2]>3697)&&(AD_Value[2]<3817)))
		{
			send_data(0x0d);
		}
	// set_scan()	;
  // RF_operate();
	 switch (ADC1_KeySample()) 
	 {
		 
		 case 1:
			     (CHILD[0].Key)= KEY_UP;      
		          break;
		 case 2: 
			 (CHILD[0].Key)= KEY_STOP;      
		      break;
		 case 3: 
			      (CHILD[0].Key)= KEY_DOWN;     
           		 break;
		 case 4:  (CHILD[0].Key)= KEY_MUTE;     break;
		 case 5:  (CHILD[0].Key)= KEY_STOPWARN;     break;
		 case 6:  (CHILD[0].Key)= KEY_SELFTEST;     break;
     case 7: 
			      (CHILD[0].Key)= KEY_DOWN;  break;
		 default :(CHILD[0].Key)=0;     break;//(CHILD[0].Key)=0;  
	 }
	 switch (ADC2_KeySample()) 
	 {
		 case 1:(CHILD[0].Key)= KEY_UP_yuancheng;      break;
		 case 2: (CHILD[0].Key)= KEY_STOP_yuancheng;      break;
		 case 3: (CHILD[0].Key)= KEY_DOWN_yuancheng;      break;
		  case 4:     break;
		 case 5:       break;
		 case 6:      break;
     case 7: 
			      (CHILD[0].Key)= KEY_DOWN_yuancheng;  break;
		default :    break;//(CHILD[0].Key)=0;  
	 }
 }

int32_t Sample_Volt[] = {2042UL,2832UL,3150UL,100UL,1758UL,2559UL,3510UL};//child[0].key:0.45V/558 -上，0.83V/830-停，1.47V/1824-下，1.65V/2048-锁  0.32v/397-停+上 0.485/782-停+下
                                                                         //child[1].key: 2.07v/2569-上，2.83v/3512-停,3.05v/3786-下，0v/50-锁 1.88v/2333-停+上 2.64/3277-停+下

 int ADC1_KeySample(void)
{
	int i;
	//uint16_t  KeyVolt = filter(ADC1_CHANNEL_3,ADC1_SCHMITTTRIG_CHANNEL3);//ADC1_GetVolt为采集到按键电压函数，如果是八位单片机考虑浮点运算压力，可以使用采集AD模拟量来进行判断
	
     for( i = 0;i < 7;i++)
      {
 		     if(( ANJIAN >= (Sample_Volt[i] - 100UL)) && (ANJIAN <= (Sample_Volt[i] + 100UL)))//允许电压偏差正负0.2V，但还是会存在由于电源不稳导致误判断
            {
							DelayMS(30);
						if(( ANJIAN >= (Sample_Volt[i] - 100UL)) && (ANJIAN <= (Sample_Volt[i] + 100UL)))//允许电压偏差正负0.2V，但还是会存在由于电源不稳导致误判断
						{
              return i + 1;//返回按键数
            }
				   	}
                              
      }
     return 0;//返回0：无按键按下

}
int ADC2_KeySample(void)
{
	int i;
	//uint16_t  KeyVolt = filter(ADC1_CHANNEL_3,ADC1_SCHMITTTRIG_CHANNEL3);//ADC1_GetVolt为采集到按键电压函数，如果是八位单片机考虑浮点运算压力，可以使用采集AD模拟量来进行判断
	
     for( i = 0;i < 7;i++)
      {
 		     if(( YUANCHENG >= (Sample_Volt[i] - 100UL)) && (YUANCHENG <= (Sample_Volt[i] + 100UL)))//允许电压偏差正负0.2V，但还是会存在由于电源不稳导致误判断
            {
							DelayMS(30);
						if(( YUANCHENG >= (Sample_Volt[i] - 100UL)) && (YUANCHENG <= (Sample_Volt[i] + 100UL)))//允许电压偏差正负0.2V，但还是会存在由于电源不稳导致误判断
						{
              return i + 1;//返回按键数
            }
				   	}
                              
      }
     return 0;//返回0：无按键按下

}
/**
  * @brief This function handles System tick timer.
  */
UCHAR gc1ms = 0;                //1ms到标志，控制多任务处理的时间隔为1ms
UCHAR TMP1msCount = 0;
__attribute__((section("RAMCODE")))
void SysTick_Handler(void)
{
   if(gc1ms==1)
    {
        TMP1msCount++;
    }              
    gc1ms=1;   
    RunTimer++; 
    HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}
//阻塞等待 us
//等待us
void DelayUs(uint32_t us)
{      
    int32_t t =0;
    uint32_t ms=us/1000; 
    int32_t delayTick =  ((us%1000)*64);
    while(ms--){ DelayMS(1);}
    t=SysTick->VAL-delayTick; 
    if(t<0)
    {
        t+=SysTick->LOAD;   
        while((SysTick->CTRL&SysTick_CTRL_COUNTFLAG_Msk) == 0);//等待COUNTFLAG标志位置1
    }
    while(SysTick->VAL>t){;}
}      
//阻塞等待 ms
//等待1mS
void DelayMS(uint32_t Delay)
{   
    uint32_t tickstart = 0;
    tickstart =  HAL_GetTick();
    while((HAL_GetTick() - tickstart) <  Delay)
    {
    }
}
