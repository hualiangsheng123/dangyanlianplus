#ifndef _KEY_H_
#define _KEY_H_
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define KEY_DUMMY   0X80 
#define KEY_MUTE    0X08  
#define KEY_DOWN    0x04 
#define KEY_STOP    0x02
#define KEY_UP      0x01
#define KEY_STOP2   0X82  //ÖÐÍ£2±£´æ
#define KEY_LOCK    0x88  //Ëø¶¨ 
#define KEY_DOWN_yuancheng    0x14 
#define KEY_STOP_yuancheng    0x12
#define KEY_UP_yuancheng      0x11



#define KEY_DEBUG        (KEY_UP|KEY_DOWN)
#define KEY_HANDCTRL     (KEY_UP|KEY_DOWN|KEY_STOP|KEY_MUTE|KEY_UP_yuancheng|KEY_DOWN_yuancheng|KEY_STOP_yuancheng)
#define KEY_STOPWARN     (KEY_UP|KEY_STOP)
#define KEY_SELFTEST     (KEY_DOWN|KEY_STOP)


#define KEY_MAP          (KEY_UP|KEY_STOP|KEY_DOWN|KEY_MUTE)  
#define KEY_MOV          (KEY_UP|KEY_STOP|KEY_DOWN)  

#define MAX_KEY_STACK    10
#define MAX_KEY_FILTER  1

extern unsigned char gcKey;

unsigned char GetKey(void);
void TaskScanKey(void);
#ifdef __cplusplus
}
#endif
#endif 
