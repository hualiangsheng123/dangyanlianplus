#include "WarnCtrl.h"
signed char bMiddleWaitTimerID = -1;
//中停
char  bMidWait = FALSE;
void SetMidWait(char _bMidWait)
{
    if(bMidWait!=_bMidWait)
    {       
     //   printf("bMidWait = %d\n",bMidWait);
        bMidWait =   _bMidWait; 
    }
}
void HostWarnCtrl(void)
{
    IED.curWarnState =((IED.Err&(MAIN_POWER_ERROR|ErrPhase|ERR_MOTOR_LINE))!=0)?DCWarn:ACWarn;

    if(IED.preWarnState!=IED.curWarnState)  //报警状态发生切换,重新初始化报警状态
    {
        IED.IED_DebugState = WarnInit;
    }
    if(IED.curWarnState==DCWarn)
    {
        //DCHostWarnCtrl();
			 ACHostWarnCtrl();
    }
    else
    {
        ACHostWarnCtrl();
    }
    IED.preWarnState =  IED.curWarnState;
    SaveLifeCtrl();           ///////逃生模式 
    Cont_Break_Ctrl();        //点动，连续控制
}

//__attribute__((section("RAMCODE")))
void ACHostWarnCtrl(void)
{ 
    if(ProgSelect==1)    
    {
        ACHostWarnMode1();
    }
    else
    {
        ACHostWarnMode2();
    }
}


   

void DelayMoveMotor(void * p )
{
    char cPos = *(char*)p;
    MovMotor(cPos);
}


void SetIED_DebugState(
    unsigned char IED_DebugState)
{
    IED.IED_DebugState =  IED_DebugState;
    switch(IED_DebugState)
    {
        case WarnInit:
       //     printf("WarnInit!\n");
            break;    
        case WarnMidWait:    
      //      printf("WarnMidWait!\n");
            break;
        case WarnMoveMid:    
     //       printf("WarnMoveMid!\n");
            break;
        case WarnMoveBot:   
     //       printf("WarnMoveBot!\n");
            break;
        default :   
     //       printf("default!\n");
            break;
    }
}
char DestPos = POS_MIDDLE;
void DCHostWarnCtrl(void)
{
    switch(IED.IED_DebugState)
    {
        case  WarnInit:                                                   
        {
            if((IED.b_SmokeSensor==1)||(CtrlHalfDown==0)||(IED.b_Halfdown==1)     //下降信号,到中停
              ||(IED.b_TempSensor==1)||(CtrlAllDown==0)||(IED.b_Alldown==1))       
            {                     
                //TODO:启动半降信号    
                #ifdef _DEBUG
                printf("半降信号\n");
                #endif
                if( IED.cMoving==1)             //如果向上，停止
                {                              
                    #ifdef _DEBUG
                    printf("电机运行，停止上行，1S后半降运行");
                    #endif
                    StopMotor();
                    //IED.MotorMoveDelayTimerID = StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorMid,0);     //一秒后启动运行到中位  
                    StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0);     //一秒后启动运行到中位    
                }
                else
                {
                    #ifdef _DEBUG
                    printf("电机 半降运行");
                    #endif
                    
                    MovMotor(POS_MIDDLE);//启动半降信号  
                }
                if((IED.b_TempSensor==1)||(CtrlAllDown==0)||(IED.b_Alldown==1))
                {
                    SetMidWait(TRUE);
                } 
                else              
                    SetMidWait(FALSE); 
                //IED.IED_DebugState = WarnMoveMid;  
                SetIED_DebugState(WarnMoveMid);                
            }            
        }                                 
        StartWarning();       
        break;
    case WarnMidWait: 
        if(IED.cPos==POS_MIDDLE)
        {
            if(IED.MotorMoveDelayTimerID == -1)
            { 
                StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MidWait,0);
                //IED.IED_DebugState = WarnMoveBot;      //延时下行  
                SetIED_DebugState(WarnMoveBot);        
            }
        }
        
        if((IED.cPos==POS_TOP)&&(IED.cMoving==0)) 
        {   
            if(IED.MotorMoveDelayTimerID == -1)
            {    
                StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,200,DelayMoveMotor,&DestPos);
            }
        }
        //错误运行到顶位,立即下行  
        break;
    case WarnMoveMid:    
        //如果起始下降信号由全降给的,则延时启动下降 
    
        if(bMidWait)        
        {
            if(IED.cPos==POS_MIDDLE)
            {
                 StartTimer2(&bMiddleWaitTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorDown,0);
                //IED.IED_DebugState     = WarnMoveBot;            
                SetIED_DebugState(WarnMoveBot);      
                    SetMidWait(FALSE);
            } 
        }
        else    //如果是半降信号引起的下降,收到全降型信号后下降
        {
            if((IED.cPos==POS_MIDDLE)&&((CtrlAllDown==0)||(IED.b_Alldown==1)||(IED.b_TempSensor==1)))
            {                        
                #ifdef _DEBUG                
                printf("全降信号\n");                
                #endif    
                
                MovMotor(POS_BOT);//启动全降信号
                //IED.IED_DebugState = WarnMoveBot;       
                SetIED_DebugState(WarnMoveBot);                 
            }
        }   
