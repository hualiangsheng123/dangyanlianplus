#include "DC2AC.H"
#define PULSE_PRIOD 800UL //20KHz����Ƶ��
 extern unsigned char MOTOR__UP__FLAG;
 extern unsigned char MOTOR__DOWN__FLAG;
double voltage[7];  //��ѹ


//ADC0  24V��ѹ -----��ص�ѹ
//ADC1  ��ص�ѹ-----ɲ����� 
//ADC2  �¸�    ---- ANJIAN
//ADC3  �̸�    -----������
//ADC4  ������ ----ANJIAN_YUANCHENG
//ADC5  ɲ�����-----�¸�
//ADC6  �»��ٶȼ��--24V��ѹ
//ADC7  �»��ٶȼ��---�̸�

//AD_Value[0]:ͨ��0����ص�ѹ���ͨ��
//AD_Value[1]:ͨ��1���ػ����ͨ��
//AD_Value[2]:ͨ��2���������ͨ��
//AD_Value[3]:ͨ��4��Զ�̿��Ƽ�ͨ
//AD_Value[4]:ͨ��5�����¸м��ͨ��
//AD_Value[5]:ͨ��6��24v���ͨ��
//AD_Value[6]:ͨ��7���̸м��ͨ��

#define BAT_POWER            voltage[0]
#define OFF_JIANCE              voltage[1] 
//#define VOLPOT               voltage[2]
//#define MAINPOWER            voltage[3]
#define SENSOR_HEAT          voltage[4]
#define VOLTAGE24            voltage[5]//  
#define SENSOR_SMOKE         voltage[6]

  
//24V ���
void InitInvert(void *p)
{  
    INVERT__SET;  
    #ifdef _DEBUG
    printf("����24V��ѹ\n  " );
    #endif  
}

void StopInvert(void)
{
    INVERT__CLR;       
    #ifdef _DEBUG
    printf("ֹͣ24V��ѹ\n  " );
    #endif  
}

 


void InitDC2AC(void* p)
{  
    //���ö�ʱ��
    StartPulse(0);  //����ͨ��
}
 
#define SHUTDOWN PD3_O
//220V���
void StartPulse(void* p)
{
    IED.MotorMoveDelayTimerID=-1; 
   // SHUTDOWN__CLR;
}

void DeInitDC2AC(void *p)
{     
   // SHUTDOWN__SET; 
}
 
void StopDC2ACConvert(void* p)
{ 
    IED.cMoving = 0;  
    #ifdef _DEBUG
    printf("ֱֹͣ�����\n  " );
    #endif  
    StopTimer(&IED.MotorMoveDelayTimerID );
    DeInitDC2AC(0);  
}   
 

unsigned char BRAKECHECK;
void EnableBrakPower (UCHAR bEnable)
{
    
    if(IED.BrakeDelayTimerID!=-1)
        StopTimer(&IED.BrakeDelayTimerID);
    if(bEnable)
    {
        BRAKECHECK=1;      //���Ҫ��ɲ��,������  
		//	  BRAKECHECK__SET;
        SuspendTask(TASK_CHECK_COIL);     
        SuspendTask(TASK_CHECK_MOTOR); 
        //EndMotorInterrupt(); 
        
        #ifdef _DEBUG
        printf("ֹͣɲ�����\n ");
        #endif             
    }
    else               //������ɲ��,��ʱ�ſ�,��ֹ�����Ƹ���
    { 
        if(KEY_MODE!=USER_MODE)        //�û�ģʽ���ü��ɲ��
             StartTimer2( &IED.BrakeDelayTimerID ,
                    TIMER_MODE_ONCEROUTINE,400UL,StartCheckBrake,0);//400
    }
    
} 

void StartCheckBrake(void *p)
{ 
    IED.BrakeDelayTimerID = -1;
    BRAKECHECK = 0;
	//  BRAKECHECK__CLR;
    if(KEY_MODE!=USER_MODE)  
    {
        ActiveTask(TASK_CHECK_COIL,50L);     
        //InitMotorInterrupt();        
        ActiveTask(TASK_CHECK_MOTOR,1000L); 
        MotorLineCnt = 0;
        //SetTaskLastTime(TASK_CHECK_MOTOR,5000L);         //�ӳ�5S���
    }
    #ifdef _DEBUG
        printf("����ɲ�����\n ");
    #endif             
    //�����������
    //BRAKECHECK 
}

