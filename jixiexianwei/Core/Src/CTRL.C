/*********************************************************************************************************
    FILE:       TASK.C

*********************************************************************************************************/

#include "ctrl.h"
#define MAXDELAYCNT 5
IED_DATABASE  IED;
 unsigned char MOTOR__UP__FLAG;
 unsigned char MOTOR__DOWN__FLAG;
unsigned char MOTOR__RUN ;
unsigned char MAIN_POWER_SOUND;
unsigned char BAT_SOUND;
unsigned char MOTOR_LINE_SOUND;
unsigned char COIL_SOUND;
unsigned char TOP_OVERROLL_SOUND;
unsigned char SMOKE_TEMP_SOUND;
char CtrlHalfDown = 0;
MyTimeOut TimeOut;// __attribute__((at(0x20000500)));
CHILD_IED_STRUCT CHILD[3];

char CtrlAllDown = 0;
unsigned int power_on;
unsigned int send_data_flag;
unsigned int send_end_timer;
int MotorLineCnt = 0;//电机检测
#define FIXTIMECNT 100L   //25S      50ms*100 = 5S

//填充键盘结构体
void InitKey(void)
{
    int i=0;
    for(i=0;i<3;i++)
    {
        CHILD[i].Key = 0xFF;
    }
}
 
unsigned char gcBeepTrig = 0;
unsigned int gcNumBeep   = 0; 
//蜂鸣任务 
//A
unsigned int BeepFreq =  2400;

            


/***********************************************************
    激活蜂鸣器任务
***********************************************************/


            
unsigned char gcMotorDir = 0;            //方向控制薄码开关    
//启动电机运行,1S 后启动位置检测信号       
//#define gcPhaseCtrl
unsigned char gcPhaseCtrl,gcPrePhaseCtrl; 
//当前相序,上次相序
signed char  gcTimer_Motor_ID = -1;

//#define TIME_OVERROLL 40L 
#define TIME_OVERROLL (IED.OverRollTime)
signed char MovMotor(unsigned char cPos)
{     
    GetCurretPos();          
    
    if(IED.cMoving==1)
    { 
        if(cPos!=IED.PreMotor_Cmd)//电机运行中收到新的命令
        {
            #ifdef _DEBUG
            printf("电机正在运行,无法执行该命令\n");
            #endif
            AddCommand(cPos);  
        }
        return -1;
    }      
    _nop_();    
    if(cPos==IED.cPos)
    {
        #ifdef _DEBUG
        printf("电机已经到位,无需执行该命令\n");
        #endif                           
        return -2;
    } 
    _nop_();
    
    
    #ifdef _MCU_301 
    MOTOR__RUN = 0;
    MOTOR__RUN__CLR;		
    #elif _MCU_302 
    MOTOR__UP__CLR;
		MOTOR__UP__FLAG=0;
		MOTOR__DOWN__CLR; 
		MOTOR__DOWN__FLAG=0;
    #endif
    
    if(IED.Err&(COIL_SHORT_ERR|COIL_BREAK_ERR))     //线圈开路,不转
        return -3;            
      
 

     switch(cPos)
     {
         case POS_TOP:                                  //到顶,方向为向上
            IED.Direction=MOTOR_UP;
            IED.TimeOutTimer = 0;                     //到顶部,超时时间为0  
            //修正短期行走为0的问题   
            break;
        case POS_BOT:
            IED.Direction=MOTOR_DOWN;                 //到底,方向为向下
            IED.TimeOutTimer = IED.TimeTopToBot ;      
            //到底部,超时时间为TimeTopToBot     
            break;
        case POS_MIDDLE:                            
            IED.TimeOutTimer = IED.TimeTopToMid;;     
            //到中间,超时时间为TimeTopToMid
            if(IED.TimerS<IED.TimeTopToMid)            
            //目标位置为中位,当前位置在中位上,方向为向上
            {
                    IED.Direction=MOTOR_DOWN;
                    if(!pos_bot_bit)        //中挺位置设定超出下沿
                        return -2;
            }
             else                                    
            //目标位置为中位,当前位置在中位下,方向为向下
            {
                IED.Direction=MOTOR_UP;
                if(!pos_top_bit) //中挺位置设定超出上沿
                    return -2;
            }                      
            break;       
        case POS_MIDDLE2:  
            IED.TimeOutTimer = IED.TimeTopToMid2;;     
            //到中间,超时时间为TimeTopToMid2
            if(IED.TimerS<IED.TimeTopToMid2)            
            //目标位置为中位,当前位置在中位上,方向为向上
            {
                    IED.Direction=MOTOR_DOWN;
                    if(!pos_bot_bit)        //中挺位置设定超出下沿
                        return -2;
            }
             else                                    
            //目标位置为中位,当前位置在中位下,方向为向下
            {
                IED.Direction=MOTOR_UP;
                if(!pos_top_bit) //中挺位置设定超出上沿
                    return -2;
            }                      
            break;                       
        case POS_UNKNOWN:     
        default:
            return -5;     
     }    
     /////////////////////////////////////////////////////
     if((IED.Err&ERR_TOP_OVERROLL)&&( MOTOR_UP==IED.Direction))     //上过卷,禁止上行
            return -4;      
     if((IED.Err&ERR_BOT_OVERROLL)&&( MOTOR_DOWN==IED.Direction))         //下过卷,禁止下行
            return -4;      

    ////////////////////////////////////////////////////// 
    //上下行位置修正

    IED.PreDir=IED.Direction;
    //////////////////////////////////////////////////////
     #ifdef _DEBUG                                                 
//     printf("方向%s ",(  IED.Direction==MOTOR_UP?"向上":"向下"));
     #endif        
   
 
        
    IED.PreMotor_Cmd  = cPos;      
        //有电源错误,判断运行方向后推出
//    if(IED.Err&(ErrPhase|                         //相位错
//                ErrOverHeat|MAIN_POWER_ERROR|     //过热//无主电
//                COIL_SHORT_ERR|COIL_BREAK_ERR|      //刹车线圈坏 
//                ERR_MOTOR_LINE
//                ))
//    {            
//        AddCommand(cPos);            
//        return -4;     
//    }
    ///////////////////////////////////////////////
   // EnableBrakPower(TRUE);     
    run_vol_set();	 
    setDIR();        
    IED.RunMute = 0;
    IED.cMoving = 1;      
    //启动电机到位检测任务     
    StartTimer2(&IED.BrakeDelayTimerID ,TIMER_MODE_ONCEROUTINE,
                100L,MotorDelay,0);                                //延时启动原为20L
      
    return 0; 
}


void ErrorStopMotor(void)
{
    if(IED.cMoving)
    {
        AddCommand(IED.PreMotor_Cmd);
        #ifdef _MCU_301
        MOTOR__RUN = 0; 
        MOTOR__RUN__CLR;			
        #elif _MCU_302
          MOTOR__UP__CLR;
		MOTOR__UP__FLAG=0;
		MOTOR__DOWN__CLR; 
		MOTOR__DOWN__FLAG=0;
        #endif
        SuspendTask(TASK_STOPMOTOR);
        StopTimer(&IED.MotorMoveDelayTimerID);    
        StopTimer(&IED.BrakeDelayTimerID);        
        IED.cMoving = 0;     
        IED.cMovingBeepDelayCnt = 0; 
        #ifdef _DEBUG
        printf("有故障电机停止 !\n");      
        #endif    
    }      
}

