#ifndef _CTRL_H_
#define _CTRL_H_
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
//////////////////////////////////////////////////////////////////
#ifdef _MCU_301
    #define MOTOR__RUN__SET              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)    
    #define MOTOR__RUN__CLR              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

    #define MOTOR__DIR__SET              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)    
    #define MOTOR__DIR__CLR              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)
#elif _MCU_302
    #define MOTOR__UP__SET               HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)      
    #define MOTOR__UP__CLR               HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)  

    #define MOTOR__DOWN__SET             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)    
    #define MOTOR__DOWN__CLR             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)
		
#endif

        ////////////////////////////////////////  
#define LED_MAINPOWER__SET                 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET)
#define LED_MAINPOWER__CLR                 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET)
#define LED_BAT__SET                       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET)    
#define LED_BAT__CLR                       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET)
#define LED_FeedWarn__SET                  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET)    
#define LED_FeedWarn__CLR                  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET)
#define FEED_RUN__SET                      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET)       
#define FEED_RUN__CLR                      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET)
#define LED_ERR0__SET                      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET)    
#define LED_ERR0__CLR                      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET)
#define LED_ERR1__SET                      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET)         	
#define LED_ERR1__CLR                      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET)
//#define BRAKECHECK__SET                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET) //刹 车检测控制继电 器  
//#define BRAKECHECK__CLR                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)
#define FeedTopRelay__SET               HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET)
#define FeedTopRelay__CLR               HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET)
#define FeedMiddleRELAY__SET            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET)            
#define FeedMiddleRELAY__CLR            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET)
#define FeedBottomRELAY__SET            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET)                      
#define FeedBottomRELAY__CLR            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET)
#define FeedErrorRELAY__SET             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET)                    
#define FeedErrorRELAY__CLR             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET)
#define FeedWarningRELAY__SET           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)      
#define FeedWarningRELAY__CLR           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET)
#define BAT_CTRL_RELAY__CLR            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)  
#define BAT_CTRL_RELAY__SET             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
////////////////////////////////////////  
//#define CHARGE_BAT__SET                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)
//#define CHARGE_BAT__CLR                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)
//#define SHUTDOWN__SET                   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)
//#define SHUTDOWN__CLR                   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)
#define INVERT__SET                     HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_SET)//PA0_O
#define INVERT__CLR                     HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_RESET)
//#define FLUSH_BAT                   PA8_O // 
//#define reserve1                    LED.sOut.RESERVE 
 
//#define Feed_Temp_Relay             PD5_O  //温感反馈 
//#define Feed_Smoke_Relay            PD5_O  //烟感反馈
///////////////////////////////////////////////////////////  
#define KEY_MODE                    0  
enum BOARDTYPE{BOARD1PHASE=0,BOARD3PHASE=0x02}; 
#define pos_bit1                  HAL_GPIO_ReadPin (GPIOF,GPIO_PIN_0) //PA4_I   //下到位PA5_I
#define pos_bit2                  HAL_GPIO_ReadPin (GPIOA,GPIO_PIN_3) // PA5_I //上到位 PA4_I 
//#define gcMotoType                  PC12_I //电机类型 

//#define gcMotoType                    1       //端子
#define gcMotoType                    0         //插件
#define pos_top_bit                 (gcMotoType?pos_bit1:(gcDirCtrl?pos_bit2:pos_bit1))
#define pos_bot_bit                 (gcMotoType?pos_bit2:(gcDirCtrl?pos_bit1:pos_bit2))
        ///////////////////////////////////////////////
//#define LOCK_CHECK                  HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10)// PB13_I //
//#define MOTORLINE                  HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)// PB13_I //门运行监测        
#define CtrlHalfDown_IN            HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15)// PD1_I    //半降       
#define CtrlAllDown_IN             HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14)//PA8_I    //全降  


#define CONT_BRK_Select            HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)// PD0_I//SPIIO.InPort.sIn.CONT_BRK_Select //点动,连续 
//#define PHASEA                     HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)//PA7_I
//#define PHASEB                     HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)//PA6_I

#define ProgSelect                 HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)// PC3_I//SPIIO.InPort.sIn.ProgSelect //程序选择 
#define gcDirCtrl                  HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_14) //PC2_I//SPIIO.InPort.sIn.gcDirCtrl  //方向控制

