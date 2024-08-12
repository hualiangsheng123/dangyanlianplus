/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TASK_H__
#define __TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/******************************************************************************
    about task
*******************************************************************************/
/*-----------------------------------------------------------------------------
    定义任务表，增加任务时需在此增加表项。
-------------------------------------------------------------------------------*/
enum TASK_TYPE {
    TASK_TIMER=1,TASK_BLINK,TASK_IO,
    TASK_WDT,TASK_HANDWARN,
    TASK_WARN,TASK_STOPMOTOR,TASK_FEED, 
    TASK_FREQ2,
    TASK_KEY,TASK_STATE,TASK_BREAK,
    TASK_POWER_CHECK,TASK_CMMD,TASK_INVERT,
    TASK_CHARGE    , TASK_CHECK_COIL,TASK_CHECK_MOTOR,
      TASK_ANJIAN,
    TASK_SENSOR ,TASK_LED};          

    enum DIR{DIR_INPUT=0,DIR_OUTPUT=1};
#define TICKCOUNT (1000L)     //系统时间脉冲为1000次        
#define PHASE_PRIOD 2000L 
    
#define __NOP __nop 
#define _nop_ __nop

enum TASK_MODE {TASK_MODE_ONCE,TASK_MODE_PERIOD}; 
 
typedef  unsigned char             UCHAR;
typedef  unsigned long             ULONG;
typedef  unsigned int              UINT;
typedef  unsigned short             USHORT;
typedef  void *                       PVOID;
typedef  signed char               SCHAR;
typedef  signed long                SLONG    ;
typedef  signed int                SINT;         
 
typedef  unsigned char          u8;
typedef  unsigned char          uint8;
typedef  unsigned short         uint16;
typedef  unsigned short         u16;
typedef  unsigned int           uint32;
typedef  unsigned int           u32;


typedef  volatile unsigned short         vu16;
 
typedef struct task
{
    UCHAR   cActive;            // >0:活动,1:初始化后立即启动
    UCHAR   cTaskMode;          // 任务工作方式
    SLONG   lLastTime;
    SLONG   lPeriod;            // 活动周期,uint=1mS
    UCHAR   cType;              // 任务类型
    void    (*TaskProc)(void);  // 任务处理子程序

}TASK_DATA;       
/***************************************************************************
    about timer
****************************************************************************/
#define MAX_TIMER               8         // the number of timer can be used

#define TIMER_MODE_TIMER        0           // mode constant: timer only
#define TIMER_MODE_ONCEROUTINE  1           // timerly routine ( once )
#define TIMER_MODE_CYCROUTINE   2           // cyclely routine

// define timer data struct
typedef struct mytimer
{
    UCHAR   cUse;
    UCHAR   cMode;                  // timer work mode
    int8_t *  TimerID;                //定时器ID
    long    lPeriod;                // timer period, unit=10ms.
    long    lLastTime;              // timer count for reload        
    void    (*routine)(void *   Param1);        // user's timer routine
    void *   Param1;
}TIMER_DATA; 
extern TIMER_DATA TimerData[MAX_TIMER];
 
extern UCHAR gc1ms  ;                //1ms到标志，控制多任务处理的时间隔为1ms
extern UCHAR TMP1msCount ;
typedef struct
{
  UINT  iYear;        // year since 2000
  UCHAR cMon;        // months since January - [1, 12]
  UCHAR cDay;        // day of the month - [1, 31]
  UCHAR cHour;        // hours since midnight - [0, 23] 
  UCHAR cMin;        // minutes after the hour - [0, 59 ]
  UCHAR cSec;        // seconds after the minute - [0, 59] 
}TIME;
typedef unsigned long time_t;
// function prototype
//void Init_task_timer(void); 
void DelayUs(uint32_t us); 
void TaskTimer(void);
void TaskProc(void);   
//Task Function 
void ActiveTask(UCHAR cTaskType, ULONG lPeriod);
void SuspendTask(UCHAR cTaskType);         
//Timer Function 
SCHAR StartTimer(UCHAR cMode,ULONG lPeriod,void (* routine)(void *   Param1),void *   Param1);     
void StartTimer2(int8_t *TimerID,UCHAR cMode,ULONG lPeriod,void (* routine)(void *   Param1),void *   Param1);
SCHAR StopTimer (SCHAR * pcTimerNumber );
void StopAllTimer(void);
//

void SetTaskLastTime(UCHAR cTaskType,ULONG lLastTime);
int TaskState(UCHAR cTaskType);
void ActiveTaskNow(UCHAR cTaskType, ULONG lPeriod);
void InitPLL(void);              
void TaskBlinLed(void);           
   
void DelayMS(uint32_t delayCnt) ; 
void Start_OneLine(void);
void SendByte_Oneline(unsigned char addr);
void send_data(unsigned char addr);
void Task_Anjian_Check(void);//按键检测处理任务
int ADC1_KeySample(void);//按键取值
int ADC2_KeySample(void);//远程控制取值
 //day time
time_t MakeTime(TIME *ptm);
void LocalTime(TIME *t,time_t *tSec);  
extern TASK_DATA TaskDb[] ;
extern unsigned long RunTimer;
#define nop _nop_
#define enable()   __enable_irq()   //开中断
#define disable()  __disable_irq()    //关中断 
enum _BOOL{FALSE=0,TRUE=0xff} ;      

//#define BP() __BKPT(0)
#define BP() __breakpoint
#ifdef __cplusplus
}
#endif
#endif 
