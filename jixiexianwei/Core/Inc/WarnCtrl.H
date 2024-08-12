#ifndef _WARN_CTRL_H_
#define _WARN_CTRL_H_ 
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
void DelayMoveMotor(void * p );
void ACHostWarnCtrl(void);
void DCHostWarnCtrl(void);
void HostWarnCtrl(void);
void ACHostWarnMode1(void);
void ACHostWarnMode2(void);

void DCManualCtrl(void); 
void ACManualCtrl(void); 
void ManualCtrl(void);

void ACSaveLifeCtrl(void);
void DCSaveLifeCtrl(void);
void SaveLifeCtrl(void);

void MuteRun(void);
#ifdef __cplusplus
}
#endif
#endif 