#define shi_dian_in                   HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) //PC2_I//SPIIO.InPort.sIn.gcDirCtrl  //方向控制
#define MAINPOWER__ON                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)//       PB11_O 
#define MAINPOWER__OFF                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)
#define CPU_POWER__ON                 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET)//       PB11_O 
#define CPU_POWER__OFF                 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET)
enum BrakeType{AC_BREAK=0,DC_BREAK};//交流刹车  //直流刹车
enum ErrorWarnStae{ACWarn=0,DCWarn};
 

typedef struct _CHILD_IED_STRUCT
{
    short nId;
    short Key;
    short KeyLOCKDummy;
    short LOCKLED;
}CHILD_IED_STRUCT;
extern CHILD_IED_STRUCT CHILD[3];
enum RUNMODE{GB_MODE = 0,USER_MODE};
#pragma anon_unions    //运行匿名的union变量
typedef  struct
{
    unsigned char nId;
    unsigned char IED_State;
    unsigned char IED_DebugState;
    unsigned char IED_PreDebugState;
    unsigned char B_dummy1;
    unsigned char B_LOCK;
    //报警声音状态
    unsigned char b_warning;   //正在报警
    unsigned char b_Alldown;
    unsigned char b_Halfdown;
    unsigned char b_SmokeSensor;
    unsigned char b_TempSensor;
    
    unsigned int  RunMute;
    unsigned int  WarnMute;
    union{
        struct
        {
            unsigned char  HalfDown:1;
            unsigned char  AllDown:1;
            unsigned char  bHalfDown:1;
            unsigned char  bAllDown:1;
            unsigned char  b_Sensor_Smoke:1;
            unsigned char  b_Sensor_Temp:1;
        }s_WarnState;
        unsigned char  WarnState;
    }u_WarnState;
    
    unsigned long  lMotorErr;
    
    unsigned char  pre_u_WarnState;
    unsigned short int  FreqCurr;
    signed      char Freqdir;
    union{
        int  Err;
        struct{
            int eErrPhase:1;
            int eErrOverHeat:1;
            int eMAIN_POWER_ERROR:1;
            int eBAT_LOW_ERR:1;
            int eBAT_OVER_ERROR:1;
            int eBAT_SHORT:1;
            int eCOIL_SHORT_ERR:1;
            int eCOIL_BREAK_ERR:1;
            int eERR_SENSOR_SMOKE:1;
            int eERR_SENSOR_TEMP:1;
            int eERR_TOP_OVERROLL:1;//过卷
            int eERR_BOT_OVERROLL:1;//过卷
            int eERR_MOTOR_LINE:1;//电机短线检测
        };
    };
    unsigned int  ErrMute;
    //电机
     
    unsigned char cPos; 
    unsigned char PreDir;    //上次的方向
    unsigned char Direction;
    unsigned char cMoving;
    unsigned int  cMovingBeepDelayCnt; //运行声音延迟
    unsigned char cDCCovert;
    //时间测量
    long  OverRollTime;
    long  TimerS;       //测量电机运行计数
    long  TimeOutTimer; //超时定时器
    long  SlideTimeOutTimer;      //滑行中停超时定时
    //----------
    //
    long SlideTimeTopToMid ;//顶部滑行到中部时间
    long TimeTopToMid;    //顶部到中部的时间
        long TimeTopToMid2;
    long TimeTopToBot;    //顶部到底部的时间    
    //long TimeBotToTop;    //底部到顶部的时间
    long MidTimeWait ; //中位停止等待时间  
    signed char MidWaitTimerID ;//中停定时器ID
    signed char MotorMoveDelayTimerID;  //电机延时启动定时器ID
    signed char BrakeDelayTimerID;  //刹车切换延时定时器ID
    //----------
    unsigned char Motor_Cmd;                  //将要执行的电机指令 ,执行后清除
    unsigned char PreMotor_Cmd;                //正在执行的电机指令,执 行完毕后清除

    unsigned char Bat_Low_Cnt;
    unsigned char Main_Power_OK_Cnt;  
    unsigned char preWarnState;           //过去报警状态(备电,主电?)
    unsigned char curWarnState;           //当前报警状态(备电,主电?)
    //
//    unsigned char RunMode; //运行模式
    unsigned char ErrDelayTime;
    
     
}
IED_DATABASE;
extern  IED_DATABASE IED;
typedef  struct
{
    long TimeTopToMid;    //顶部到中部的时间
    long TimeTopToBot;    //顶部到底部的时间
    long TimeBotToTop;    //底部到顶部的时间
    long MidTimeWait;
    long MidTimeWait1;
    long SlideTimeOutTimer    ;      
    long TimeTopToMid2;    //顶部到中部2的时间    
    long EnStop2;
}MyTimeOut ;
extern  MyTimeOut TimeOut ; 
////////////////////////////////////////////////////////////////////////
enum MotorDir {MOTOR_UP=0,MOTOR_DOWN};                          
enum CtrlState    {StateManualCtrl=0,StateWarning,StateDebug}; //
enum MOTORPOS {POS_TOP=0,POS_MIDDLE,POS_MIDDLE2,POS_BOT,POS_UNKNOWN,POS_NONE};
enum WarningState {WarnInit=0,WarnMidWait,WarnMoveMid,WarnMoveBotWAIT,WarnMoveBot} ;
enum WarningSource{S_HalfDown=1,S_AllDown=2,S_bHalfDown=4,S_bAllDown=8,S_SensorSmoke=0x10,S_SensorTemp=0x20};        
enum ErrorType{ErrPhase=0x1,ErrOverHeat=0x2,MAIN_POWER_ERROR=0x4,
                BAT_LOW_ERR=0x8,BAT_OVER_ERROR=0x10,BAT_SHORT=0x20,
                COIL_SHORT_ERR=0x40,COIL_BREAK_ERR=0x80,
                ERR_SENSOR_SMOKE=0x100,ERR_SENSOR_TEMP=0x200,
                ERR_TOP_OVERROLL=0x400,//过卷
                ERR_BOT_OVERROLL=0x800,//过卷
                ERR_MOTOR_LINE=0x1000//电机断线检测
                };         
               //相位错误      过热保护      主电错误        备电电压低       电池损坏