unsigned char BAT_CTRL_RELAY;
void EnableBattary(void)
{
    if(BAT_CTRL_RELAY==0)
    {
        #ifdef _DEBUG
        printf("���ӵ��\n ");
        #endif             
        BAT_CTRL_RELAY = 1;
			  BAT_CTRL_RELAY__SET;
    }
}


void DisableBattary2(void* p )
{
    DisableBattary();
}
void DisableBattary(void)
{   
    if(BAT_CTRL_RELAY!=0)
    {
        #ifdef _DEBUG
        printf("�Ͽ����\n ");
        #endif    
        BAT_CTRL_RELAY = 0;
			  BAT_CTRL_RELAY__CLR ;
    } 
}


signed char BotCheckTimerID = -1;            
void BotCheck(void *p)
{
    
    if(pos_bot_bit==0)
    {
        StopMotor();
        StopTimer(&BotCheckTimerID); 
        
        #ifdef _DEBUG
        printf("�����»�����,ֹͣ���");
        #endif              
        IED.TimerS  = IED.TimeTopToBot; 
        IED.cPos = POS_BOT ;
    }
    
}

signed char TimerDeInitDC2AC    = -1;
uint16_t dc_run_time;
//void Task_DC_Brake_down()       //TASK_BREAK
//{ 
//    int Percent = 0;
//    Percent = VOLPOT/40;    
//    #ifdef _DEBUG
////    printf("VOLPOT=%d\n",(int)VOLPOT); 
////    printf("Percent = %d\n",Percent); 
//    #endif               
//    //������� ������� ���� ����������� ���Ͽ�������п���
//    //����ָ� �������      ֹͣ������� ���ӵ�����п��� 
//    if(((IED.Err&(MAIN_POWER_ERROR|ErrPhase|ERR_MOTOR_LINE))!=0)&& //����������� //�������
//    ((IED.Err&(BAT_LOW_ERR|BAT_OVER_ERROR|BAT_SHORT|COIL_SHORT_ERR|COIL_BREAK_ERR))==0)    
//    //�ޱ������     //|
//    )
//    {  
//        #ifdef _DEBUG
//        //printf("���з���%s\n",IED.Direction==MOTOR_DOWN?"����":"����");     