/*        
        if((IED.cPos==POS_TOP)&&(IED.cMoving==0)) 
        {   
            if(IED.MotorMoveDelayTimerID == -1)
            {    
                DestPos = POS_MIDDLE;
                IED.MotorMoveDelayTimerID = StartTimer(TIMER_MODE_ONCEROUTINE,200,DelayMoveMotor,&DestPos);
            }
        }   
*/        
        break;
    case WarnMoveBot:  
        _nop_();
        
        break;         
    default:break;
    }          
} 
unsigned char halfdown_life;
//__attribute__((section("RAMCODE")))
void ACHostWarnMode1(void)  //疏散模式
{
    switch(IED.IED_DebugState)
    {
        case  WarnInit:         
        {
            #ifdef _DEBUG
            if(IED.IED_PreDebugState!=IED.IED_DebugState)
            {
                IED.IED_PreDebugState = IED.IED_DebugState;
                printf("IED.IED_DebugState=WarnInit\n");
            }
            #endif    
            #ifdef _DEBUG
            printf("关门模式一\n");
            #endif            
               if(((IED.b_SmokeSensor==1)||(CtrlHalfDown==0)||(IED.b_Halfdown==1))
              &&(IED.b_TempSensor==0)&&(CtrlAllDown==1)&&(IED.b_Alldown==0))      //只有半降信号
            {     
                SetMidWait(FALSE);                           
//                HALFDOWN:
                #ifdef _DEBUG
                printf("半降信号\n");
                #endif
                if( IED.cMoving==1)             //如果向上，停止
                {        
                    if(IED.PreMotor_Cmd!=POS_MIDDLE)
                    {
                        #ifdef _DEBUG
                        printf("电机运行，停止上行，1S后半降运行");
                        #endif
                        StopMotor();
											run_vol_set();
                        if(IED.MotorMoveDelayTimerID ==-1)
                        StartTimer2(&IED.MotorMoveDelayTimerID ,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0);
                        //一秒后启动运行到中位    
                    }
                }
                else
                {
                    #ifdef _DEBUG
                    printf("电机 半降运行");
                    #endif 
                   // MovMotor(POS_MIDDLE);//启动半降信号
									run_vol_set();
									if(IED.MotorMoveDelayTimerID ==-1)
									 StartTimer2(&IED.MotorMoveDelayTimerID ,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0);
                }
                IED.IED_DebugState = WarnMoveMid;   
            }
            else if((IED.b_TempSensor==1)||(CtrlAllDown==0)||(IED.b_Alldown==1))
              //有全降信号
            {  
                 SetMidWait(TRUE); ; 
                //if(KEY_MODE!=USER_MODE)   //非用户模式下
                {
                    #ifdef _DEBUG
                    printf("全降信号\n");
                    #endif
                    if( IED.cMoving==1 )             //如果向上，停止    
                    {
                        #ifdef _DEBUG
                        printf("电机向上运行，停止后1S后全降运行");
                        #endif
                        if((IED.Direction==MOTOR_UP)||(    IED.PreMotor_Cmd  !=POS_BOT))
                        {
                            StopMotor(); 
                            run_vol_set();													
                            if(IED.MotorMoveDelayTimerID ==-1)
                             StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,100L,MotorDown,0);//一秒后启动全降信号              
                        }
                    }
                    else
                    {
                        #ifdef _DEBUG
                        printf("电机 全降运行");
                        #endif 
                       // MovMotor(POS_BOT);//启动全降信号
											run_vol_set();	
											if(IED.MotorMoveDelayTimerID ==-1)
                             StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,100L,MotorDown,0);//一秒后启动全降信号  
                    }
                    IED.IED_DebugState = WarnMoveBot;
                }
                /*
                else
                {
                    //用户模式下 半降
                    goto HALFDOWN;
                }
                */
                  
            }                                
        }                              
        StartWarning();       
        break;
    case WarnMidWait:
        #ifdef _DEBUG
        if(IED.IED_PreDebugState!=IED.IED_DebugState)
        {
            IED.IED_PreDebugState = IED.IED_DebugState;
            printf("IED.IED_DebugState=WarnMidWait\n");
        }
        #endif    
         
        if((IED.cPos==POS_MIDDLE)&&(IED.cMoving==0))
        {    
            if(IED.MotorMoveDelayTimerID == -1)                
            {            
                StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MidWait,0);
                IED.IED_DebugState = WarnMoveBot;     
            } 
        }  
        if((IED.cPos==POS_TOP)&&(IED.cMoving==0))
        {     
            // //一秒后启动运行到中位      
            if(IED.MotorMoveDelayTimerID == -1)
            {    
                DestPos = POS_MIDDLE; 
                if((CtrlAllDown==0)||(IED.b_Alldown==1)||(IED.b_TempSensor==1))
                {
                    DestPos = POS_BOT;
                }  
								run_vol_set();
                StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,200,DelayMoveMotor,&DestPos);
            } 
            //错误运行到顶位,立即下行  
        }
        break;
    case WarnMoveMid:        