signed char TimerReleaseDir = -1;
void ReleaseDir(void* p)
{
    #ifdef _MCU_301
    MOTOR__DIR__CLR; 
    IED.cMoving = 0; 
    #endif    
}

void StopMotor(void)
{
    
    #ifdef _MCU_301
	  MOTOR__RUN=0;
    MOTOR__RUN__CLR;    
    #elif _MCU_302
     MOTOR__UP__CLR;
		MOTOR__UP__FLAG=0;
		MOTOR__DOWN__CLR; 
		MOTOR__DOWN__FLAG=0;
    #endif     
    DeInitDC2AC(0);//停止逆变
    StopTimer(&IED.MotorMoveDelayTimerID);
    StopTimer(&IED.BrakeDelayTimerID);
    SuspendTask(TASK_STOPMOTOR);

    IED.cMovingBeepDelayCnt = 0; 
    #ifdef _DEBUG
    printf("StopMotor()电机停止 !\n");      
    #endif        
    IED.Motor_Cmd  = POS_NONE ;  
    IED.PreMotor_Cmd    = POS_NONE ;         
    EnableBrakPower(FALSE);    

    #ifdef _MCU_301    
    StartTimer2(&TimerReleaseDir,TIMER_MODE_ONCEROUTINE,50UL,ReleaseDir,0);
    #else
    IED.cMoving = 0;  
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 		
    #endif
}
void LoadOverRollTime(void)
{
            ////////////////////////////////////////用户模式超时时间为10倍
    if(KEY_MODE==USER_MODE) 
        TIME_OVERROLL = DEFAULT_OVERROLL_TIME*20L; //50S 
    else 
        TIME_OVERROLL = DEFAULT_OVERROLL_TIME*8L;             //2.5S
}
void GetCurretPos(void)
{   
           
    #ifdef _DEBUG
    //printf("Times=%d\n",IED.TimerS );
    #endif    
    if(pos_top_bit==0)
    { 
        
        IED.Err&=(~ERR_BOT_OVERROLL);  //清除下过卷错误
        IED.cPos = POS_TOP;
        IED.SlideTimeOutTimer = 0;
        IED.TimerS = 0; 
        #ifdef _DEBUG
        printf(" 行程开关到顶位,TIMES清0\n");
        #endif
        LoadOverRollTime();
    }
    else if(pos_bot_bit==0)
    {  
        IED.Err&=(~ERR_TOP_OVERROLL);//清除上过卷错误
        IED.cPos = POS_BOT; 
        
        LoadOverRollTime();
        //如果不是调试状态,timers = 到底时间
        
        if(IED.IED_State!=StateDebug)    
        {
            IED.TimerS = IED.TimeTopToBot; 
            #ifdef _DEBUG 
            printf("重新装载Timers =到底时间!\n");  
            #endif
        }
        
    }
    
    else if((IED.TimerS==IED.TimeTopToMid)&&(IED.cMoving==0))    
    {
        IED.cPos = POS_MIDDLE;
    }else if((IED.TimerS==IED.TimeTopToMid2)&&(IED.cMoving==0))    
    {        
        IED.cPos = POS_MIDDLE2;
    }
    else
        
    {
        IED.cPos = POS_UNKNOWN;
    }
    #ifdef _DEBUG
    //printf("Current POS is %d\n",(int)IED.cPos);
    #endif
    _nop_();
}

//检测是否到位,到位后停止电机
#define MOTORLINKECHECK MOTORLINKE
void TaskStopMotor(void)                                //TASK_MOTOR
{     
//报警没消音,消除运行音,延时不到,不发出运行音量
    __nop();
    if((IED.WarnMute==IED.u_WarnState.WarnState)
        &&(!IED.RunMute)&&(IED.cMovingBeepDelayCnt++>30))            //延迟不超过3S
    {
      //  IED.cMovingBeepDelayCnt = 30;
      //    send_data(0x0e); 
    } 
    //防止运行中相序发生变化 
    if(gcPrePhaseCtrl!=gcPhaseCtrl)
    {
        StopMotor();  //相序不对停机
    }
    gcPrePhaseCtrl    =    gcPhaseCtrl    ;
    //位置检测            
    GetCurretPos();
    ///////////////////////////////////////////
        //到位停止
    if(((IED.Direction==MOTOR_UP)&&(pos_top_bit==0))        ////运行方向向上,检测到上到位 停止
        ||((IED.Direction==MOTOR_DOWN)&&(pos_bot_bit==0)))    //运行方向向下,检测到下到位 停止
    {               
        #ifdef _DEBUG        
        printf("电机到位\n上到位=%d,下到位=%d,继电器释放!\n",(int)pos_top_bit,(int)pos_bot_bit);
        #endif
        StopMotor(); 
        _nop_(); 
        //if((IED.Direction==MOTOR_DOWN)&&(pos_bot_bit==0))
        //    IED.TimeTopToBot  =  IED.TimerS;       
        _nop_();
    } 
    //
    if(IED.cMoving)
    {        
        //超时停止
        long DestTime = 0;
        //////////////////////////////////////
        if(IED.Direction==MOTOR_UP)              //向上,计时递减
        {
            IED.TimerS--;  
            DestTime =(0L-TIME_OVERROLL);            
            //        
            if(IED.TimerS<DestTime)                        //到顶
            {      
                StopMotor();    
                #ifdef _DEBUG
                printf("上过卷\n");
                #endif
                IED.cPos = POS_TOP ;
                IED.Err|=ERR_TOP_OVERROLL;
            }
        }
        else                                  //向下,计时递增
        {
            IED.TimerS++;     

            DestTime =(IED.TimeTopToBot+TIME_OVERROLL);              
            if(IED.TimerS>DestTime)        //到底
            {      
                StopMotor();    
                #ifdef _DEBUG
                printf("下过卷\n");
                #endif
                IED.cPos = POS_BOT ;
                IED.Err|=ERR_BOT_OVERROLL;
            }  
        }     
        #ifdef _DEBUG
        //printf("TimerS=%ld\n",IED.TimerS);
        #endif 
        __nop();
        /////////////////////////////////////
        if(IED.TimerS==IED.TimeOutTimer)       //超时时间到.停止
        {           
            if(IED.TimerS==IED.TimeTopToMid) 
            { 
                #ifdef _DEBUG
                    printf("中停1 时间到!\n");
                #endif 
                IED.cPos = POS_MIDDLE ;
                
                StopMotor();
            } 
            else if(IED.TimerS==IED.TimeTopToMid2) 
            { 
                #ifdef _DEBUG
                    printf("中停2 时间到!\n");
                #endif 
                IED.cPos = POS_MIDDLE2 ;
                
                StopMotor();
            }
        } 
    } 
}

 enum  DebugMode{ACMODE=0,DCMODE};
 enum  DebugMode gcDebugMode = ACMODE;
  
void ResumeCheckSensor(void* p)
{  
    ActiveTask(TASK_SENSOR,500UL);
}
 
