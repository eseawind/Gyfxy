#include "ctrl.h"
#include "test.h"
#include "task.h"
#include "string.h"
#include "stm32f10x_tim.h"
#include "math.h"
#include "KEY.H"
#include "modbus.h"
#include "EEPROM.h"
#include "string.h"
IED_DATABASE  IedDb __at (0x20001000);
void InitIED(void)
{

	memset(&IedDb,0,sizeof(IedDb));
//	strcpy(IedDb.DownloadWorkParam.reserve,"Attrib Block\n");
	strcpy(IedDb.InstrumentName,"TF_GYFXY2012");
	IedDb.cID = 21;

	IED_DATABASE::YP_DATA volatile *  Sample = IedDb.Sample1;
	memset((void*)Sample,0,sizeof(IedDb.Sample1)/sizeof(char));
    
    IedDb.DownloadWorkParam.m_btMaxSampleCnt        = 24;				//���λ	 
	IedDb.DownloadWorkParam.m_sPulsePerRound1Spit   = 200;;				//ÿȦ1ϸ��������Ҫ������
// 	IedDb.DownloadWorkParam.m_btDriverSpitCnt       = 16;;
// 	for(int i=0;i<sizeof(IedDb.Sample1)/sizeof(IED_DATABASE::YP_DATA);i++)
// 	{
// 		Sample[i].cSta=S_WAIT;	
// 	} 
}
//					  
 				  
class Timer4{
    public:
        static Timer4 *getTimer()
        {
            if (timer == NULL) {	  
                timer  = new Timer4();
				
            }
            return timer;
        }
	
    private: 
        Timer4(){InitTimer();};
        Timer4(const Timer4 &s){};
        void operator=(const Timer4 &rhs){};
		void InitTimer()
		{
			GPIO_InitTypeDef GPIO_InitStructure;
			TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_9;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����ӵأ��������ĸߵ�ƽ
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50Mʱ���ٶ�
			GPIO_Init(GPIOB, &GPIO_InitStructure);
		
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_7|GPIO_Pin_8;		  //����/�ѻ�IO��
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�����ӵأ��������ĸߵ�ƽ
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50Mʱ���ٶ�
			GPIO_Init(GPIOB, &GPIO_InitStructure);
													  
		    TIM_DeInit(TIM4);
		
		  	TIM_TimeBaseStructure.TIM_Period 		= 0xFFFF;//65535;
		  	TIM_TimeBaseStructure.TIM_Prescaler 	= 17;
		  	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		  	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;
		  	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
		
		  	/* TIM enable counter */
		  	TIM_Cmd(TIM4, ENABLE);
		}
		private:
        	static Timer4  *timer;	 //TIM_TypeDef	
		public:
			StepMotorCtrl::StepMotorState  *pMtrState[5];			  
	 
};
							    
 
Timer4 *Timer4::timer  = NULL;	    							    
////////////////////////////////////////////////////////////////////
Beep beep;
void TaskBeep(void)
{
	beep.TaskBeep();
}	

////////////////////////////////////////////////////////////////////
#if !defined ( __CC_ARM   )
VertStoveBar VertStove[3] =

{
	{&PD3_I,&PD2_I,&PC4_O,&PC5_O,500,1000},
	{&PD5_I,&PD4_I,&PC0_O,&PC1_O,500,1000},
	{&PD7_I,&PD6_I,&PC2_O,&PC3_O,500,1000} 
};
#else 
LowVertStoveBar  m_LowTempStoveBar[1];
HighVertStoveBar m_HighTempStoveBar[2]; 

void InitStove(void)
{																	   
	IED_DATABASE::DownParam *param = &IedDb.DownloadWorkParam;														
	LowTempStove.InitStoveBar(&PD12_I,&PD13_I,&PC0_O,&PC1_O,&param->m_sTimePut2weigh);
	LeftHighTempStove.InitStoveBar(&PD10_I,&PD11_I,&PC2_O,&PC3_O,&param->m_sTimePut2HighTemp);
	RightHighTempStove.InitStoveBar(&PD8_I,&PD9_I,&PC4_O,&PC5_O,&param->m_sTimePut2HighTemp);
    
	LowTempStove.InitStoveBar(&PD12_I,&PD13_I,&PC0_O,&PC1_O,&param->m_sTimePut2weigh);
	LeftHighTempStove.InitStoveBar(&PD10_I,&PD11_I,&PC2_O,&PC3_O,&param->m_sTimePut2HighTemp);
	RightHighTempStove.InitStoveBar(&PD8_I,&PD9_I,&PC4_O,&PC5_O,&param->m_sTimePut2HighTemp);
      
}
#endif