////////////////////////////////////////////////////////////////////////
//#define DEFAULT_OVERROLL_TIME 300        //300X50=15S//默认值
#define DEFAULT_OVERROLL_TIME 100           //100*50=5S//默认值
void UpdateIO(void); 
void Translate(unsigned char out);
unsigned char Receive(void);
//void Translate3(unsigned char out);
 
//////////////////////////////////////
void InitKey(void);
void InitBeep(void);// 
//void Beep(unsigned char    cNumBeep,unsigned int beepTime);

//void SetWarningFreq(unsigned  int freq); ;
void StopBeep(void)     ;
void ErrorStopMotor(void); 
void run_vol_set(void);
void charge_vol_set(void);	
void jiance_vol_set(void);								
void setDIR(void);
void StopMotor(void);
void Cont_Break_Ctrl(void); 
void TaskWarn(void);
void DebugCtrl(void); 
 void DebugCtrl_DC(void);
void GetCurretPos(void);
void MotorDelay(void *p);
signed char MovMotor(unsigned char cPos);
void TaskStopMotor(void) ;    
void Task_WDT(void);

void checkUserMode(void);

void TaskStateCheck(void); 
void TaskSelfTestLED(void* param);
///////////////////////////// 
void MotorMid(void *p);
void MidWait(void* p); 
void MotorDown(void *p); 

void MotorDown_Middle(void *p);
void MotorUp(void *p);
///////////////////////////// 
void StartWarning(void);
void StopWarning(void);
void MuteError(void);
void MuteError2(void *p);
/////////////////////////////
void Feed(void);   
void BlinkMotorErrLed(void* param);  
void SelfTest(void);
///////////////

//int abs(int x) ;
void GetChildState(void);   
void WriteLEDInfo(int led);    
void ManualCtrl(void); 
void InitMotorInterrupt(void); 
void EndMotorInterrupt(void);
void SetLock(int bLock);
int GetLock(void); 

extern char CtrlHalfDown;
extern char CtrlAllDown;
extern int MotorLineCnt;
#ifdef __cplusplus
}
#endif
#endif 