int8_t TimerStartInitInvertTimer = -1;     
int8_t TimerResumeCheckSensor = -1;
extern signed char bMiddleWaitTimerID;
void Cont_Break_Ctrl(void)
{ 
    if(CONT_BRK_Select==0)
    {
        #ifdef _DEBUG
        //printf("报警可撤销模式\n");
        #endif                           
        if((CtrlHalfDown==1)&&(CtrlAllDown==1)&&(IED.b_SmokeSensor==0)&&(IED.b_TempSensor==0))
        {                           
            StopWarning();
            IED.IED_State       = StateManualCtrl;  //退出报警状态
            IED.IED_DebugState = WarnMoveBot;   //撤销报警
					 StopTimer(&bMiddleWaitTimerID);
            #ifdef _DEBUG
            printf("报警撤销停止电机! \n");
            #endif
            if(IED.MotorMoveDelayTimerID!=0)            //如果还有延时起动电机任务未启动，撤销
                StopTimer(&IED.MotorMoveDelayTimerID);
            StopMotor();                                                        
        }    
    } 
    else
    {
        #ifdef _DEBUG
        //printf("报警不可撤销模式\n");
        #endif
        if((gcKey&KEY_STOPWARN)==KEY_STOPWARN)        //手动按键撤销
        {
            #ifdef _DEBUG
            printf("报警撤销\n");
            #endif
            
            IED.IED_State       = StateManualCtrl;  //退出报警状态
            IED.IED_DebugState = WarnMoveBot;       ////撤销报警
             StopTimer(&bMiddleWaitTimerID);
            StopWarning();    
            #ifdef _DEBUG
            printf("报警撤销停止电机! \n");
            #endif                
            StopMotor();                                                          
        }
    } 
    if((gcKey&KEY_STOPWARN)==KEY_STOPWARN)
    {
        StopWarning();
        /////////////////////////////////////////////////////////////////////
        StopInvert();      //停止升压
        SuspendTask(TASK_SENSOR);
        
        if(TimerStartInitInvertTimer != -1)    
        {
            StopTimer(&TimerStartInitInvertTimer);  
        }
         
        StartTimer2(&TimerStartInitInvertTimer,TIMER_MODE_ONCEROUTINE,1400,InitInvert,0);    //14S后重新启动逆变  
        if(TimerResumeCheckSensor != -1)    
        {
            StopTimer(&TimerResumeCheckSensor);  
        }
        StartTimer2(&TimerResumeCheckSensor,TIMER_MODE_ONCEROUTINE,1500,ResumeCheckSensor,0); 
        /////////////////////////////////////////////////////////////////////    
    }
}

//开始报警
void StartWarning(void)
{    
    IED.b_warning = 1;                 
    ActiveTask(TASK_WARN,100);   
    #ifdef _DEBUG
    printf("Start Warning !\n");
    #endif                             
}

//停止报警
void StopWarning(void)
{                
//    SetWarningFreq(0);
    IED.b_warning = 0;
    IED.b_Alldown = 0;
    IED.b_Halfdown= 0 ;
    IED.b_SmokeSensor = 0;
    IED.b_TempSensor  = 0 ;
     SuspendTask(TASK_WARN);    
    #ifdef _DEBUG
    printf("Stop Warning !\n");      
    #endif
                   
} 


#define FREQ_MIN 1900L 
#define FREQ_MAX 2400L    
//定时器产生的脉冲
//////////////报警控制////////////////////////////
//unsigned int gcFreqCurr = 0;     //当前频率
//char gcFreqdir = 1;                 //频率方向
//报警任务
void TaskWarn(void)
{                             
	 if((send_data_flag==0)&&(IED.WarnMute==0))
		           {
								 if((IED.Err&(BAT_LOW_ERR|BAT_SHORT|BAT_OVER_ERROR))==0)
						   {
			         send_data_flag=1;
				       send_end_timer=200;
				       send_data(0x0c);//
		           }
							 
						 else if(IED.cMoving==0)
							 {
			         send_data_flag=1;
				       send_end_timer=200;
				       send_data(0x0c);//
		           }
						 
						 }
}     

 



enum DEBUG_STATE{DEBUG_INIT=0,DEBUG_MOTOR_TOP,
                DEBUG_WAIT_KEYDOWN1,DEBUG_WAIT_KEYSTOP,
                DEBUG_WAIT_KEYDOWN2,DEBUG_MOTOR_BOT,
                DEBUG_MOTOR_TOP2,DEBUG_WAIT_KEYDOWN3,
                DEBUG_WAIT_KEYSTOP2,DEBUG_WAIT_SAVE ,
                
};     
 void DebugCtrl_DC(void)
 {
    if((gcKey&KEY_STOPWARN)==KEY_STOPWARN)       //复位调试状态
    {
        IED.IED_State = StateManualCtrl;
        StopMotor();
    }     
    switch(IED.IED_DebugState)
    {
        case  DEBUG_INIT:  
            MovMotor(POS_TOP);
            IED.IED_DebugState = DEBUG_MOTOR_TOP; 
            break;   
    }        
 }