//       if(KEY_MODE!=USER_MODE)   //非用户模式下(调试模式)
 //       {
            #ifdef _DEBUG 
            if(IED.IED_PreDebugState!=IED.IED_DebugState)
            {
                IED.IED_PreDebugState = IED.IED_DebugState;
                printf("IED.IED_DebugState=WarnMoveMid\n");
            }
            #endif  						
            if(((CtrlHalfDown==0)||(IED.b_Halfdown==1)||(IED.b_SmokeSensor==1))&&
            (CtrlAllDown==1)&&(IED.b_Alldown==0)&&(IED.b_TempSensor==0))      //只有半降信号
            {      
                if((IED.TimerS!=IED.TimeTopToMid)&&(IED.cMoving==0))
                //if((!IED.cMoving)&&(IED.cPos!=POS_MIDDLE)    )      
                //如果没有运行,运行到中位
                {     
                    DestPos = POS_MIDDLE;  
                            //TODO:启动半降信号    
                    #ifdef _DEBUG
                    printf("半降信号\n");
                    #endif   
									run_vol_set();
                    StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,200,DelayMoveMotor,&DestPos);
                }                                                  
            }
            else if((CtrlAllDown==0)||(IED.b_Alldown==1)||(IED.b_TempSensor==1))
            {
                if((IED.cMoving)&&(IED.PreMotor_Cmd==POS_MIDDLE))         // 如果正在运行到中位
                {                                                         //停车
                    #ifdef _DEBUG
                    printf("全降信号\n");
                    #endif     
                    StopMotor();
                }
								run_vol_set();
                MotorDown(0);
                IED.IED_DebugState = WarnMoveBot;
            }         
 //       }