void TaskStopBar(void)
{												 
	for(unsigned int i=0;i<sizeof(m_LowTempStoveBar)/sizeof(LowVertStoveBar);i++)
	{
		m_LowTempStoveBar[i].TaskStopBar();
	}
    for(unsigned int i=0;i<sizeof(m_HighTempStoveBar)/sizeof(HighVertStoveBar);i++)
	{
		m_HighTempStoveBar[i].TaskStopBar();
	}
}
///////////////////////////////////////////////////////////////////////////////
StepMotorCtrl StepMotor[2];	 
 
StepMotorCtrl::StepMotorCtrl()
{
    /*
	AllSampleCnt 			= MAX_SAMPLE_CNT ;				//���λ	 
	PulsePerRoundSubDiv		= DEFAULT_PULSECNT_PERROUND;	//ÿȦ1ϸ��������Ҫ������
	SubDiv					= DEFAULT_SUBDIV;				//ϸ����
	*/
	//pMtrState->CurSamplePos		= 0;						//��ǰ��λ	 
	AllSampleCnt 			= &IedDb.DownloadWorkParam.m_btMaxSampleCnt;				//���λ	 
	PulsePerRoundSubDiv		= &IedDb.DownloadWorkParam.m_sPulsePerRound1Spit;;				//ÿȦ1ϸ��������Ҫ������
	SubDiv					= &IedDb.DownloadWorkParam.m_btDriverSpitCnt;;				//ϸ���� 
	
}

void StepMotorCtrl::InitMotor(long AllSampleCnt,long PulsePerRoundSubDiv,long SubDiv)
{
	*this->AllSampleCnt 		= AllSampleCnt ;		//���λ	 
	*this->PulsePerRoundSubDiv	= PulsePerRoundSubDiv;	//ÿȦ1ϸ��������Ҫ������
	*this->SubDiv				= SubDiv;				//ϸ����	
	
}

void StepMotorCtrl::InitCtrl(TIM_TypeDef *TIMx ,int Channel, uint32_t* pPortFree, uint32_t* pPortDir,uint32_t* pResetIn)
{
 	pMtrState	  				= &IedDb.MtrState;
	pMtrState->bRun				= false;					//����
	pMtrState->bDir				= FORWARD;					//����
	pMtrState->DestPulseCnt		= 0x4FFFF;					//����Ŀ��λ��������
	pMtrState->CurPulseCnt		= 0x3FFFF;					//��ǰλ��������
	pMtrState->CurSamplePos		= 0;
	pMtrState->ResetIn			= pResetIn;

	this->TIMx				= TIMx;						//ʹ�õĶ�ʱ��
	this->Channel			= Channel;					//��ʱ��ͨ��
	if(TIMx==TIM4)
	{							  
		Timer4 *timer  				= Timer4::getTimer();	
		timer->pMtrState[Channel]	= pMtrState;	   
		__nop();
	}

	this->pPortFree			= pPortFree;				//��������
	*pPortFree				= UNLOCK ;					//��ʼ����δ������
	this->pPortDir			= pPortDir;					//��������
	*pPortDir				= pMtrState->bDir;
	InitTimer();
}

void InitStepMotor(void)
{				
	StepMotor[0].InitCtrl(TIM4,1,&PB7_O,&PB8_O,&PD15_I);									    
	//StepMotor[1].InitCtrl(TIM4,4,&PB1_O,&PB0_O);
}	   

int StepMotorCtrl::MoveStepMotor(long DestPulseCnt,StepMotorCtrl::DIR direction)
{ 
	if(DestPulseCnt==pMtrState->CurPulseCnt)
	{
		return 0;
	}										  
	if((LeftHighTempStove.m_StovePos!=HighVertStoveBar::POS_BOT)
		||(RightHighTempStove.m_StovePos!=HighVertStoveBar::POS_BOT))
    {
        strncpy(ErrBuf,"��¯����¯���˲��ڵ�λ",INFO_LENTH);
        ErrFlg = true;
		return -2;
    }
	if(LowTempStove.m_StovePos!=LowVertStoveBar::POS_TOP)
    {
        strncpy(ErrBuf,"���̲��ڶ�λ",INFO_LENTH);
        ErrFlg = true;
		return -3; 
    }
	pMtrState->bRun 			= true;
	*pPortFree					= LOCK ;
	*pPortDir = pMtrState->bDir = direction;    	 
	pMtrState->DestPulseCnt	= DestPulseCnt;
	SetFreq(200*(*SubDiv)/16); 	
	#ifdef _DEBUG
		printf("ת���������") ;
	#endif 	
	return 0;	
}