#define MAXTIMETOPTOMID   0x4000
#define MAXTIMETOPTOBOT   0x7FFF
#define MAXWAITTIME       60000L
#define MAXSLIDEWAITTIME  0x4000
 unsigned short debug_time;
 extern unsigned int write_eeprom_flag;
 __attribute__((section("RAMCODE")))
 void DebugCtrl(void)
{
    if((gcKey&KEY_STOPWARN)==KEY_STOPWARN)       //复位调试状态
    {
        IED.IED_State = StateManualCtrl;
        StopMotor();
    }     
   // if(IED.IED_DebugState!=DEBUG_WAIT_KEYDOWN2)   
    {
      //  Beep(1,500);
    }                              
    switch(IED.IED_DebugState)
    {
        case  DEBUG_INIT: 
            
        
          //  MovMotor(POS_TOP);
            IED.IED_DebugState = DEBUG_MOTOR_TOP;
            
            break;     
        case DEBUG_MOTOR_TOP:
            //GetCurretPos();         
            if(IED.cPos==POS_TOP)       //运行到顶
            { 
                IED.IED_DebugState = DEBUG_WAIT_KEYDOWN1; 
            }
            break;     
        case DEBUG_WAIT_KEYDOWN1: 
             	debug_time++;	
             if(debug_time>3000)
						 {
							  IED.IED_State =     StateManualCtrl;
							 debug_time=0;
							  break;   
						 }							 
            if(gcKey==KEY_DOWN)
						{
							debug_time=0;
            if(gcDebugMode==ACMODE)
            {    
                TimeOut.EnStop2 = 0;            //学习后消除大半降落
                IED.MidWaitTimerID = -1;
                IED.TimeTopToMid = MAXTIMETOPTOMID    ;  
                //调试时把超时 时间调大，防止提前超时停止电机运行
                IED.TimeTopToBot = MAXTIMETOPTOBOT; 
                IED.MidTimeWait = MAXWAITTIME;    
                IED.SlideTimeOutTimer = 0;
                IED.SlideTimeTopToMid = 3;
                
                MovMotor(POS_BOT);
                IED.IED_DebugState = DEBUG_WAIT_KEYSTOP; 
            } 
            //if (((gcKey&KEY_MUTE)==KEY_MUTE)         //MUTE 逆变下降
            //    ||((gcKey&KEY_STOP2)==KEY_STOP2) )
            else
            {   
                EEReadStruct(&TimeOut,sizeof(TimeOut),0);                
                IED.SlideTimeOutTimer = 0;
                IED.SlideTimeTopToMid = MAXSLIDEWAITTIME; 
                SuspendTask(TASK_POWER_CHECK)     ;    
                IED.Err |= MAIN_POWER_ERROR;  
                
                




							MovMotor(POS_BOT);
                IED.IED_DebugState = DEBUG_WAIT_KEYSTOP2; 
            }
					}
            break;     
        case DEBUG_WAIT_KEYSTOP:    
            if((gcKey&KEY_STOP)==KEY_STOP)
            {
                StopMotor();
                //得到运行到中位的时间
                TimeOut.TimeTopToMid = IED.TimerS;    
                //开始启动中位值暂停计时
                TimeOut.MidTimeWait = 3000; 
                IED.IED_DebugState = DEBUG_WAIT_KEYDOWN2;     
                          
            }
            break;
        case DEBUG_WAIT_KEYDOWN2:  
            if((gcKey&KEY_STOP)==KEY_UP)
            {
               // StopBeep();
               //  send_data(0x0e);   
                if(TimeOut.MidTimeWait<9000L) 
                    TimeOut.MidTimeWait+=1000;//10S 
            }
            if((gcKey&KEY_DOWN)==KEY_DOWN)
            { 
                MovMotor(POS_BOT);   
                IED.IED_DebugState = DEBUG_WAIT_SAVE;
                //IED.IED_DebugState = DEBUG_MOTOR_BOT;
                //转结束保存
            }
            
            break;    
#if 1            
        case DEBUG_MOTOR_BOT:
                                                      
            if(((gcKey&KEY_UP)==KEY_UP)&&(IED.cPos==POS_BOT))        //向上运行    //运行到底部
            {                  
                //TimeOut.TimeTopToBot = IED.TimerS+TIME_OVERROLL;    ;          //下过卷 60X50 = 2S
                TimeOut.TimeTopToBot = IED.TimerS;          //下过卷 60X50 = 2S
                #ifdef _DEBUG 
                printf("TimeOut.TimeTopToBot=%ld\n",TimeOut.TimeTopToBot);
                #endif
                //得到 到底的超时时间           
                IED.IED_DebugState = DEBUG_MOTOR_TOP2;
                MovMotor(POS_TOP);             
                //开始启动向上运行延时
                StartTimer2(&IED.MidWaitTimerID,TIMER_MODE_TIMER,100000,0,0);         
                #ifdef _DEBUG
                printf("IED.MidWaitTimerID=%d\n",(int)IED.MidWaitTimerID);
                #endif
            } 
            break;
        case DEBUG_MOTOR_TOP2:
            if(IED.cPos==POS_TOP)       //运行到顶
            { 
                #ifdef _DEBUG
                printf("TimerData[IED.MidWaitTimerID].lPeriod=%ld\n",TimerData[IED.MidWaitTimerID].lPeriod);
                printf("TimerData[IED.MidWaitTimerID].lLastTime=%ld\n",TimerData[IED.MidWaitTimerID].lLastTime);
                #endif
                //TimeOut.TimeBotToTop = TimerData[IED.MidWaitTimerID].lPeriod-TimerData[IED.MidWaitTimerID].lLastTime ;
                //TimeOut.TimeBotToTop/=5;  //定时是10mS为周期,电机 检测为50ms为一个周期
                //TimeOut.TimeBotToTop+=TIME_OVERROLL;    ;          //上过卷 60X50 = 3S 
                StopTimer(&IED.MidWaitTimerID);
            //    StopBeep();
                  if(send_data_flag==0)
		           {
			         send_data_flag=1;
				       send_end_timer=8;
				      send_data(0x0e); //
		           }
                IED.IED_DebugState = DEBUG_WAIT_KEYDOWN3; 
                SuspendTask(TASK_POWER_CHECK)     ;    
                IED.Err |= MAIN_POWER_ERROR;  
            }            
            break;
        case DEBUG_WAIT_KEYDOWN3:
            if((gcKey&KEY_DOWN)==KEY_DOWN)
            {       
                MovMotor(POS_BOT);
                IED.IED_DebugState = DEBUG_WAIT_KEYSTOP2; 
                //初始化下滑定时器  
            }
            break;
        case DEBUG_WAIT_KEYSTOP2:
            if((gcKey&KEY_STOP)==KEY_STOP)
            {
                IED.IED_State =     StateManualCtrl; 
                StopMotor();
                //
                ActiveTask(TASK_POWER_CHECK,1000L)     ;    
                IED.TimerS = IED.TimeTopToBot;
                //保存点击下滑时间
						//	 __disable_irq();
                IED.TimeTopToMid = TimeOut.TimeTopToMid ;
                IED.TimeTopToBot = TimeOut.TimeTopToBot ;  
                IED.MidTimeWait  = TimeOut.MidTimeWait  ;      
                TimeOut.SlideTimeOutTimer = IED.SlideTimeOutTimer ;
                IED.SlideTimeTopToMid = TimeOut.SlideTimeOutTimer ; 
                HAL_Delay(1000);							
                #ifdef _DEBUG
                 printf("顶部到中停时间= %ld \n暂停时间=%ld \n顶部到底 时间=%ld \n",
                    TimeOut.TimeTopToMid,TimeOut.MidTimeWait,TimeOut.TimeTopToBot);
                printf("顶部滑行到中停时间= %ld \n",TimeOut.TimeTopToMid);
                printf("底到顶时间= %ld \n",TimeOut.TimeTopToMid);
                printf("滑行时间%ld \n",TimeOut.SlideTimeOutTimer);
                #endif    
							write_eeprom_flag=1;
              //  EEWriteStruct(&TimeOut,sizeof(TimeOut),0);  
						//		HAL_Delay(1000);
              //   enable();							
            }
            break;        
#endif
        case DEBUG_WAIT_SAVE:
            if( IED.cMoving == 0)
            {
		           
			         send_data_flag=1;
				       send_end_timer=200;
				       send_data(0x0B);//中停设置完毕
		           
                IED.IED_State =     StateManualCtrl;    //手动控制模式
                StopMotor();   
                //保存点击下滑时间
						//	 __disable_irq();
                TimeOut.TimeTopToBot = IED.TimerS ;
                IED.TimeTopToMid = TimeOut.TimeTopToMid ;
                IED.TimeTopToBot = TimeOut.TimeTopToBot ;  
                IED.MidTimeWait  = TimeOut.MidTimeWait  ;      
                //默认下滑2次到中停 
                TimeOut.SlideTimeOutTimer = 2;
                IED.SlideTimeTopToMid = TimeOut.SlideTimeOutTimer;  						
                #ifdef _DEBUG
                 printf("顶部到中停时间= %ld \n暂停时间=%ld \n顶部到底 时间=%ld \n",
                    TimeOut.TimeTopToMid,TimeOut.MidTimeWait,TimeOut.TimeTopToBot);
                printf("顶部滑行到中停时间= %ld \n",TimeOut.TimeTopToMid);
                printf("底到顶时间= %ld \n",TimeOut.TimeTopToMid);
                printf("滑行时间%ld \n",TimeOut.SlideTimeOutTimer);
                #endif    
								write_eeprom_flag=1;
             //   EEWriteStruct(&TimeOut,sizeof(TimeOut),0);  
              //  HAL_Delay(1000);								
           //      enable();								
            }
            break;
        default:
            break;
    }
}

 void run_vol_set(void)
 {
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
 }
 void charge_vol_set(void)
 {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 
 }
 void jiance_vol_set(void)
 {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); 
 }