//        else//工作模式
//        {
//            if((IED.cPos==POS_MIDDLE)&&(IED.cMoving==0))
//            {    
//                if(IED.MotorMoveDelayTimerID == -1)                
//                {            
//                    StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorDown,0);
//                    IED.IED_DebugState = WarnMoveBot;     
//                } 
//            }  
//        } 
        if((IED.cPos==POS_TOP)&&(IED.cMoving==0)) 
        {   
            if(IED.MotorMoveDelayTimerID == -1)
            {    
                DestPos = POS_MIDDLE; 
                run_vol_set();							
                StartTimer2(&IED.MotorMoveDelayTimerID ,TIMER_MODE_ONCEROUTINE,200,DelayMoveMotor,&DestPos);
            }
        }
        break;
    case WarnMoveBot:  
        #ifdef _DEBUG 
        if(IED.IED_PreDebugState!=IED.IED_DebugState)
        {
            IED.IED_PreDebugState = IED.IED_DebugState;
            printf("IED.IED_DebugState=WarnMoveBot\n");
        }
        #endif    
        _nop_();
        break;         
    default:break;
    }                
}
unsigned char Warn_flag;
// __attribute__((section("RAMCODE")))
void ACHostWarnMode2(void)  //隔离模式
{
    switch(IED.IED_DebugState)
    {
        case  WarnInit:         
        {
            #ifdef _DEBUG
            if(IED.IED_PreDebugState!=IED.IED_DebugState)
            {
                IED.IED_PreDebugState = IED.IED_DebugState;
                printf("IED.IED_DebugState=WarnInit\n");
            }
            #endif    
            #ifdef _DEBUG
            printf("关门模式一\n");
            #endif            
            
//               if(( (CtrlHalfDown==0)||(IED.b_Halfdown==1))
//                &&(IED.b_SmokeSensor==0)
//              &&(IED.b_TempSensor==0)&&(CtrlAllDown==1)&&(IED.b_Alldown==0))      //只有半降信号
//               
//               
//            {                      
//                #ifdef _DEBUG
//                printf("半降信号\n");
//                #endif
//                if( IED.cMoving==1)             //如果向上，停止
//                {        
//                    if(IED.PreMotor_Cmd!=POS_MIDDLE)
//                    {
//                        #ifdef _DEBUG
//                        printf("电机运行，停止上行，1S后半降运行");
//                        #endif
//                        StopMotor();
//                        if(IED.MotorMoveDelayTimerID ==-1)
//                        StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0);
//                        //一秒后启动运行到中位    
//                    }
//                }
//                else
//                {
//                    #ifdef _DEBUG
//                    printf("电机 半降运行");
//                    #endif 
//                    MovMotor(POS_MIDDLE);//启动半降信号
//                }
//                IED.IED_DebugState = WarnMoveMid;             
//            }
        if(IED.cPos!= POS_BOT)
				{		
           if((IED.b_TempSensor==1)||(CtrlAllDown==0)||(IED.b_Alldown==1)
                ||(IED.b_SmokeSensor==1)||(CtrlHalfDown==0)||(IED.b_Halfdown==1))                           //有全降信号
            {   
							//  Warn_flag=1;
                #ifdef _DEBUG
                printf("全降信号\n");
                #endif
                if( IED.cMoving==1 )             //如果向上，停止    
                {
                    #ifdef _DEBUG
                    printf("电机向上运行，停止后1S后全降运行");
                    #endif
                  //  if(IED.Direction==MOTOR_UP)
									  if(IED.PreMotor_Cmd==POS_TOP)
                    {
                        StopMotor();  
                        run_vol_set();											
                        if(IED.MotorMoveDelayTimerID ==-1)
                        //StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,100L,MotorDown,0);//一秒后启动全降信号           MotorMid  
                        StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0);//一秒后启动全降信号      
                    }
                }
                else
                {          
                    DestPos  = POS_MIDDLE;
                    #ifdef _DEBUG
                    printf("电机 全降运行");
                    #endif 
                  //  MovMotor(POS_MIDDLE);//启动全降信号
									run_vol_set();											
                        if(IED.MotorMoveDelayTimerID ==-1) 
                        StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0);//一秒后启动全降信号     
                }
                IED.IED_DebugState = WarnMoveBotWAIT;
            } 
					}						
        }                              
        StartWarning();       
        break;
    case WarnMidWait:
        #ifdef _DEBUG
        if(IED.IED_PreDebugState!=IED.IED_DebugState)
        {
            IED.IED_PreDebugState = IED.IED_DebugState;
            printf("IED.IED_DebugState=WarnMidWait\n");
        }
        #endif    
        if((IED.cPos==POS_MIDDLE)&&(IED.cMoving==0))
        {    
            if(IED.MotorMoveDelayTimerID == -1)                
            {            
                StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MidWait,0);
                IED.IED_DebugState = WarnMoveBot;     
            } 
        }  
        if((IED.cPos==POS_TOP)&&(IED.cMoving==0))
        {       
             // //一秒后启动运行到中位     
            if((IED.cPos==POS_TOP)&&(IED.cMoving==0)) 
            {   
                if(IED.MotorMoveDelayTimerID == -1)
                {    
                    DestPos = POS_MIDDLE; 
                    if((CtrlAllDown==0)||(IED.b_Alldown==1)||(IED.b_TempSensor==1)||(IED.b_SmokeSensor==1))
                    {
                        DestPos = POS_BOT ;
                    } 
										run_vol_set();		
                    StartTimer2(&IED.MotorMoveDelayTimerID ,TIMER_MODE_ONCEROUTINE,200,DelayMoveMotor,&DestPos);
                }
            }
        }
        break;
    case WarnMoveMid:        
        #ifdef _DEBUG 
        if(IED.IED_PreDebugState!=IED.IED_DebugState)
        {
            IED.IED_PreDebugState = IED.IED_DebugState;
            printf("IED.IED_DebugState=WarnMoveMid\n");
        }
        #endif    