void StepMotorCtrl::StopMotor(void)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;							  
	TIM_OCInitStructure.TIM_OCMode			= TIM_OCMode_Timing	;	  	
	TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable; 	  
	TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_Low; 
	TIM_OCInitStructure.TIM_Pulse 			= 0;//CCR3_Val;
	pMtrState->bRun							= 0;
	switch(Channel)
	{
		case 1:					    
			TIM_OC1Init(TIMx, &TIM_OCInitStructure);
			TIM_ITConfig(TIMx,  TIM_IT_CC1, DISABLE); 
			
			break;
		case 2:					    
			TIM_OC2Init(TIMx, &TIM_OCInitStructure);
			TIM_ITConfig(TIMx,  TIM_IT_CC2, DISABLE);
			break;
		case 3:					    
			TIM_OC3Init(TIMx, &TIM_OCInitStructure);
			TIM_ITConfig(TIMx,  TIM_IT_CC3, DISABLE);
			break;
		case 4:					    
			TIM_OC4Init(TIMx, &TIM_OCInitStructure);
			TIM_ITConfig(TIMx,  TIM_IT_CC4, DISABLE);
			break;
		default:
			break;
	}
}
		    
int StepMotorCtrl::MoveStepMotor(long DestPos)
{
	int32_t step;
	StepMotorCtrl::DIR direction = FORWARD; 
	////////////////////////////////////////////////////////////////////////////////
	//��Ŀ��λ���޶���24����Ʒ֮��
    if(DestPos<0)
        DestPos		= (DestPos-1)%(*AllSampleCnt)+*AllSampleCnt+1;
	if(DestPos!=0)
		DestPos		= (DestPos-1)%(*AllSampleCnt)+1;
	pMtrState->DestSamplePos = DestPos;
    ////////////////////////////////////////////////////////////////////////////////

	int tDest = (DestPos<pMtrState->CurSamplePos)?DestPos+(*AllSampleCnt):DestPos;
	//�ж�����ת	
	direction = ((tDest - pMtrState->CurSamplePos )>=(*AllSampleCnt)/2)?BACKWARD:FORWARD;
	////////////////////////////////////////////////////////////////////////////////
	
	pMtrState->PulsePerRound 	= 2 //ÿ������Ҫ��ת����
								*(*PulsePerRoundSubDiv)*(*SubDiv);	 		
	////////////////////////////////////////////////////////////////////////////////
	if(pMtrState->CurSamplePos==0)									  //��һ����ת
	{
		pMtrState->DestPulseCnt = pMtrState->PulsePerRound;//
	} 																			  	
 	////////////////////////////////////////////////////////////////////////////////
	step =  (DestPos-1)*pMtrState->PulsePerRound/(*AllSampleCnt); 			//�ߵ�Ŀ��λ�ô�1λ���������Ŀ,(���Զ�λ)
     ////////////////////////////////////////////////////////////////////////////////
	return MoveStepMotor(step,direction);  
}

vu16 CCR1_Val = 10000;	
vu16 CCR2_Val = 10000;	
vu16 CCR3_Val = 10000;	
vu16 CCR4_Val = 10000;	
 
void StepMotorCtrl::InitTimer(void)
{	
	if(TIMx==TIM4)							  
		Timer4 *timer  = Timer4::getTimer();
}