void setDIR(void)
{  
    
    #ifdef _MCU_302  
    uint32_t DIR = 0;    
    #endif
    gcPrePhaseCtrl=gcPhaseCtrl;
    if((gcDirCtrl^gcPhaseCtrl^1)==1)                                                  //运行方向控制、、  
    {
        
        #ifdef _MCU_301 
            if(IED.Direction==MOTOR_UP)
						{
							MOTOR__DIR__SET;
						}
            else{MOTOR__DIR__CLR;}							
 //           MOTOR__DIR =     IED.Direction==MOTOR_UP?1:0;     
        #elif _MCU_302     
            DIR   = IED.Direction==MOTOR_UP;
        #endif
    }
    else 
    {
        
        #ifdef _MCU_301 
            if(IED.Direction==MOTOR_UP)
						{
							MOTOR__DIR__CLR;
						}
            else{MOTOR__DIR__SET;}	
 //           MOTOR__DIR =     IED.Direction==MOTOR_UP?0:1;   
        #elif _MCU_302          
            DIR   = IED.Direction==MOTOR_DOWN;
        #endif
    } 
    
    #ifdef _MCU_302    
    if(DIR)
        MOTOR__UP__SET; 
    else
        MOTOR__DOWN__SET; 
    #endif    
    
}

void MotorDelay(void *p)
{
    IED.BrakeDelayTimerID  = -1; 
    
    #ifdef _MCU_301   
    MOTOR__RUN = 1;
	  MOTOR__RUN__SET;
    #elif _MCU_302
    #endif 
    ActiveTask(TASK_STOPMOTOR,50);    ////每50ms检测一次电机是否到位
    #ifdef _DEBUG                                                 
    //printf("启动电机!\n");
    #endif    
}


//volatile unsigned int INT1Cnt = 0;
 int HiCnt     = 0;
 int LowCnt    = 0;   
//int abs(int x) {return x>0?x:-x;}
unsigned char PreEx1,PreEx2;
int PreMotorLine =0;
//int abs(int x) {return x>0?x:-x;}




char preUserMode = 3;
void checkUserMode(void)
{ 
    //用户模式和检测模式切换
    if(preUserMode!=KEY_MODE)
    {
        if(KEY_MODE==USER_MODE)
            EnableBrakPower(TRUE);
        else
            EnableBrakPower(FALSE);
    //   send_data(0x0e); 
    } 
    preUserMode = KEY_MODE;
}

uint32_t bCtrlAllDown_IN_cnt = 0; 
uint32_t bCtrlHalfDown_IN_cnt = 0;
void CheckCtrlSignal(void)
{  
    if(!CtrlAllDown_IN)
    {
        if(bCtrlAllDown_IN_cnt++>100)
        {
            CtrlAllDown = 0;
        }
    }
    else
    {
        bCtrlAllDown_IN_cnt = 0;
        CtrlAllDown = 1;
    }
    ////////////////////////////
    if(!CtrlHalfDown_IN)
    {
        if(bCtrlHalfDown_IN_cnt++>100)
        {
            CtrlHalfDown = 0;
        }
    }
    else
    {
        bCtrlHalfDown_IN_cnt = 0;
        CtrlHalfDown = 1;
    }
}

void TaskStateCheck(void)
{ 
    checkUserMode();
    gcKey=GetKey();
    CheckCtrlSignal();
    if(CONT_BRK_Select==1)
    {
        if(((CtrlAllDown==0)||(IED.b_TempSensor==1))&&(IED.b_Alldown==0))          //锁存全降信号
        { 
            #ifdef _DEBUG            
            printf("锁存全降信号\n") ;
            #endif
            IED.b_Alldown = 1;
        }
        if(((CtrlHalfDown==0)||(IED.b_SmokeSensor==1))&&(IED.b_Halfdown==0))    
        {
            #ifdef _DEBUG            
            printf("锁存半降信号\n") ;
            #endif
            IED.b_Halfdown = 1;    
        }
    }    
    //2015-1-5  增加
     
    //将烟感的和联动信号的全降优先级做调整
    //来了烟感,取消联动信号信号的全降
    
//    if(IED.b_SmokeSensor==1)
//    {   
//        IED.b_Alldown = 0;
//        CtrlAllDown     = 1; 
//    }
    //if(((gcKey&KEY_FIRE)!=0)&&(IED.b_warning==0))        //用读取键值的方法可以抗干扰  有烟温度信号为报警
    if(
        ((CtrlHalfDown==0)||(CtrlAllDown==0)||(IED.b_SmokeSensor==1)||(IED.b_TempSensor==1))
        &&(IED.b_warning==0))
     {

        IED.IED_State = StateWarning; 			 
        IED.IED_DebugState = WarnInit;
			  
				
			  if((send_data_flag==0)&&(IED.WarnMute==0))
		           {
								if((IED.Err&(BAT_LOW_ERR|BAT_SHORT|BAT_OVER_ERROR))==0)
								 {
			         send_data_flag=1;
				       send_end_timer=200;
				       send_data(0x0c);//
		           }
						 }
        #ifdef _DEBUG            
        printf("_报警模式 \n") ;
        #endif
						 
     }
     else
     {

     }
    //WarningSource{S_HalfDown=1,S_AllDown=2,S_bHalfDown=4,S_bAllDown=8,S_SensorSmoke=0x10,S_SensorTemp=0x20};
    //静音控制

    IED.u_WarnState.s_WarnState.HalfDown        =    (CtrlHalfDown==0?1:0L);
    IED.u_WarnState.s_WarnState.AllDown            =    (CtrlAllDown==0?1:0L);
    IED.u_WarnState.s_WarnState.bHalfDown        =    (IED.b_Halfdown==1?1:0L);
    IED.u_WarnState.s_WarnState.bAllDown        =    (IED.b_Alldown==1?1:0L);
    IED.u_WarnState.s_WarnState.b_Sensor_Smoke    =    (IED.b_SmokeSensor==1?1:0L);
    IED.u_WarnState.s_WarnState.b_Sensor_Temp    =    (IED.b_TempSensor==1?1:0L);
    IED.WarnMute &= IED.u_WarnState.WarnState;    
    //
    if(IED.pre_u_WarnState!=IED.u_WarnState.WarnState)
    { 
        IED.IED_DebugState = WarnInit;
        IED.pre_u_WarnState=IED.u_WarnState.WarnState;
    }
    ///////////////////////////////////////////////////////////////////////                     
     switch(IED.IED_State)
     {                                              
         case StateWarning:      //报警状态    
            HostWarnCtrl();    
             break;
        case StateDebug:          //进入调试状态     
             if(send_data_flag==0)
		           {
								 
			          send_data_flag=1;
				        send_end_timer=300;
				        send_data(0x0A);//进入中停位置设置
		           }					
              DebugCtrl();    //调试模式,计算门的时间
							
            break; 
        case StateManualCtrl:
            ManualCtrl();
            break;
        default:
            IED.IED_State = StateManualCtrl;
            break;      
     }  
}

 

