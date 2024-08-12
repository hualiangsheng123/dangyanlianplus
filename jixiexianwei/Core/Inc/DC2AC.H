#ifndef _DC2AC_H_
#define _DC2AC_H_

//#include "adc.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#define FREQ_DC2AC  18000UL
#define DC_DN_PR 6000
void InitDC2AC(void* p);
void StartPulse(void* p);
void DeInitDC2AC(void *p);  
void InitInvert(void *p);
void StopInvert(void ); 
 
void StopDC2ACConvert(void* p);
void EnableBrakPower (unsigned char bEnable);
void StartCheckBrake(void *p);

void EnableBattary(void);        
void DisableBattary2(void* p );
void DisableBattary(void);
void DeChargeBat(char bEnable);
void TaskChargeBat(void);  
void BatCheck(void);
void GetVoltage(void);

void Task_DC_Brake_down(void)      ;
void StartDC2ACConvert(void);


void ActiveTaskCheckPhase(void);
void TaskCheckBrakeCoil(void); 
 
void TaskCheckSensor(void);
void TaskCommand(void);
void Task_Power_Check(void);        //µÁ—π¥ÌŒÛ–≈∫≈ºÏ≤‚
void AddCommand(unsigned char cMotorCmd); 
void ResumeMotorRun(void *p)  ; 

void DelayShutDown(void);
extern double voltage[7];  
#ifdef __cplusplus
}
#endif
#endif 