void StepMotorCtrl:: SetFreq(uint16_t freq)
{
	uint16_t CCR_Val;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	
	RCC_ClocksTypeDef ClockFreq;
	RCC_GetClocksFreq(&ClockFreq) ;   

	ASSERT(freq<MAX_FREQ);				//�޶�������Ƶ��
	if(freq==0)
	{
	 	// Output Compare Toggle Mode configuration: Channel1 
	  	TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_Timing; 
		TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable; 
		TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_Low; 
		TIM_OCInitStructure.TIM_Pulse 			= CCR_Val;//CCR3_Val;  
		
		TIM_OC1Init(TIMx, &TIM_OCInitStructure); 
		TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Disable);
		TIM_ITConfig(TIM4,  TIM_IT_CC1, DISABLE);   
	}
	else 
	{
		CCR_Val = ClockFreq.SYSCLK_Frequency/freq/36;	 
		TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_Toggle;  
		
	}
	TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_Low; 
	TIM_OCInitStructure.TIM_Pulse 			= CCR_Val;//CCR3_Val;
	switch(Channel)
	{
		case 1:
			CCR1_Val = CCR_Val;
			if(((TIMx->CCMR1)&TIM_OCMode_Toggle)!=TIM_OCMode_Toggle)
			{
				TIM_OC1Init(TIMx, &TIM_OCInitStructure); 
				TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Disable);
				TIM_ITConfig(TIM4,  TIM_IT_CC1, ENABLE); 
			}
			break;
		case 2:
			CCR2_Val = CCR_Val;
			if(((TIMx->CCMR1>>8)&TIM_OCMode_Toggle)!=TIM_OCMode_Toggle)
			{
				TIM_OC2Init(TIMx, &TIM_OCInitStructure); 
				TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Disable);	
				TIM_ITConfig(TIM4,  TIM_IT_CC2, ENABLE);
			}
			break;
		case 3:
			CCR3_Val = CCR_Val;
			if(((TIMx->CCMR2)&TIM_OCMode_Toggle)!=TIM_OCMode_Toggle)
			{
				TIM_OC3Init(TIMx, &TIM_OCInitStructure); 
				TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Disable);
				
				TIM_ITConfig(TIM4,  TIM_IT_CC3, ENABLE);
			}
			break;
		case 4:	  
			CCR4_Val = CCR_Val;
			if(((TIMx->CCMR2>>8)&TIM_OCMode_Toggle)!=TIM_OCMode_Toggle)
			{
				TIM_OC4Init(TIMx, &TIM_OCInitStructure); 
				TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Disable);	 
				TIM_ITConfig(TIM4,  TIM_IT_CC4, ENABLE);
			}
			break;
		default:
			break;
	} 
}
	 

void TIM4_IRQHandler(void) 
{ 	
 
	TIM_OCInitTypeDef  TIM_OCInitStructure;	
	TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_Timing;  	
	TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_Low; 
																		  
	Timer4 *timer  = Timer4::getTimer();
	StepMotorCtrl::StepMotorState  **pMtrSte = timer->pMtrState;

  	if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)               	//���ָ����TIM�жϷ������
  	{  
		static uint32_t PreResetIn        = 0;
		TIM4->SR = (uint16_t)~TIM_IT_CC1;				   			//���TIMx���жϴ�����λ
 		TIM4->CCR1 	+=CCR1_Val;							  			//��Ҳ����ͬ��������

		if(pMtrSte[1]!=0)								    		//����ʼ��
		{
			if(pMtrSte[1]->bDir==StepMotorCtrl::FORWARD)
			{
				(pMtrSte[1]->CurPulseCnt)++;
			}
			else
			{
				(pMtrSte[1]->CurPulseCnt)--;
			} 
		}
		if(pMtrSte[1]->CurPulseCnt >pMtrSte[1]->PulsePerRound)
			pMtrSte[1]->CurPulseCnt %=pMtrSte[1]->PulsePerRound; 
		if(pMtrSte[1]->CurPulseCnt <0)
			pMtrSte[1]->CurPulseCnt +=pMtrSte[1]->PulsePerRound; 	  
		if(pMtrSte[1]->CurPulseCnt%(pMtrSte[1]->DestPulseCnt/pMtrSte[1]->DestSamplePos)==0)          //�����̬����λ
			pMtrSte[1]->CurSamplePos = pMtrSte[1]->CurPulseCnt/(pMtrSte[1]->DestPulseCnt/(pMtrSte[1]->DestSamplePos-1))+1;

		/////////////////////////////////////////////
		//���̾���1��λʱ�Զ���λ
		if((PreResetIn==0)&&
			(*pMtrSte[1]->ResetIn==1)&&
			(pMtrSte[1]->bDir==StepMotorCtrl::FORWARD)
		) //������ת,������,������Ŀ��λ
		{
			pMtrSte[1]->CurSamplePos 	= 1;		  
			pMtrSte[1]->CurPulseCnt		= 0;   
		}
		PreResetIn = *pMtrSte[1]->ResetIn; 
		//////////////////////////////////////
		if(pMtrSte[1]->CurPulseCnt==pMtrSte[1]->DestPulseCnt)				//�趨��λ�жϴ���;
//			||((pMtrSte[1]->DestSamplePos==1)&&(*pMtrSte[1]->ResetIn==1))   //����Ŀ��Ϊ1��λʱ�ִ�1��
			
		{
			//ֹͣIO�ڷ�ת		
			TIM_OCInitStructure.TIM_Pulse 	= TIM4->CCR1;//CCR3_Val;						    
			TIM_OC1Init(TIM4, &TIM_OCInitStructure);
			TIM_ITConfig(TIM4,  TIM_IT_CC1, DISABLE);
			pMtrSte[1]->bRun	= 0;
			__nop();													 
			pMtrSte[1]->CurSamplePos 	= pMtrSte[1]->DestSamplePos;
		}
	} 
}
//////////////////////////////////////////////////////////////////////////////////////////////
void TaskBlinLed(void)
{ 
	static unsigned int count =0;
	++count;			
	count=count%10;	
	PE15_O = count>0?1:0;		  	                     
}