#define B_LOCK IED.B_LOCK
int8_t DelayLock= -1; 

int GetLock(void)
{
    return B_LOCK ;
}  

void SetLock2(void *bLock)
{
    int  Lock = *(int*)  bLock;
    SetLock(Lock);
}

void WriteALLLED(int bLock)
{ 
 //   CHILD[0].LOCKLED = CHILD[1].LOCKLED =  CHILD[2]. LOCKLED = bLock;     
 //   WriteLEDInfo (1); 
}

//加锁 如果为加锁,全部等亮
//解锁,只解定时关闭
//实际解锁为键盘程序里
void SetLock(int bLock)
{
    static int willLock = 1; 
     B_LOCK = bLock;     
    CHILD[0].LOCKLED = CHILD[1].LOCKLED =  CHILD[2]. LOCKLED = bLock;
    if( bLock)
    { 
     //   CHILD[0].LOCKLED = CHILD[1].LOCKLED =  CHILD[2]. LOCKLED = bLock;
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
			 StopTimer( &DelayLock);
    }
        
   else
    {  
			
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
        if(DelayLock!=-1)
        {  
            StopTimer( &DelayLock);
        }  
     //   DelayLock = StartTimer(TIMER_MODE_ONCEROUTINE,100L*30*60,SetLock2,&willLock);  //     
     //   StartTimer2(&DelayLock,TIMER_MODE_ONCEROUTINE,100L*30*60,SetLock2,&willLock);     
     }

   		
}
 

char LockLedCnt[3];
/*
    gcKeyStack[gcKeySavePtr]=KEY_LOCK;   
                        LockLedCnt[i]++ ;
                        if(LockLedCnt[i]==3)      //3次加锁或者解锁
                        {
                            CHILD[i].LOCKLED = !CHILD[i]. LOCKLED;
                            
                            if((CHILD[2].LOCKLED != CHILD[1].LOCKLED)   //不一致,总开关失效
                                ||(CHILD[2].LOCKLED ==0)
                                ||(CHILD[1].LOCKLED ==0)
                            )                 //都为0 总开关失效
                            { 
                                SetLock(0);
                            } 
                        }

                    if(Key!=KEY_MUTE)
                    {
                        LockLedCnt[i] =0;
                    }
*/
//手动控制
void ManualCtrl(void)
{    
    if(gcKey==KEY_MUTE)
    {
        MuteError();
        MuteRun();
    }
    
    if((gcKey&KEY_SELFTEST)==KEY_SELFTEST)
    {  
//        int i= 0;
//        StopInvert();       
        NVIC_SystemReset();
        //SelfTest();
    }
 //   if(B_LOCK==0)          //没有锁定,可以手动操控
 //   {
                    //无报警信号,有调试按键 按下为调试状态
        if(((gcKey&KEY_DEBUG)==KEY_DEBUG)&&(IED.IED_State!=StateWarning)&&(IED.cPos == POS_TOP))
        {
            IED.IED_State = StateDebug;    
            IED.IED_DebugState = DEBUG_INIT;
            gcDebugMode = ACMODE;
            #ifdef _DEBUG
            //printf("_调试模式 \n") ;
            #endif
            return;
        }
        if(((gcKey&KEY_STOP2)==KEY_STOP2)&&(IED.IED_State!=StateWarning)&&(IED.cPos == POS_TOP))
        {
            IED.IED_State = StateDebug;    
            IED.IED_DebugState = DEBUG_INIT;
            gcDebugMode = DCMODE;
            #ifdef _DEBUG
            //printf("_调试模式 \n") ;
            #endif
            return;
        }
        /*
        if(gcKey==KEY_STOP2)  //长按3S 就保存 中停2
        {
            TimeOut.TimeTopToMid2 = IED.TimerS ;  
            IED.TimeTopToMid2 = TimeOut.TimeTopToMid2 ;
            TimeOut.EnStop2 = 1;
            EEWriteStruct(&TimeOut,sizeof(TimeOut),0);  
            return ;          
        }
        */
        if((IED.Err&(MAIN_POWER_ERROR|ErrPhase|ERR_MOTOR_LINE))!=0)  //主电错误的情况
        { 
            //DCManualCtrl();
					 ACManualCtrl();
        }
        else
        {
            #ifdef _DEBUG
            //printf("交流手动控制\n") ;
            #endif
            ACManualCtrl();
        } 
  //  }
//    else
//    {
//        #ifdef _DEBUG
//        printf("_系统已被锁定 \n") ;
//        #endif
//    }

}



void MotorMid(void *p)
{
    #ifdef _DEBUG
    printf("MOTOR_MIDDLE!\n");
    #endif        
    MovMotor(POS_MIDDLE);//启动半降信号
    IED.MotorMoveDelayTimerID = -1;
}

void MidWait(void* p)
{
    IED.MotorMoveDelayTimerID = -1;
    IED.IED_DebugState = WarnMoveMid;  
}

//全部降落
void MotorDown(void *p)
{
    #ifdef _DEBUG    
    printf("MOTOR_DOWN!\n");        
    #endif             
    MovMotor(POS_BOT);//启动全降信号
    IED.MotorMoveDelayTimerID = -1;
}

void MotorDown_Middle(void *p)
{
    #ifdef _DEBUG    
    printf("MOTOR_DOWN!\n");        
    #endif             
    MovMotor(POS_MIDDLE2);//启动大半降落2信号
    IED.MotorMoveDelayTimerID = -1;
}

void MotorUp(void *p)
{
    #ifdef _DEBUG
    printf("MOTOR_UP!\n");
    #endif        
    MovMotor(POS_TOP);//启动全升信号
    IED.MotorMoveDelayTimerID = -1;
}

#define UNMASKERR      0xFFFFFFFF
void MuteError2(void *p)
{
    MuteError();
}
void MuteError(void)
{
    IED.ErrMute = IED.Err&UNMASKERR ;
    __nop() ;
}


void TaskCheckHeatErr(void)
{    //电机过热保护报错
    if((pos_bot_bit==0)&&(pos_top_bit==0))//过热保护
    {
        IED.Err |= ErrOverHeat;
    }
    else
    {
        IED.Err &= (~ErrOverHeat); 
    }    
}        


char preLEDMainPower;
char preLEDBatPower;
char preLEDWarn;
char preLEDRUN;
  

#define FEEDSMOKE           Feed_Smoke_Relay
#define FEEDLOCK            FEED_LOCK_LED
#define FEEDTEMP            Feed_Temp_Relay

    
uint32_t FEED_ERR_LED    ;
uint32_t Feed_Err_Cnt = 0;