//        if(( (CtrlHalfDown==0)||(IED.b_Halfdown==1))
//                &&(IED.b_SmokeSensor==0)
//              &&(IED.b_TempSensor==0)&&(CtrlAllDown==1)&&(IED.b_Alldown==0))      //只有半降信号
//        {     
//            
//            if((IED.TimerS!=IED.TimeTopToMid)&&(IED.cMoving==0))
//            //if((!IED.cMoving)&&(IED.cPos!=POS_MIDDLE)    )      
//            //如果没有运行,运行到中位
//            {    
//                        //TODO:启动半降信号    
//                #ifdef _DEBUG
//                printf("半降信号\n");
//                #endif  
//                //MovMotor(POS_MIDDLE); ; 
//                if(IED.MotorMoveDelayTimerID == -1)
//                {    
//                    DestPos  = POS_MIDDLE;
//                    StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,200,DelayMoveMotor,&DestPos);
//                }
//            }                                                  
//        }
         if((CtrlAllDown==0)||(IED.b_Alldown==1)||(IED.b_TempSensor==1)||(IED.b_SmokeSensor==1)||(CtrlHalfDown==0)||(IED.b_Halfdown==1))
        {
            if((IED.cMoving)&&(IED.PreMotor_Cmd==POS_MIDDLE))         // 如果正在运行到中位
            {                                                         //停车
                #ifdef _DEBUG
                printf("全降信号\n");
                #endif     
                StopMotor();
            }
						run_vol_set();		
            MotorDown(0);
            IED.IED_DebugState = WarnMoveBot;
        }     
        //一秒后启动运行到中位      
        if((IED.cPos==POS_TOP)&&(IED.cMoving==0)) 
        {   
            if(IED.MotorMoveDelayTimerID == -1)
            {    
                DestPos = POS_MIDDLE; 
                run_vol_set();									
                StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,200,DelayMoveMotor,&DestPos);
            }
        }
        break;
    case WarnMoveBotWAIT:
        if((IED.cPos==POS_MIDDLE)&&(IED.MotorMoveDelayTimerID==-1))
        {
					run_vol_set();		
            StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorDown,0); 
        }   
        if((IED.cPos==POS_TOP)&&(IED.cMoving==0))
        {
           run_vol_set();		
            StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorDown,0); 
        }
        break;
    case WarnMoveBot:  
        #ifdef _DEBUG 
        if(IED.IED_PreDebugState!=IED.IED_DebugState)
        {
            IED.IED_PreDebugState = IED.IED_DebugState;
            printf("IED.IED_DebugState=WarnMoveBot\n");
        }
        #endif    
        _nop_();
        break;         
    default:break;
    }            
}




