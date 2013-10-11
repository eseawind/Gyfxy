#ifndef _CTRL_H_
#define _CTRL_H_
#include "task.h" 
#include "FUZZY.H"
#include "balance.h"
#ifndef __cplusplus
	#error "this file must be compiled by C++"
#endif
#define FALSE false
#define TRUE  true
 

class Beep
{
	private:
		uint32_t mBeepTrig;
		int32_t mNumBeep ;
		uint32_t *pBeepPort;
	public: 
		Beep()
		{
			mBeepTrig 	= 0;
			mNumBeep  	= 0;
			pBeepPort	= (uint32_t*)&PC13_O;
		}
		void beep(uint32_t	cNumBeep,uint32_t beepTime)
		{			    
			mNumBeep=cNumBeep;	 		    
		    ActiveTask(TASK_BEEP,beepTime<10?10:beepTime); 
		}
        
		void TaskBeep(void)
		{
		    if(!mBeepTrig)
			{
				*pBeepPort = 1;      
			}
		    else
		    {
				*pBeepPort = 0;						   
				mNumBeep--;
				if(mNumBeep==0)
				{
			    	SuspendTask(TASK_BEEP);
				}
		    }
		    mBeepTrig=!mBeepTrig;
		}	  
} ;
extern Beep beep;
#define BeepMtrErr()    beep.beep(1,50)
#define BeepBalErr()    beep.beep(1,100)
#define BeepOK()        beep.beep(1,1000)


 
#define STOVE_BAR_MID 		0
#define STOVE_BAR_LEFT 		0
#define STOVE_BAR_RIGHT 	1 

template<typename TBase>
class VertStoveBar
{

private:
	bool     bRun;							//����
	bool     bDirUp;						//���з���
	
	uint32_t *Top_OptoCoupler;				//��������
	uint32_t *Bot_OptoCoupler;				//���׹���
	uint32_t *Motor_UpCtrl;					//�������п���
	uint32_t *Motor_DnCtrl;					//�������п���
	int32_t  t_RunTimer;            		//��������ʱ��
	uint16_t  *t_ToptoBotTimer;				//�������׳�ʱʱ��	
public:
	uint32_t m_StovePos;				   	//λ��
	enum StoveBarPos{POS_TOP=0,POS_UNKNOWN,POS_BOT};
	enum StoveDir{UP=0,DOWN}	;
    bool isRun(){return bRun;}
    uint32_t GetTopOpto(){ return *Top_OptoCoupler;} 
    uint32_t GetBotOpto(){ return *Bot_OptoCoupler;}
	///////////////////////////////////////////////////////////////////
	VertStoveBar()
	{
		bRun				= false;						//����
	  	bDirUp				= false;						//���з���
		m_StovePos			= VertStoveBar::POS_UNKNOWN;   	//λ��
		t_RunTimer			= 8000;            				//��������ʱ��
		//*t_ToptoBotTimer	= 18000;						//�������׳�ʱʱ��	
	}
	VertStoveBar(										  
		uint32_t *Top_OptoCoupler,				//��������
		uint32_t *Bot_OptoCoupler,				//���׹���
		uint32_t *Motor_UpCtrl,					//�������п���
		uint32_t *Motor_DnCtrl,					//�������п���
		int32_t  t_RunTimer,            		//��������ʱ��
		uint16_t  *t_ToptoBotTimer				//�������׳�ʱʱ��	
	)
	{	
		VertStoveBar();
		InitStoveBar(
			Top_OptoCoupler,				//��������
			Bot_OptoCoupler,				//���׹���
			Motor_UpCtrl,					//�������п���
			Motor_DnCtrl,					//�������п���						  
			t_ToptoBotTimer);				//�������׳�ʱʱ��				
	}
    void SetMaxRunTimer(){t_RunTimer=*t_ToptoBotTimer;}
    void SetMinRunTimer(){t_RunTimer=0;}
    void ReadOutputInfo(void); 
	void StopStoveBar(void)
	{
		*Motor_DnCtrl = 0;
		*Motor_UpCtrl = 0;
        
		bRun = 0;
	}
	void TaskStopBar(void)
	{
		if(bRun)
		{  
			m_StovePos = POS_UNKNOWN;
			if((*Motor_DnCtrl)&&(*Motor_UpCtrl))   //�������ߵ�ƽ ֹͣ
			{
				StopStoveBar(); 
			}
			if(bDirUp)   //���� 
			{
				t_RunTimer++;
				*Motor_UpCtrl = Top_OptoCoupler?1:0;
					//��絽λ  			//��ʱ,ֹͣ
				if((*Top_OptoCoupler == 0)||(t_RunTimer>=*t_ToptoBotTimer))
				{ 
					StopStoveBar();					//�м����� 1370    //����¯	 1270
					__NOP();
				}					   
			}
			else	
			{
				t_RunTimer--;
				*Motor_DnCtrl = Bot_OptoCoupler?1:0;
				if((*Bot_OptoCoupler == 0)||(t_RunTimer<=0 ))//��ʱ,ֹͣ
				{
					t_RunTimer	  	= 0;
					StopStoveBar();
					__NOP();
				}
			}  
		}
		else
		{ 
			if(*Top_OptoCoupler==0)//��������Ϊ0
				m_StovePos = POS_TOP;	
			else if((!bDirUp)&&(*Bot_OptoCoupler==0)) //�������� ���ҵ���  ���ǵ�λ
			{
				m_StovePos = POS_BOT;
				t_RunTimer = 0;
			}
			else
				m_StovePos = POS_UNKNOWN;	
		} 
        ReadOutputInfo(); //IedDb.OutputCoil.iCoil16 = GPIOC->ODR;        
	}
	int MoveStoveBar(unsigned int pos);	
	void InitStoveBar(
		uint32_t *Top_Opto,				//��������
		uint32_t *Bot_Opto,				//���׹���
		uint32_t *UpCtrl,					//�������п���
		uint32_t *DnCtrl,					//�������п���
		uint16_t *t_ToptoBotTimer
	)
	{
	
		Top_OptoCoupler = Top_Opto;				//��������
		Bot_OptoCoupler = Bot_Opto;				//���׹���
		Motor_UpCtrl	= UpCtrl;				//�������п���
		Motor_DnCtrl	= DnCtrl;				//�������п���
		
		          		//��������ʱ��
		this->t_ToptoBotTimer		= t_ToptoBotTimer;	
		this->t_RunTimer			= *t_ToptoBotTimer/2;  	
	}
		    
};								//¯������



class LowVertStoveBar :public VertStoveBar<LowVertStoveBar>
{
};
class HighVertStoveBar :public VertStoveBar<HighVertStoveBar>
{
};
 
#define LOW_POS_BOT         LowVertStoveBar::POS_BOT 
#define HIGH_POS_BOT        HighVertStoveBar::POS_BOT
#define LOW_POS_TOP         LowVertStoveBar::POS_TOP 
#define HIGH_POS_TOP        HighVertStoveBar::POS_TOP 

extern  LowVertStoveBar     m_LowTempStoveBar[];
extern  HighVertStoveBar    m_HighTempStoveBar[];
//
#define LowTempStove        m_LowTempStoveBar[0]
#define LeftHighTempStove   m_HighTempStoveBar[0]
#define RightHighTempStove  m_HighTempStoveBar[1]


#define MAX_SAMPLE_CNT  24
//#define DEFAULT_SUBDIV	16
#define DEFAULT_SUBDIV	40
#define DEFAULT_PULSECNT_PERROUND	200  
#define MAX_FREQ		16000
#define INFO_LENTH      32
////////////////////////////////////////////////////////////
class  StepMotorCtrl
{
public:
	enum DIR{FORWARD=1,BACKWARD=!FORWARD};
	enum B_LOCK{LOCK=0,UNLOCK=!LOCK};
	struct StepMotorState 
	{										
		bool bRun;							//����
		bool bDir;							//����
		volatile int32_t DestPulseCnt;		//����Ŀ��λ��������
		volatile uint32_t DestSamplePos;	//Ŀ�깤λ

		volatile int32_t CurPulseCnt;		//��ǰλ��������
		volatile uint32_t CurSamplePos;		//��ǰ��λ
																		  
		
		volatile int32_t PulsePerRound;	//ÿ�ƶ�һȦ��Ҫ��������
		volatile uint32_t *ResetIn;			//��λ����˿�
	};
private:
	//StepMotorState MtrState;
	StepMotorState *pMtrState;
	/////////////////////////////////////////////////////////////
	short *PulsePerRoundSubDiv;			//ÿȦ1ϸ��������Ҫ������
	BYTE *SubDiv;						//ϸ����  	
	BYTE *AllSampleCnt;					//���λ
	////////////////////////////////////////////////////////////////
	TIM_TypeDef* TIMx;					//ʹ�õĶ�ʱ��
	int  Channel;						//��ʱ��ͨ��				
	////////////////////////////////////////////////////////////////	
	uint32_t *pPortFree;				//��������
	uint32_t* pPortDir;					//��������
private:
	