//        //printf("IED.SlideTimeTopToMid=%ld\n",IED.SlideTimeTopToMid);
//        #endif
//        //if((IED.Direction==MOTOR_DOWN)&&(IED.PreMotor_Cmd!=POS_NONE))                     //���¿������      
//        if(((IED.PreMotor_Cmd==POS_BOT)||(IED.PreMotor_Cmd==POS_MIDDLE))&&((IED.Err&BAT_LOW_ERR)==0))
//        {  
//            if(pos_bot_bit!=0)
//            {   
//                EnableBrakPower(TRUE);  
//                #ifdef _DEBUG
//                printf("IED.SlideTimeOutTimer=%ld\n",IED.SlideTimeOutTimer);                
//                printf("IED.PreMotor_Cmd=%s\n",IED.PreMotor_Cmd==POS_MIDDLE?"��λ":
//                                                                        IED.PreMotor_Cmd==POS_BOT?"��λ": 
//                                                                        IED.PreMotor_Cmd==POS_TOP?"��λ":
//                                                                        IED.PreMotor_Cmd==POS_NONE?"NONE":
//                                                                        "����");
//                #endif
//                if(dc_run_time<=40)
//								{
//                StartDC2ACConvert();
//								}
//								else
//                {
//								//	 if(bMiddleWaitTimerID!=-1)
//                //    StopTimer(&bMiddleWaitTimerID);
//								dc_run_time=0;
//                StopMotor(); 
//									return;
//								}
//                if(IED.SlideTimeOutTimer !=0)
//                {
//                    SetTaskLastTime(TASK_BREAK,DC_DN_PR-1000+Percent*31);  //��������+ֹͣ��ʱ��
//                      
//                    if(TimerDeInitDC2AC!=-1)
//                    {
//                        StopTimer(&TimerDeInitDC2AC);
//                    }
//                    StartTimer2(&TimerDeInitDC2AC,TIMER_MODE_ONCEROUTINE,(DC_DN_PR+Percent*3-2000)/10,DeInitDC2AC,0) ;
//                }
//                else
//                { 
//                    SetTaskLastTime(TASK_BREAK,DC_DN_PR+Percent*31);  //��������+ֹͣ��ʱ��
//                    
//                    if(TimerDeInitDC2AC!=-1)
//                    {
//                        StopTimer(&TimerDeInitDC2AC);
//                    }
//                    StartTimer2(&TimerDeInitDC2AC,TIMER_MODE_ONCEROUTINE,(DC_DN_PR+Percent*3-2000)/10,DeInitDC2AC,0) ;
//                }
//                //�»�����ͣλ��                                    //Ŀ��λ������ͣ                
//                if((IED.SlideTimeTopToMid<=IED.SlideTimeOutTimer)&&(IED.PreMotor_Cmd==POS_MIDDLE))
//                {
//                    StopMotor();
//                    IED.cPos = POS_MIDDLE ;
//                    IED.TimerS  = IED.TimeTopToMid; 
//                    #ifdef _DEBUG
//                    printf("��ͣ\n");
//                    #endif
//                }
//                else
//                {    
//                    if(!IED.b_warning)
//										{
//                        send_data(0x0e); 
//										}
//                    IED.cPos = POS_UNKNOWN ; 
//                    
//                   if(IED.TimerS!=0)
//                    {
//                        IED.SlideTimeOutTimer = IED.TimerS/(IED.TimeTopToMid/IED.SlideTimeTopToMid+1);
//                       IED.TimerS = 0;
//                   }
//                    //
//                    //������������е�λ��,����ɻ��е�λ�� 
//                    if(BotCheckTimerID==-1)
//                         StartTimer2(&BotCheckTimerID,TIMER_MODE_CYCROUTINE,5,BotCheck,0) ;  
//                } 
//                 
//                IED.SlideTimeOutTimer ++;   
//                dc_run_time++;
//            }
//            else
//            { 
//                BotCheck(0);
//            }      
//        } 
//        else                   
//        {   
//            //SetTaskLastTime(TASK_BREAK,100);
//        }        
//    }
//                                                   
//    //����ʱ��ʱ�ر� 
//    //����ָ����ߵ���˲��ᱨȱ�����ֹͣ���
//    //�ָ�����ʱ��������ж�ʧ��                   
//}

char DC2AC_CTRL = FALSE;
void StartDC2ACConvert(void)
{
    IED.cMoving = 1;
    if(DC2AC_CTRL==FALSE)
    {
         DC2AC_CTRL    = TRUE;  
        #ifdef _DEBUG
        printf("����ֱ�����\n " );
        #endif      
    }
    StopTimer(&IED.MotorMoveDelayTimerID);
    StartTimer2(&IED.MotorMoveDelayTimerID,TIMER_MODE_ONCEROUTINE,10,StartPulse,0);      //��ʱ0.1S���
    
    IED.OverRollTime = IED.TimeTopToBot+100; 
}

void AddCommand(unsigned char cMotorCmd)
{
    if(IED.Motor_Cmd != cMotorCmd)
    {
        #ifdef _DEBUG
        switch(cMotorCmd)
        {
        case POS_TOP:
            printf("Add command POS_TOP\n ") ;
            break;
        case POS_MIDDLE:
            printf("Add command POS_MIDDLE\n ") ;
            break;
        case POS_BOT:
            printf("Add command POS_BOT\n ") ;
            break;
        case POS_UNKNOWN:
            printf("Add command POS_UNKNOWN\n ") ;
            break;
        case POS_NONE :
            printf("Add command POS_NONE\n ") ;
            break;
        }
        #endif
        IED.Motor_Cmd = cMotorCmd;
        
        SetTaskLastTime(TASK_BREAK,100);  //�趨����ִ�м��ʱ��
    }
}

void ActiveTaskCheckPhase(void)
{
    #ifndef _SINGLE
    ActiveTask(TASK_PHASE,PHASE_PRIOD);  
    #endif
}


