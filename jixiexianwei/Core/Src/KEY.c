#include "key.h"
/*
    unsigned char gcOldKey = 0;
    unsigned char gcKeyFilter = 0;
    unsigned char gcKeyStack[MAX_KEY_STACK] = {"\0"};
    unsigned char gcKeySavePtr= 0;
    unsigned char gcKeyGetPtr = 0;  
    */
typedef struct
{
    unsigned char OldKey  ;
    unsigned short KeyFilter  ;
    unsigned char KeyStack[MAX_KEY_STACK]  ;
    unsigned char KeySavePtr ;
    unsigned char KeyGetPtr  ;  
}KEYPAD;

KEYPAD keyPad[3]={{0,0,0,0},{0,0,0,0},{0,0,0,0}}; 


#define gcOldKey      keyPad[0].OldKey
#define gcKeyFilter   keyPad[0].KeyFilter
#define gcKeyStack    keyPad[0].KeyStack
#define gcKeySavePtr  keyPad[0].KeySavePtr  
#define gcKeyGetPtr   keyPad[0].KeyGetPtr   

unsigned char gcKey = 0;                                 //键盘键值                                            
unsigned char  yuancheng_flag;
unsigned char KEY_DOWN_UNLOCK=0;
unsigned char KEY_LOCK_FLAG=0;
extern unsigned int send_data_flag;
extern unsigned int send_end_timer;
extern unsigned char sound_feed_on;
//IED.b_warning
void TaskScanKey(void)
{    
   // int i = 0;
    unsigned short Key ;
//    for(i=0;i<3 ;i++) 
//    {  
        Key= CHILD[0].Key;     //按键输入     //去掉LOCK,上下到位  半降全降信号  
        if((!IED.b_warning) &&(CHILD[0].LOCKLED ))   
       {
//            //锁定状态,过滤运动按键  
//            //没有报警,过滤键值         
            if((Key==KEY_UP)||(Key==KEY_STOP)||(Key==KEY_DOWN)||(Key==KEY_STOP))        //还原按键音
						{
//							Beep(1,100); 
							 if(send_data_flag==0)
		           {
			         send_data_flag=1;
				       send_end_timer=8;
				       send_data(0x0d);//
		           }
							
						}
//                Key = KEY_DUMMY      ;
//            Key&= ~KEY_MOV; 
       }   

        if(Key!=0x0)
        {                                          
            if(Key!=gcOldKey)                //切换按键表示有新键值
            {
                gcOldKey=Key;
                gcKeyFilter=0;
              if(CHILD[0].LOCKLED==0)
							{
								 if(send_data_flag==0)
		           {
			         send_data_flag=1;
				       send_end_timer=8;
				      send_data(0x0e); //
		           }
						  
							 sound_feed_on=1;
							}
							if((Key==KEY_UP_yuancheng)||(Key==KEY_DOWN_yuancheng)||(Key==KEY_STOP_yuancheng))
							{
						   yuancheng_flag=1;
							}
            }
            else
            {                                 //长按不放的情况
                gcKeyFilter++;  
                if(gcKeyFilter>3000/10)  
                {
                    gcKeyFilter=0;       								
                    if((Key==KEY_UP)&&(CHILD[0].LOCKLED==0))                        //长按2.5S 上升就变成了调试按键    
										{
                        gcKeyStack[gcKeySavePtr]=KEY_DEBUG; 
											 gcKeySavePtr=(gcKeySavePtr+1)%MAX_KEY_STACK;  
										}
                    if((Key==KEY_STOP)&&(CHILD[0].LOCKLED==0))     //长按2.5S 停止就变成了中停2按键 
										{  
											gcKeyStack[gcKeySavePtr]=KEY_STOP2; 
                     gcKeySavePtr=(gcKeySavePtr+1)%MAX_KEY_STACK;  											
										}
										 if(Key==KEY_MUTE)    //长按2.5S 消音就变成了锁按键
                      {
											   KEY_LOCK_FLAG=1;
                          CHILD[0].LOCKLED = !CHILD[0]. LOCKLED;
									    
                        if(CHILD[0].LOCKLED ==0)   //不一致,总开关失效                           
                        { 
                            SetLock(0);//解锁	
                        } 
												else{SetLock(1);}
                      
                    }		
	                 if((Key==KEY_DOWN)&&(CHILD[0].LOCKLED==1))
					            	{
													KEY_DOWN_UNLOCK=1;
							      
					 	            }
               												
                    if(send_data_flag==0)
		           {
			         send_data_flag=1;
				       send_end_timer=8;
				      send_data(0x0e); //
		           }      
                } 
                if((gcKeyFilter>MAX_KEY_FILTER))      //save key to stack
                {                                        
                    if(((Key&KEY_STOPWARN)==KEY_STOPWARN)
                    ||((Key&KEY_DEBUG)==KEY_DEBUG)
                    ||((Key&KEY_SELFTEST)==KEY_SELFTEST))
                    {    
                        int preKeySavePtr = 0;
                        gcKeyFilter=0;       
                        
                        preKeySavePtr=(preKeySavePtr+MAX_KEY_STACK-1)% MAX_KEY_STACK;
                    //    if((gcKeyStack[preKeySavePtr]!=Key )) //不等于以前的值,或者松开后按的键值值
                    //    {
                            gcKeyStack[gcKeySavePtr]=Key;
                            gcKeySavePtr=(gcKeySavePtr+1)%MAX_KEY_STACK;   
                              if(send_data_flag==0)
		           {
			         send_data_flag=1;
				       send_end_timer=8;
				      send_data(0x0e); //
		           }       
                    //    }                        
                    }
                    if(IED.cMoving)
                    {        
                        gcKeyFilter=0;       
                        if(((Key&KEY_STOP)==KEY_STOP)) 
                        {
                            gcKeyStack[gcKeySavePtr]=Key;
                            gcKeySavePtr=(gcKeySavePtr+1)%MAX_KEY_STACK;   
                             if(send_data_flag==0)
		           {
			         send_data_flag=1;
				       send_end_timer=8;
				      send_data(0x0e); //
		           }        
                        } 
                    } 
                } 
            }             
        }
        else
        { 
					
            if((gcKeyFilter>=1)&&((CHILD[0].LOCKLED==0) ||(IED.b_warning==1)||(yuancheng_flag==1)))            //延时去抖动 ,按键松开的情况
            {
                gcKeyFilter=0;
                gcKeyStack[gcKeySavePtr]=gcOldKey;
                gcKeySavePtr=(gcKeySavePtr+1)%MAX_KEY_STACK;
                if(send_data_flag==0)
		           {
			         send_data_flag=1;
				       send_end_timer=8;
				      send_data(0x0e); //
		           }  
              yuancheng_flag=0;							 
            }    
            if((gcKeyFilter>2)&&(KEY_DOWN_UNLOCK==1)&&(CHILD[0].LOCKLED==1))
					            	{
							           CHILD[0].LOCKLED =0;
											  SetLock(0);
					 	            }						

								KEY_DOWN_UNLOCK=0;
						    KEY_LOCK_FLAG=0;
						 gcKeyFilter=0;
        }
        #ifdef _DEBUG                                 
        //printf("Key = 0x%X \n",(int)Key);    
        #endif
//    }
}
                                                                                  
                                                           
/**************************************************************************
    get key code,return 0 if no key
***************************************************************************/
unsigned char GetKey(void)
{
    unsigned char ret=0;
//    int i =0;
//    for(i=1;i<3;i++)
//    {
        if(gcKeyGetPtr!=gcKeySavePtr)
        {
            ret=gcKeyStack[gcKeyGetPtr];
            gcKeyGetPtr=(gcKeyGetPtr+1)%MAX_KEY_STACK;
        }
        else
            ret=0;    
        if(ret!=0)
        { 
            //不为锁定,延迟加锁
            if((ret!=KEY_LOCK)&&(ret!=KEY_DUMMY)&&(ret!=0)&&(CHILD[0].LOCKLED==0))
                SetLock(0);
          //  break;
        }
//    }
    return ret;
}
//