	void InitTimer(void);
public:
	StepMotorCtrl();	  	
	//int MoveStepMotor(unsigned long step,StepMotorCtrl::DIR direction);	
	void SetFreq(uint16_t Freq);  
	int MoveStepMotor(long DestPulseCnt,StepMotorCtrl::DIR direction);	
	int MoveStepMotor(long DestPos);
	void StopMotor(void);

	void InitCtrl(TIM_TypeDef *TIMx ,int Channel, uint32_t* pPortFree, uint32_t* pPortDir ,uint32_t* pResetIn);
	void InitMotor(
			long AllSampleCnt 			= MAX_SAMPLE_CNT,
			long PulsePerRoundSubDiv	= DEFAULT_PULSECNT_PERROUND,
			long SubDiv					= DEFAULT_SUBDIV);
};
extern StepMotorCtrl StepMotor[2];  

#define  StepMotor0 StepMotor[0]
#define  StepMotor1 StepMotor[1]

class CFuzzy;
///////////////////////////////////////////
typedef struct 
{
	struct DownParam
	{
        
		USHORT			m_sTimePut2weigh;//����������ʱ��(0_1S);
		USHORT			m_sTimePut2HighTemp;//����������ʱ��(0_1S);
		SHORT			m_sPulsePerRound1Spit;//����һȦ������;
		BYTE			m_btDriverSpitCnt;//������ϸ����;
		BYTE			m_btMaxSampleCnt;//��λ����
        BYTE			m_ByteReserve[8];
        
		SHORT			m_fBalanceMaxErr;//������ֵ(0.1mg);
		SHORT			m_fVolSampleMax;	  //����������
		SHORT			m_fVolSampleMin;     //��С��������  
            
		SHORT			m_fWaterAshSampleMax;
		SHORT			m_fWaterAshSampleMin;
        
		//SHORT			m_sLowStoveTempSpeed;//����¯��������
		//SHORT			m_sHighStoveTempSpeed;//����¯��������;	
        /*
		SHORT			m_sQuick_WaterTestTime;//����ˮ�ָ���ʱ��(��);
		SHORT			m_sQuick_VolTestTime;//�ָ���ʱ��(��);
		SHORT			m_sQuick_AshTestTime;//���ٻҷָ���ʱ��(��);

		SHORT			m_sGB_WaterTestTime;//����ˮ�ָ���ʱ��(��);
		SHORT			m_sGB_VolTestTime;//����ӷ��ָ���ʱ��(��);
		SHORT			m_sGB_AshTestTime;//����ҷָ���ʱ��(��);
        */
        
		SHORT			m_sWaterTestTime;//����ˮ�ָ���ʱ��(��);
		SHORT			m_sVolTestTime;//����ӷ��ָ���ʱ��(��);
		SHORT			m_sAshTestTime;//����ҷָ���ʱ��(��);
        
		SHORT			m_sWaterCoolTime;		//ˮ��¯������ȴʱ��
		SHORT			m_sHighTempCoolTime;	//����¯������ȴʱ��
        
		SHORT			m_sBeepTime;	

		LONG			m_sWaterTestTemp;//����ˮ�������¶�(��);
		LONG			m_sVolTestTemp;//���ٻӷ��������¶�(��);
		LONG			m_sAshTestTemp;//���ٻҷ������¶�(��);
        
		SHORT			m_szTestSequence;     //����˳��
		SHORT			m_szalgorithm;			//���Է���
		enum M_ALG{M_GB=0,M_QK};	
        char            m_szSystemType;
		char            m_cReserve[3];
		long            m_reServe[8]; 
	}DownloadWorkParam;  //���ز���

	    ///////////////////////////////////////////////////////////////////   
	union 			  //����Ĵ���
	{
		struct 	
		{	
			UINT BALANCE_MOTOR_UP	:1;		//��������
			UINT BALANCE_MOTOR_DN	:1;		//�����½�
			UINT LEFT_MOTOR_UP		:1;		//��¯����
			UINT LEFT_MOTOR_DN		:1;		//��¯�½�

			UINT RIGHT_MOTOR_UP		:1;		//��¯����
			UINT RIGHT_MOTOR_DN		:1;		//��¯�½�	
			UINT Reserve1			:2;		//��������׼�����������������ת����

			
			UINT OXYGEN		        :1;		//��¯��������
			UINT NITROGEN		    :1;		//��¯��������
            UINT WATER_STOVE_FAN	:1;		//ˮ��¯�ӽ�������
			UINT LEFT_STOVE_FAN		:1;		//��¯����
			UINT RIGHT_STOVE_FAN	:1;		//��¯����
			UINT SPK				:1;		//������
			UINT Reserve2			:2;		// 
		}sCoil16;	
		UCHAR	iCoil8;			 
		USHORT  iCoil16;          
		UINT    cCoil32;                  
	}OutputCoil;                                                      //32BIT
    union 					        								  //�˲����Ѿ�����
    {																							   
        struct  input
        {
			UINT    SAMPLE_POS_1			:1;  //��λ1
			UINT    SAMPLE_POS_N			:1;  //��λN
			UINT    WATER_STOVE_PUT			:1;  //��λN
			UINT    WATER_STOVE_WEIGH		:1;  //������
			UINT	LEFT_STOVE_BOT			:1;  //��λ
			UINT	LEFT_STOVE_TOP			:1;  //���λ
			UINT	RIGHT_STOVE_BOT			:1;  //��λ
			UINT	RIGHT_STOVE_TOP			:1;  //���λ												  
        };
		uint8_t 	cInput8;
        uint16_t    cInput16;  
		uint32_t    cInput32; 
    }InputStatus;     
	struct
	{
		long ad[3];
		long    adInstTemp;   //���廷���¶�
	}ADValue;									 	    
	CFuzzy fuzzy[FUZZY_ARRAY_SIZE];
	Balance::BalanceBuf buf;
	StepMotorCtrl::StepMotorState MtrState;

	//////////////////////////////////////////////////
	short  	ResetState;		//��λ״̬
    short   ErrorFlag;      //�����־,��������д���,��ȡ����״̬��
	long 	TestState;		//����״̬
	long    MetageState;	//��������
	typedef struct ypd      //��Ʒ���ݶ���
	{
		//enum STATE {S_NULL=0,S_WAIT,S_WEIGHT_POT,S_WEIGHT_SAMPLE,
		//				S_TEST_WATER,S_WEIGHT_COVER,S_TEST_VOL,
		//				S_TEST_ASH};
	 	UCHAR cSta;     //��ǰ��Ҫ����״̬, ��λ���������� 0x03+(cItem<<2)
	 	UCHAR cCSta;    //����������״̬, //��Ҫ����������
	 	UCHAR cItem;    //�������ݣ�b0=1:ˮ�֣�b1=1:�ӷ��֣�b2=1:�ҷ� 
		UCHAR reserve;	
		union{	
			struct{
			long  lMPot;    //��������(unit=0.1mg)
			/////////////////////////////////////////////////////////////////
		    long  lM0;      //����ǰ��Ʒ����(unit=0.1mg),������������
		    long  lM1;      //ˮ���������Ʒ����(unit=0.1mg),������������
			/////////////////////////////////////////////////////////////////
			long  lMPotSample2;//�ӷ���ʵ��ǰ�Ӹ�����
			//
			long  lM2;      //�ӷ����������Ʒ����(unit=0.1mg),������������
			long  lM3;      //�ҷ��������Ʒ����(unit=0.1mg),������������
			}SQuality;
			long Q[6];
		}UQuality;
		
	}YP_DATA;	
	YP_DATA Sample1[24] ;	 
	
	//////////////////////////////////////////////////
	struct
	{
		UCHAR   CmdType;
        UCHAR   CmdParam1;
        UCHAR   CmdParam2;   
        UCHAR   CmdParam3;   
        LONG    CmdParam;   
	}Cmd;
 
	char InstrumentName[32];//32BIT[]
    char ErrorMsg[INFO_LENTH];  //������Ϣ
    /////////////////////////////////////////////////////////////////   
	uint8_t   cID;                //IEDվ��
	uint8_t	  ReserveID;

} IED_DATABASE;
extern  IED_DATABASE  IedDb;
#define ErrBuf IedDb.ErrorMsg
#define ErrFlg IedDb.ErrorFlag
#define MaxSampleCnt IedDb.DownloadWorkParam.m_btMaxSampleCnt
enum enum_IED_type{G_NONE = 0,G5000,G5200,G5500}; 
/////////////////////////////////////////
void TaskBlinLed(void);
void TaskStopBar(void);
void TaskParsaCmd(void);
void TaskBeep(void);
void InitIED(void);
void CtrlOxygenValve(bool bEnable);
void CtrlNitrogenValve(bool bEnable);
void StopAllFan(void);
void EnableLeftFan(bool Enable)	 ;
void EnableRightFan(bool Enable);
void EnableWaterFan(bool Enable) ;

#if defined ( __CC_ARM   )
	void InitStepMotor(void);
	void InitStove(void);
#endif


#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                                                                 
	extern "C"        {
	#endif
#endif
	void TIM4_IRQHandler(void) 	 ;
#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                 
	}
	#endif
#endif

#endif