int32_t glErr = 0;
int Pre_err;
void FeedSound(void)
{

		if((IED.Err!=0)&&(send_data_flag==0)&&(IED.ErrMute==0))
		{		
//		if((IED.Err&(MAIN_POWER_ERROR|ErrPhase))&&(MAIN_POWER_SOUND==0)&&(send_data_flag==0))   
//		{
//			 MAIN_POWER_SOUND=1;
//			 send_data(0x02);//主电故障
//			 send_data_flag=1;
//			 send_end_timer=20;
//				
//		}
		if((IED.Err&(BAT_LOW_ERR|BAT_SHORT|BAT_OVER_ERROR))&&(BAT_SOUND==0)&&(send_data_flag==0)) 
		{			
			 BAT_SOUND=1;
			 send_data(0x03);//备电故障
			 send_data_flag=1;
			 send_end_timer=160;
		}
		if((IED.Err&ERR_MOTOR_LINE)&& (MOTOR_LINE_SOUND==0)&&(send_data_flag==0))
		{
			MOTOR_LINE_SOUND=1;
			 send_data(0x04);//电机故障
			 send_data_flag=1;
			 send_end_timer=160;
		}
		if((IED.Err&(COIL_SHORT_ERR|COIL_BREAK_ERR))&&(COIL_SOUND==0)&&(send_data_flag==0)) 
		{
			 COIL_SOUND=1;
			 send_data(0x05);//刹车故障
			 send_data_flag=1;
			 send_end_timer=180;
		}
		if((IED.Err&(ErrOverHeat|ERR_BOT_OVERROLL|ERR_TOP_OVERROLL))&& (TOP_OVERROLL_SOUND==0)&&(send_data_flag==0))     
		{
			 TOP_OVERROLL_SOUND=1;
			send_data(0x06);//限位故障
			 send_data_flag=1;
			 send_end_timer=160;
		}
		if((IED.Err&(ERR_SENSOR_SMOKE|ERR_SENSOR_TEMP))&&(SMOKE_TEMP_SOUND==0)&&(send_data_flag==0))                    
		{			
			SMOKE_TEMP_SOUND=1;
			send_data(0x07);//烟,温感故障
			send_data_flag=1;
			send_end_timer=150;
		}
	
   }
}
unsigned char sound_feed_on=1;
unsigned char up_sound,down_sound;
void FeedLED(void)
{
   // IED.Err = glErr;
	if(IED.RunMute==0)
	{
			 if(!(IED.Err&(BAT_LOW_ERR|BAT_SHORT|BAT_OVER_ERROR)))
		 {			 
		    if((IED.cMoving==1)&&(MOTOR_UP==IED.Direction))
	      {
		     if(send_data_flag==0)
		      {
			      if(up_sound==1)
			        {
					     send_data_flag=1;
				       send_end_timer=20;
				       send_data(0x0e);//
				      }
		       else
				       {
					       up_sound=1;
			           send_data_flag=1;
				         send_end_timer=100;
				         send_data(0x08);//请注意，电机正在上行。
				        }
		 }
	 }
	 if((IED.cMoving==1)&&(MOTOR_DOWN==IED.Direction))
	 {
	
		if(send_data_flag==0)
		 {
			 if(down_sound==1)
			   {
					 send_data_flag=1;
				send_end_timer=20;
				send_data(0x0e);//
					 
				 }
		   else
				 {
					 down_sound=1;
			    send_data_flag=1;
				  send_end_timer=100;
				  send_data(0x09);//请注意，电机正在下行。
				 }
		 
		 }
	 }

  }
 }
	  if(IED.cMoving)
		{
			FEED_RUN__CLR;
		}
		else
     {
			 down_sound=0;
			 up_sound=0;
		 FEED_RUN__SET;
		 }
      if(IED.Err==0)
    {  
			if((power_on==1)||(Pre_err!=0))
			{
				power_on=0;
				send_data_flag=1;
				send_end_timer=8;
				send_data(0x01);//进入正常工作状态
			}
        LED_MAINPOWER__CLR;   
        LED_BAT__CLR ;   
        LED_ERR1__SET;LED_ERR0__SET;
    }
    else 
    {  
        if(IED.Err&(MAIN_POWER_ERROR|ErrPhase))                             FEED_ERR_LED = 0x20;       //主电缺相      	
        else if(IED.Err&ERR_MOTOR_LINE)                                     FEED_ERR_LED = 0x21;       //电机         
        else if(IED.Err&(COIL_SHORT_ERR|COIL_BREAK_ERR) )                   FEED_ERR_LED = 0x22;       //刹车    
        else if(IED.Err&(ErrOverHeat|ERR_BOT_OVERROLL|ERR_TOP_OVERROLL))    FEED_ERR_LED = 0x23;       //行程开关
        
        else if(IED.Err&(BAT_LOW_ERR|BAT_SHORT|BAT_OVER_ERROR))             FEED_ERR_LED = 0x10;       //备电
        else if(IED.Err&ERR_SENSOR_SMOKE)                                   FEED_ERR_LED = 0x12;       //烟感            
        else if(IED.Err&ERR_SENSOR_TEMP)                                    FEED_ERR_LED = 0x11;       //温感               
      //else if(IED.Err&ERR_TOP_OVERROLL)                                   FEED_ERR_LED = 0x13;       //越限     
        ///////////////////////////////////////////////
        if(!(IED.Err&(MAIN_POWER_ERROR|ErrPhase)))
        {
            LED_MAINPOWER__CLR;    
        }
         //////////////////////////////////////////////
        if(!(IED.Err&(BAT_LOW_ERR|BAT_SHORT|BAT_OVER_ERROR)))
        {
            LED_BAT__CLR; 
        }
        ///////////////////////////////////////////////
        
        if(FEED_ERR_LED&0x20)   
        {
					if(Feed_Err_Cnt==0)
					{
						LED_MAINPOWER__CLR;
					}
			    else{LED_MAINPOWER__SET;}
			
                            
        }
        if(FEED_ERR_LED&0x10)
        {   
           if(Feed_Err_Cnt==0)
					 {
						LED_BAT__CLR; 
					 }
           else	{LED_BAT__SET;}				 
        }  
        if(FEED_ERR_LED)
        {
            if((FEED_ERR_LED!=0x20)&& (FEED_ERR_LED!=0x10))
            {
                switch(FEED_ERR_LED&0x0F)
                {
                    case 0:  LED_ERR0__SET; LED_ERR1__SET;  break;  
                    case 1:  LED_ERR0__CLR; LED_ERR1__SET; break;  
                    case 2:  LED_ERR0__SET; LED_ERR1__CLR; break;  
                    case 3:  LED_ERR0__CLR; LED_ERR1__CLR; break;  
                    default:break;
                }
            }
            else    
            {
							if(Feed_Err_Cnt==0)
							{
								LED_ERR0__CLR;
								LED_ERR1__CLR;
							}
							else
							{
								LED_ERR0__SET; 
								LED_ERR1__SET; 
							}              
            }                
            
            
        }
    }
    Feed_Err_Cnt=(++Feed_Err_Cnt)%2;
		if(Pre_err!=IED.Err)
		{
			sound_feed_on=1;
		}
    Pre_err=IED.Err;
     
}