enum COMMAND{CMD_NAK=0,            	//������
             CMD_SETID,	            //����ID
             CMD_SETPW,	         	//��������
             CMD_LOGON,	          	//��¼
             CMD_SETDATETIME,       //����ʱ��
             CMD_READDATETIME,     	//��ȡʱ��
             CMD_RESET_INST,       	//��λ�豸 
			 CMD_MOVEMOTOR,			//�ƶ����
             CMD_MOVEMOTORSAMPLENO, //�ƶ�����
             CMD_WARMUP ,
			 CMD_TARE, 				//����
			 CMD_WEIGHT,			//������	  
			 CMD_START_TEST,		//��������
			 CMD_STOP_TEST 			//ֹͣ����
			};
void TaskParsaCmd(void)
{
	uint32_t BaseAddress;
	LONG *destTempAD;
	int NO;
	int SW;
	uint8_t nID[4];
	Metage::METAGE_TYPE 		metage_type;
	Metage::METAGE_SPEED 		metage_speed;

	

	if(IedDb.Cmd.CmdType&0x80)
		return;
    switch(IedDb.Cmd.CmdType)
    {
            case CMD_NAK:
                __NOP();
                break;              //������
            case CMD_SETID:
				IedDb.cID = IedDb.Cmd.CmdParam1;
				ReadIdFromEE();
				if(IedDb.cID!= nID[0])
				{
					nID[0] =  IedDb.cID;
					EEWriteStruct(nID,1,BaseAddress);   //char ��longд��FLASH,��Ȼ��д����,���ǲ�Ҫ��
				 }
				__NOP();    
                break;              //����ID
            case CMD_SETPW:
                __NOP();
                break;              //��������
            case CMD_LOGON:
                __NOP();
                break;              //��¼
            case CMD_SETDATETIME:
                __NOP();
                break;              //����ʱ��
            case CMD_READDATETIME:
                __NOP();
                break;              //��ȡʱ�� 
       	   case CMD_MOVEMOTOR:
		   		if(IedDb.Cmd.CmdParam2)
                {
                    if(IedDb.Cmd.CmdParam1==0)
                        LowTempStove.MoveStoveBar(IedDb.Cmd.CmdParam3);
                    else
                        m_HighTempStoveBar[IedDb.Cmd.CmdParam1].MoveStoveBar(IedDb.Cmd.CmdParam3);
                }
                else
                {
                    if(IedDb.Cmd.CmdParam1==0)
                        LowTempStove.StopStoveBar();
                    else
                        m_HighTempStoveBar[IedDb.Cmd.CmdParam1].StopStoveBar();
                }
                break;
            case CMD_MOVEMOTORSAMPLENO: //�ƶ���Ʒת��       PARAM  Ŀ����λ
                __NOP();
                //MoveSamplePos(IED.Cmd.CmdParam1);  
				if(IedDb.Cmd.CmdParam2==0)
				{
					//ֹͣ�������
					//StepMotor[0].SetFreq(150);
					StepMotor[0].StopMotor();
				}
				else if(IedDb.Cmd.CmdParam1==0)
				{
					//����ת��						   
					StepMotor[0].MoveStepMotor(0) ;
				}
				else
				{
					StepMotor[0].MoveStepMotor(IedDb.Cmd.CmdParam1);
				} 
                __NOP();
				break;
            case CMD_WARMUP:
                __NOP();							     
				   
				NO = IedDb.Cmd.CmdParam1;
				SW = IedDb.Cmd.CmdParam2; 
				destTempAD 		= &IedDb.DownloadWorkParam.m_sWaterTestTemp;	 //Ŀ���¶�AD��ַ
				destTempAD[NO]	= IedDb.Cmd.CmdParam;							 //Ŀ���¶�	
				IedDb.fuzzy[NO].CtrlStove(SW, &destTempAD[NO]);
                __NOP();
                break;              //�¶�����            param1 1 0		    
			case CMD_TARE:
				MTBalan.ZeroWeight();
				break;
			case CMD_WEIGHT:		//������
				metage_type   = (Metage::METAGE_TYPE ) IedDb.Cmd.CmdParam1;
				metage_speed  = (Metage::METAGE_SPEED )IedDb.Cmd.CmdParam2;	//����/һ��һ�� ����Ʒ
				metage.StartMetage(metage_type,metage_speed);
				__NOP();
				break;		
			case CMD_START_TEST:		//��������
				IedDb.DownloadWorkParam.m_szalgorithm = IedDb.Cmd.CmdParam1;
				test.StartTest();
				__NOP();
				break;
            case CMD_RESET_INST:	 
			case CMD_STOP_TEST: 			//ֹͣ���� 
				test.StopTest();
				__NOP();
				break;
            default:
                __NOP();
                if(IedDb.Cmd.CmdType&0x80)
                {
                    __NOP();
                    #ifdef _DEBUG
                        //printf("�����Ѿ�ִ��\n");
                    #endif   
                }
                else
                {
                    __NOP();
                    #ifdef _DEBUG
                        printf("�������ش���,����ִ��");
                    #endif
                }
                break;
    }
    IedDb.Cmd.CmdType |=0x80;
}	 