void DCManualCtrl(void)
{ 
        #ifdef _DEBUG
//        if(gcKey&KEY_HANDCTRL)
//            printf("直流手动控制\n") ;
        #endif
        switch(gcKey&KEY_HANDCTRL)     //(KEY_UP|KEY_DOWN|KEY_STOP)
        {
            case KEY_STOP:
                if(bMiddleWaitTimerID!=-1)
                    StopTimer(&bMiddleWaitTimerID);
                StopMotor(); 
                break;
						 case KEY_STOP_yuancheng:
                if(bMiddleWaitTimerID!=-1)
                    StopTimer(&bMiddleWaitTimerID);
                StopMotor(); 
                break;
            case KEY_DOWN:  
                if(IED.SlideTimeTopToMid>IED.SlideTimeOutTimer) 
                //下滑中停位置上 
                { 
                    MovMotor(POS_MIDDLE);//启动半降信号
                     SetMidWait(TRUE); ; 
                }
                else
                { 
                    MovMotor(POS_BOT);//启动全降信号
                    SetMidWait(FALSE); ; 
                }
                break;          
          case KEY_DOWN_yuancheng:  
                if(IED.SlideTimeTopToMid>IED.SlideTimeOutTimer) 
                //下滑中停位置上 
                { 
                    MovMotor(POS_MIDDLE);//启动半降信号
                     SetMidWait(TRUE); ; 
                }
                else
                { 
                    MovMotor(POS_BOT);//启动全降信号
                    SetMidWait(FALSE); ; 
                }
                break;           								
        }
        if((bMidWait)&&(IED.cPos==POS_MIDDLE))
        {
            SetMidWait(FALSE); ; 
            bMiddleWaitTimerID    =      StartTimer(TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorDown,0);
        }
}

void ACManualCtrl(void)
{
        {
            #ifdef _DEBUG
            //printf("_手动模式 \n") ;
            #endif
            switch(gcKey&KEY_HANDCTRL)     //(KEY_UP|KEY_DOWN|KEY_STOP)
            {
                case KEY_UP:
                    if(IED.cMoving==1)
                    {
                        if(IED.Direction!=MOTOR_UP)
                        {
                            StopMotor();
													   run_vol_set();
                            StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorUp,0);
                        }
                    } 
                    else
                    {
                        run_vol_set();
                        StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorUp,0);
                    } 
                    break;
							case KEY_UP_yuancheng:
                    if(IED.cMoving==1)
                    {
                        if(IED.Direction!=MOTOR_UP)
                        {
                            StopMotor();
													 run_vol_set();
                            StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorUp,0);
                        }
                    } 
                    else
                    {
                      run_vol_set();
                      StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorUp,0);
                    } 
                    break;
                case KEY_DOWN: 
                    if(IED.cMoving==1)
                    {
                        if(IED.Direction!=MOTOR_DOWN)
                        {
                            StopMotor(); 
                             run_vol_set();													
                            StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorDown,0); 
                        }
                    }
                    else
                    {  
                        if((TimeOut.EnStop2!=1)||(IED.cPos==POS_MIDDLE2)) 
												{
                            run_vol_set();													
                            StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorDown,0); 
												}
                        else 
												{
													 run_vol_set();													
                            StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorDown,0); 
												}													
                          //  MovMotor(POS_MIDDLE2);
                    }  
                    break;
							case KEY_DOWN_yuancheng: 
                    if(IED.cMoving==1)
                    {
                        if(IED.Direction!=MOTOR_DOWN)
                        {
                            StopMotor();  
													run_vol_set();
                            StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorDown,0); 
                        }
                    }
                    else
                    {  
                        if((TimeOut.EnStop2!=1)||(IED.cPos==POS_MIDDLE2)) 
												{
													run_vol_set();
                            StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorDown,0); 
												}
                        else                        
												{
													run_vol_set();
                            StartTimer(TIMER_MODE_ONCEROUTINE,100L,MotorDown,0); 
												}
                    }  
                    break;
                case KEY_STOP:
                    StopMotor();
								case KEY_STOP_yuancheng:
                    StopMotor();
                    break;        
                    
            }
        } 
}

void MuteWarning(void)
{ 
    IED.WarnMute = IED.u_WarnState.WarnState;
}

void MuteRun(void)
{
    IED.RunMute = 1;
}

void SaveLifeCtrl(void)
{
    if(gcKey==KEY_MUTE)
    {
        MuteError();
        MuteWarning();
        MuteRun();
    }
    if(IED.curWarnState==DCWarn)
    {
       // DCSaveLifeCtrl();
			ACSaveLifeCtrl();
    }
    else
    {
        ACSaveLifeCtrl();
    }
}