#define HOUR1 (60L*60L*100L)  
signed char TimDelayShutDown = -1;            
void DelayShutDown(void)
{
    if(TimDelayShutDown==-1)
    {
        StartTimer2(&TimDelayShutDown,TIMER_MODE_ONCEROUTINE,HOUR1,DisableBattary2,0) ;  
    }
}

#define MAINPOWER_VOL  7000UL      //����

#define BATLOW_VOL     9000UL   //Ƿѹ
#define BAT_TH_VOL     12000UL 
#define BATSHORT_VOL    2000UL   //��·
#define BATFULL_VOL    12200UL  //����
#define BATOVER_VOL    12600UL  //��ѹ
extern unsigned char MOTOR__RUN ;
void Task_Power_Check()         //��ѹ�����źż��
{                                     
    if(shi_dian_in==0)       // �����ѹ������ʾ
    {    
        IED.Main_Power_OK_Cnt++  ;   //������ѹ�ò��ж�����
        if(IED.Main_Power_OK_Cnt>1)
        {                 
            IED.Err &= ~MAIN_POWER_ERROR; 
					  #ifdef _SINGLE
					  IED.Err &= (~ErrPhase); 
					 #endif
      //      StopTimer(&TimDelayShutDown);

         //   ActiveTaskCheckPhase();
            IED.Main_Power_OK_Cnt = 2;
        }                              
    }
    else                          //�����ѹ��������ʾ
    {    
      //  DeChargeBat(FALSE);       //ֹͣ��طŵ�
        IED.Main_Power_OK_Cnt = 0;  
//        #ifdef _MCU_301
//        if(MOTOR__RUN==1)   
//        #elif _MCU_302 
//        if(MOTOR__DOWN__FLAG||MOTOR__UP__FLAG)    
//        #endif
       //     ErrorStopMotor();       
        IED.Err |= MAIN_POWER_ERROR; 
//        IED.Err |= ErrPhase;
//        SuspendTask(TASK_PHASE);                        //�������ֹͣ��λ���,��ֹ������
    //    DelayShutDown();
    } 
}





void ChargeBat(char bEnable)
{        
//    if(bEnable)                //���ʱ���÷ŵ�
//        DeChargeBat(FALSE);    //�����ʱ��һ���ŵ�
//    CHARGE_BAT__SET;
}

void DeChargeBat(char bEnable)
{
//    FLUSH_BAT = bEnable;
}


void BatCheck(void)
{
    #ifdef _DEBUG
//    if((IED.Err!=0x05)&&(IED.Err!=0x00))
//        printf("IED.Err=0x%04X\n",IED.Err);
    #endif
    if(BAT_POWER <BATLOW_VOL)                           //�����ѹ��
    {    
        if(BAT_POWER <BATSHORT_VOL)
        { 
            IED.Err |= BAT_SHORT; 
        }
        else
        {
            if(shi_dian_in==0 )
            {
                IED.Bat_Low_Cnt =0 ;         
                IED.Err &= ~BAT_LOW_ERR;
            }
            else
            {     
                IED.Err |= BAT_LOW_ERR;                                                //9.0--13.9V
                IED.Bat_Low_Cnt ++ ;
                if(IED.Bat_Low_Cnt>=80)        //����ѹ��80�κ�ػ� 8S
                {         
                    DisableBattary();              //��ѹ��������ػ�
                    StopDC2ACConvert(0);
                } 
            }  
        }        
    }
    else//����ʧ��,���Լ���ѹ���Ƿ�ָ�
    { 
        IED.Bat_Low_Cnt = 0; 
        IED.Err &= ~BAT_LOW_ERR;      
        IED.Err &= ~BAT_SHORT;
        
    }
    if(BAT_POWER>BATOVER_VOL)
    {           
        IED.Err |= BAT_OVER_ERROR;
    }
    else 
    {  
        IED.Err &= ~BAT_OVER_ERROR;
    }
     
}