void CtrlOxygenValve(bool bEnable)
{
	IedDb.OutputCoil.sCoil16.OXYGEN = bEnable;
    UpdateOutPut();
}

void CtrlNitrogenValve(bool bEnable)
{
	IedDb.OutputCoil.sCoil16.NITROGEN = bEnable;
    UpdateOutPut();
}

void StopAllFan(void)
{
	IedDb.OutputCoil.sCoil16.WATER_STOVE_FAN	= 0;		//ˮ��¯�ӽ�������
	IedDb.OutputCoil.sCoil16.OXYGEN		        = 0;		//��������
	IedDb.OutputCoil.sCoil16.NITROGEN		    = 0;		//��������
	IedDb.OutputCoil.sCoil16.LEFT_STOVE_FAN		= 0;		//��¯����
	IedDb.OutputCoil.sCoil16.RIGHT_STOVE_FAN	= 0;		//��¯����	
	PB0_O=PB1_O=PB9_O = 0;	   								//����¯���ȷ��������
	UpdateOutPut();
}

void EnableLeftFan(bool Enable)
{ 								  
	IedDb.OutputCoil.sCoil16.LEFT_STOVE_FAN		= Enable;		//��¯��������		
    UpdateOutPut();    
}
void EnableRightFan(bool Enable)
{								 																					 
	IedDb.OutputCoil.sCoil16.RIGHT_STOVE_FAN	= Enable;		//��¯����				  
	UpdateOutPut();
}
void EnableWaterFan(bool Enable)
{

	IedDb.OutputCoil.sCoil16.WATER_STOVE_FAN	= Enable;		//ˮ��¯�ӽ�������		
	UpdateOutPut();
	PB0_O=PB1_O=PB9_O = Enable;	   								//����¯���ȷ��������
}




template<typename TBase>
void VertStoveBar<TBase>::ReadOutputInfo(void)
{
    IedDb.OutputCoil.iCoil16 = GPIOC->ODR;
}

template<typename TBase>
int VertStoveBar<TBase>::MoveStoveBar(unsigned int pos)
{
	//  ��������ת��,��������								    
	StepMotorCtrl::StepMotorState *MtrState = &IedDb.MtrState;
    
	if(((MtrState->bRun!=0)		//����δ�Կ׻�������ת��,����������
		||(MtrState->DestPulseCnt!=MtrState->CurPulseCnt))
		&&(pos!=POS_BOT))
		return -2;
    //BUG............�����������ֻ���ڲ���λ�������ֻ������,�����½�
    
	if(m_StovePos==pos)  //�Ѿ���λ,�������ƶ�
	{
		#ifdef _DEBUG
		printf("�Ѿ���λ,����Ҫ���ƶ�!\n");
		#endif
		return -1; 
	}	
	switch(pos)
	{
		case POS_TOP:
			bDirUp 	= true;
			bRun	= true;	
			break;
		case POS_BOT:
			t_RunTimer +=500;		  //5s
			bDirUp	= false;
			bRun	= true;
			break;
		case POS_UNKNOWN:
			bRun	= false;	   
			break;				
	}								  
	return 0;
}	

	