void FeedRelay(void)
{
    if(IED.b_warning==1)
    {  
//			if(IED.cPos==POS_BOT) 
//         {			
//          FeedWarningRELAY__SET;  
//				 }
//         else{FeedWarningRELAY__CLR; }	
         FeedWarningRELAY__SET; 				 
    			LED_FeedWarn__CLR;   
    }    
    else
    {
        FeedWarningRELAY__CLR;       LED_FeedWarn__SET; 
    }
        //输出开关继电器控制
    //if((IED.b_warning==1)||(KEY_MODE==USER_MODE)) 
    {         
        switch(IED.cPos)
        {         
            case POS_TOP:         
                FeedTopRelay__SET;
                break;
            case POS_MIDDLE:    
            case POS_MIDDLE2:                
                FeedMiddleRELAY__SET;
                break; 
            case POS_BOT:     
                FeedBottomRELAY__SET;   
                break;
            default:    
                FeedBottomRELAY__CLR;
                FeedMiddleRELAY__CLR;
                FeedTopRelay__CLR;      
                break;
        }
    }
    /*
    else
    {
        FeedBottomRELAY         =0;
        FeedMiddleRELAY         =0;
        FeedTopRelay            =0;    
    }   
    */
     
}

extern IWDG_HandleTypeDef hiwdg;
void Task_WDT(void)
{
   HAL_IWDG_Refresh(&hiwdg);
}



#define MINUTE_10 (10L*60L*100L)  
signed char TimDelayMute = -1;            
 

void Feed(void)
{    
    if(IED.Err==0)
    {
        FeedLED();
        FeedRelay(); 
    }
    IED.ErrMute&=IED.Err;  //故障已清除,消除故障消音位为下一次故障声音报警做准备
    if(IED.Err!=0)
    {
        IED.ErrDelayTime++;
        if(IED.ErrDelayTime>=MAXDELAYCNT)
        {
            IED.ErrDelayTime = MAXDELAYCNT; 
					if(sound_feed_on==1)
					{
							 if((IED.Err&(BAT_LOW_ERR|BAT_SHORT|BAT_OVER_ERROR))==0)
						   {
			         FeedSound();
		           }
							 
						 else if(IED.cMoving==0)
							 {
			        FeedSound();
		           }
					  
						
					}
            FeedLED(); 
            FeedRelay(); 
            //FeedErrLED = 1;
            FeedErrorRELAY__SET;
            #ifdef _DEBUG
            //printf("IED.Err=0X%04X\n",IED.Err);
            #endif 
            //指定错误声音提示方案
            if((IED.Err^IED.ErrMute)
                &&(!(
                    (KEY_MODE==USER_MODE)                //用户模式下电池不报警
                    &&((IED.Err&(~(BAT_LOW_ERR|BAT_SHORT|BAT_OVER_ERROR)))==0)
                    )
                   )
            )
            {
//               Beep(1,300);
                
                if((KEY_MODE==USER_MODE)&&(TimDelayMute==-1) )
                { 
                    StartTimer2(&TimDelayMute,TIMER_MODE_ONCEROUTINE,MINUTE_10,MuteError2,0); 
                }
            }
        }                                         
    }      
    else
    {
        IED.ErrDelayTime = 0;
        FeedErrorRELAY__CLR; 
        StopTimer(&TimDelayMute);
    } 
    TaskCheckHeatErr();   
    
    
    
}


signed char SelftestTimer = -1;
int selftestcnt = 4;
void SelfTest(void)
{ 
    if(SelftestTimer==-1)
    { 
    //  Beep(3,500);
      selftestcnt = 6;
      StartTimer2(&SelftestTimer,TIMER_MODE_CYCROUTINE,50L,TaskSelfTestLED,0); 
    }
}

void LEDALLON(void)
{  
    LED_MAINPOWER__CLR; 
    LED_BAT__CLR;
    LED_FeedWarn__CLR;  
    FEED_RUN__CLR;
    LED_ERR0__CLR;
    LED_ERR1__CLR; 
}
void LEDALLOFF(void)
{
    LED_MAINPOWER__SET; 
    LED_BAT__SET;
    LED_FeedWarn__SET;  
    FEED_RUN__SET;
    LED_ERR0__SET;
    LED_ERR1__SET;              
}

void TaskSelfTestLED(void* param)
{  
    
    if (selftestcnt%2==0)
    { 
        LEDALLOFF(); 
        WriteALLLED(0);
    }
    else 
    {      
        WriteALLLED(1); 
        LEDALLON();   
    } 
    if(selftestcnt==0)
    {  
        StopTimer(&SelftestTimer);
        StopWarning();
//        Beep(1,100); 
        LEDALLOFF();
        ActiveTask(TASK_FEED,500L);  
        selftestcnt = 6;
    }
    /*
    if(selftestcnt==6)
    {
        SetWarningFreq(2000);
    }
    */
    selftestcnt--;
}


/*

void AddCOMMCycCommand(unsigned char cCommand, 
                    unsigned char length,
                    unsigned char tempAddr,
                    void *BaseAddr,
                    unsigned char IncSize,
                    unsigned char CmdParam)
*/


/*
UCHAR command;     //命令字 
UCHAR Length;      //多少个short
UCHAR cAddr;        //发送首地址
UCHAR *pAddr;      //接收的首地址
*/ 
//void GetChildState(void)
//{ 
//    AddCOMMCycCommand(READ_HOLDING_REGISTER, 
//        (sizeof(CHILD_IED_STRUCT)-sizeof(CHILD[0].LOCKLED))/sizeof(char)>>1,
//        0,CHILD,
//        sizeof(CHILD_IED_STRUCT),
//            0);  
//}


void WriteLEDInfo(int led)
{
    
//    signed long addr;
//    short * addr1,*addr2; 
//    
//    
//    
//    
//    addr1 = (short*)&(CHILD[0].LOCKLED);  
//    addr2 = (short*)&(CHILD[0]);
//    addr = (long)addr1 - (long)addr2; 
//    nop();
//    nop();
//    nop();
//    AddCOMMCommand(WRITE_MULTIPLE_REGISTER, sizeof(CHILD[0].LOCKLED),
//        addr>>1,&CHILD[0].LOCKLED,sizeof(CHILD_IED_STRUCT)/sizeof(char),0); 
//    nop();
//   int lock;
//	lock=led;
//	if(lock==1)
//	{
//		GPIO_SetBit(GPIOE, PIN0);
//	}
//	else
//		{
//			 GPIO_ClrBit (GPIOE, PIN0);
//	 }
} 
 
/*
void InitMotorInterrupt(void)
{
    EXTI_Init(GPIOB, PIN13,EXTI_FALL_EDGE);            //输入  
    IRQ_Connect(IRQ0_15_GPIOB13, IRQ1_IRQ, 2); 
    EXTI_Open(GPIOB, PIN13);    
}  

void EndMotorInterrupt(void)
{
    EXTI_Close(GPIOB, PIN13);
}

__attribute__((section("RAMCODE")))
void IRQ1_Handler(void)
{
    //EXTI_Clear(GPIOB, PIN13); 
    GPIOB->INTCLR = (0x01 << PIN13); 
    IED.lMotorErr  = 3;
}
 
void Task_Check_Motor (void)
{
    
    if(IED.lMotorErr>0)
    {
        IED.lMotorErr --;      
        IED.Err|=ERR_MOTOR_LINE;
    }
    else 
    {
        IED.Err&=(~ERR_MOTOR_LINE);
    }
    
}
  */
  
  