unsigned char anjian_flag;
void DCSaveLifeCtrl(void)
{
//    if(((gcKey&(KEY_UP|KEY_DOWN|KEY_STOP))!=0)
//        &&(gcKey&KEY_STOPWARN)!=KEY_STOPWARN)
	if(((CHILD[0].Key)== KEY_UP)||((CHILD[0].Key)== KEY_STOP)||((CHILD[0].Key)== KEY_DOWN))
    { 
			if(anjian_flag==0)
			{
				anjian_flag=1;
        StopMotor();
        if(IED.MotorMoveDelayTimerID !=-1)
        {
            StopTimer(&IED.MotorMoveDelayTimerID );
        }
        SetIED_DebugState(WarnMoveMid);   
        StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorMid,0);        
        /*
        if(bMidWait)
        {
            IED.MotorMoveDelayTimerID = StartTimer(TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorMid,0); 
        }
        else
        {
            IED.MotorMoveDelayTimerID = StartTimer(TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorDown,0); 
        }
        */
			}
    }
     
}

void ACSaveLifeCtrl(void)
{
//    if(
//        ((gcKey&(KEY_UP|KEY_DOWN|KEY_STOP))!=0)
//       // &&((gcKey&KEY_STOPWARN)!=KEY_STOPWARN)
//    )
	if(((CHILD[0].Key)== KEY_UP)||((CHILD[0].Key)== KEY_STOP)||((CHILD[0].Key)== KEY_DOWN))
    { 
			if(anjian_flag==0)
			{
				anjian_flag=1;
        StopMotor();   
        #ifdef _DEBUG
        printf("%s\n",IED.TimerS>IED.TimeTopToMid?"中停以下":"中停及以上");
        #endif
 //       switch(gcKey)
 //       {
//            case KEY_UP:
//                #ifdef _DEBUG
//                    printf("向上按键\n");
//                #endif
//                if((IED.TimerS<IED.TimeTopToMid)    //中停以上
//                ||(IED.cPos==POS_MIDDLE))      
//                {    //中停        
//                    if(IED.PreMotor_Cmd!=POS_TOP)
//                    {
//                        StopMotor();   
//                        StartTimer2(&IED.MotorMoveDelayTimerID ,TIMER_MODE_ONCEROUTINE,100L,MotorUp,0);                         
//                        IED.IED_DebugState = WarnMidWait;  
//                    }
//                }      
//                else  
//                {     
//                    if(IED.PreMotor_Cmd!=POS_MIDDLE)
//                    {
//                        StopMotor();       
//                        StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0);                                                                     
//                        IED.IED_DebugState = WarnMidWait; 
//                    }
//                }
//                break;
//            case KEY_DOWN:
//            case KEY_STOP:    
 if(((CtrlHalfDown==0)||(IED.b_Halfdown==1))&&
        (IED.b_SmokeSensor==0)&&
        (CtrlAllDown==1)&&(IED.b_Alldown==0)&&(IED.b_TempSensor==0))      //只有半降信号
        {  
           //      halfdown_life=1;					
                if((IED.TimerS<IED.TimeTopToMid)      
                ||(IED.cPos==POS_MIDDLE))       
                {   
                    StopMotor();  
									 StopTimer(&IED.MotorMoveDelayTimerID); 
          //          StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,3000L,MotorMid,0); 
                 IED.IED_DebugState =WarnMoveMid;      
                }
                else //if(IED.TimerS>IED.TimeTopToMid)                 
                //中停以下
                {     
                   
                      StopMotor(); 
									   run_vol_set();
                        StartTimer2(&IED.MotorMoveDelayTimerID ,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0); 
                        IED.IED_DebugState = WarnMidWait;         
                
                }
					}
                     
							
				else if((IED.b_TempSensor==1)||(CtrlAllDown==0)||(IED.b_Alldown==1)
                ||(IED.b_SmokeSensor==1))                           //有全降信号
            {
				        if((IED.TimerS<IED.TimeTopToMid)      
                ||(IED.cPos==POS_MIDDLE))       
                {   
                    StopMotor();  
									run_vol_set();
                    StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,IED.MidTimeWait,MotorDown,0); 
                    IED.IED_DebugState = WarnMoveBot;      
                }
                else //if(IED.TimerS>IED.TimeTopToMid)                 
                //中停以下
                {     
                    if(IED.PreMotor_Cmd!=POS_MIDDLE)
                    {     
                        StopMotor(); 
											run_vol_set();
                        StartTimer2(&IED.MotorMoveDelayTimerID ,TIMER_MODE_ONCEROUTINE,100L,MotorMid,0); 
                        IED.IED_DebugState = WarnMidWait;   
                    }
                }
			        } 
						}
					}			
}