enum  CHAREGE_STATE{CHARGE_INIT=0,PRE_ENCHARGE,ENCHARGE1,
CHARGE_WAIT1,DECHARGE,CHARGE_WAIT2,CHECKVOL,CHARGEEND};
//
enum CHAREGE_STATE charegeState = CHARGE_INIT;
int off_Cnt = 0;
void TaskChargeBat(void)
{
    GetVoltage();
	if(IED.cMoving==0)
	{
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	}
	else
	{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);	
	}
	if(shi_dian_in==1)
	{
		if(AD_Value[1]>1700)
		{
			off_Cnt++;
			if(off_Cnt>=50)
			{
				jiance_vol_set();
				CPU_POWER__OFF ;
			}
		}
	}
  //  BatCheck();
}



//AD_Value[0]:ͨ��0����ص�ѹ���ͨ��
//AD_Value[1]:ͨ��1���ػ����ͨ��
//AD_Value[2]:ͨ��2���������ͨ��
//AD_Value[3]:ͨ��3��������߼��ͨ��
//AD_Value[4]:ͨ��4��Զ�̿��Ƽ�ͨ��
//AD_Value[5]:ͨ��5���¸м��ͨ��
//AD_Value[6]:ͨ��6��24v���ͨ��
//AD_Value[7]:ͨ��7���̸м��ͨ��
extern uint8_t ubDmaTransferStatus ;
extern unsigned int power_on;
#define VREF 3280UL
void GetVoltage(void)
{  
    double kGain[7] = {6.0f,3.35f,0,1.0f,11.0f,11.0f,11.0f};
		if(ubDmaTransferStatus==1)
		{
    int i =0;
		ubDmaTransferStatus=0;
    for(i=0;i<M_CHANNEL;i++)
    {
        voltage[i] = kGain[i]*VREF*AD_Value[i]/0x0FFF;  
    } 
		if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
      Error_Handler(); 
    }
        /*
    voltage[0]  =  11.0f*voltage[0];      
    voltage[1]  =  6.0f*voltage[1];        //��ص�ѹ
    voltage[2]  =  11.0f*voltage[2];
    voltage[3]  =  11.0f*voltage[3];
    voltage[4]  =  8.7f*voltage[4];        //��Դ��ѹ
    voltage[5]  =  3.35f*voltage[5];        // ��Ȧ 
    */
    #ifdef _DEBUG   
    //printf("VOLPOT=%d\n",(int)VOLPOT);  
    #endif    
    }
}



void TaskCommand(void)
{
    unsigned char Motor_Cmd ;
    if(IED.Motor_Cmd!=POS_NONE)     
    {             //ȱ��//����//����Ͽ�    
        if ((IED.Err&(ErrPhase|ErrOverHeat|MAIN_POWER_ERROR|ERR_MOTOR_LINE))==0)//��Դ��������
        {
            SuspendTask(TASK_BREAK);
            #ifdef _DEBUG
            //printf("��Դ��������\\n ") ;
            #endif        
            if((IED.MotorMoveDelayTimerID==-1)&&(IED.cMoving))
            {
                Motor_Cmd = IED.Motor_Cmd;
                IED.cMoving= 0;
                StopMotor();               //ֹͣһ�����¼���λ��
                IED.Motor_Cmd = Motor_Cmd;
                StartTimer2(&IED.MotorMoveDelayTimerID ,TIMER_MODE_ONCEROUTINE,200,ResumeMotorRun,0);     //�ȴ������ж��ȶ�         
             }
            
        }
        else
        {
            ActiveTaskNow(TASK_BREAK,DC_DN_PR);
            #ifdef _DEBUG
            //printf("��Դ���� �޷��ָ��������\n ") ;
            #endif
        }
    }
}

void ResumeMotorRun(void *p)              
{             
    MovMotor(IED.Motor_Cmd)     ;
    IED.MotorMoveDelayTimerID = -1; 
    #ifdef _DEBUG
    printf("�ָ��������  %d\n ",(int)IED.Motor_Cmd) ;
        #endif
    IED.Motor_Cmd =  POS_NONE ;
}


//#define VOLTAGESHORTVOL  4200.0f //4.2
#define VOLTAGESHORTVOL  800.0f //4.2
#define VOLTAGEBreakVOL  5000.0f          
//7500 ̫��,��ص�ѹ�½�̫�������������

void TaskCheckBrakeCoil(void)
{     
    #ifndef _SINGLE
    if(BRAKECHECK==0)
    { 
        if(COILVOL>VOLTAGEBreakVOL)     //��Ȧ�Ͽ�
        {
            IED.Err|=COIL_BREAK_ERR ;
        }
        else
        {
            IED.Err&=(~COIL_BREAK_ERR); 
    
        }
        ////////////////////////////////
        if(COILVOL<VOLTAGESHORTVOL )    //��Ȧ��·
        {
            IED.Err|=COIL_SHORT_ERR;
        }        
        else
        {
            IED.Err&=(~COIL_SHORT_ERR); 
        }
    }                 
    #endif
}
 
 

#define SENSOR_DETECT_VOL 14000.0f  
unsigned char TempSensorCnt,SmokeSensorcnt;   
void TaskCheckSensor(void)
{ 
    int ErrTempSensor,ErrSmokeSensor;
    //IED.b_TempSensor = 0;
    ErrTempSensor = fabs(SENSOR_HEAT-((double)VOLTAGE24)/21.0f);
    ErrSmokeSensor= fabs(SENSOR_SMOKE-((double)VOLTAGE24)/21.0f);
   
    if((VOLTAGE24-SENSOR_HEAT)<2000) //���С��200mV //��·
    {
       // IED.Err |= ERR_SENSOR_TEMP;//ERR_SENSOR_SMOKE=0x100,ERR_SENSOR_TEMP=0x200,
			if(VOLTAGE24>20000)
			{
			 IED.Err &= (~ERR_SENSOR_TEMP);     
        if(TempSensorCnt++>3)
            IED.b_TempSensor = 1;
			}
    } 
    else if((SENSOR_HEAT)<100.0f)            //��·����
    {
        IED.Err |= ERR_SENSOR_TEMP;
        _nop_();
    }
    else if(ErrTempSensor<1000.0f)     //Ϊ�� ѹ��,����
    {   
			  if(CONT_BRK_Select==0)
				{
					IED.b_TempSensor = 0;
				}
        IED.Err &= (~ERR_SENSOR_TEMP);
        TempSensorCnt = 0;
    }             
    else                   
        if(fabs(SENSOR_HEAT-VOLTAGE24)<SENSOR_DETECT_VOL)     // �ȵ�Դ��ѹ��3V����,Ϊ�𾯶���
    {
			if(VOLTAGE24>20000)
			{
        IED.Err &= (~ERR_SENSOR_TEMP);     
        if(TempSensorCnt++>3)
            IED.b_TempSensor = 1;
			}
    } 
    ////////////////////////////////////////
    
    //IED.b_SmokeSensor = 0;
    if((VOLTAGE24-SENSOR_SMOKE)<2000.0f) //���С��200mV // ��· �����ֵ��С��ʱ���·�ᱨ�� 
    {
        //IED.Err |= ERR_SENSOR_SMOKE;//ERR_SENSOR_SMOKE=0x100,ERR_SENSOR_TEMP=0x200,
			if(VOLTAGE24>20000)
			{
			 IED.Err &= (~ERR_SENSOR_SMOKE);       
        if(SmokeSensorcnt++>3)    
            IED.b_SmokeSensor = 1;
			}
    }
    else if((SENSOR_SMOKE)<100.0f)            //��·����
    {
        IED.Err |= ERR_SENSOR_SMOKE;
    }
    else if(ErrSmokeSensor<1000.0f)     //Ϊ�� ѹ��,����  500
    {
			 if(CONT_BRK_Select==0)
				{
					 IED.b_SmokeSensor = 0;
				}
        IED.Err &= (~ERR_SENSOR_SMOKE);
        SmokeSensorcnt = 0;
    }
    else        
        if(fabs(SENSOR_SMOKE-VOLTAGE24)<SENSOR_DETECT_VOL)     //�ȵ�Դ��ѹ��3V����,Ϊ�𾯶���
    {
			if(VOLTAGE24>20000)
			{
        IED.Err &= (~ERR_SENSOR_SMOKE);       
        if(SmokeSensorcnt++>3)    
            IED.b_SmokeSensor = 1;
			}
    }                                        
}

 


 
