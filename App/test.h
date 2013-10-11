#ifndef _TEST_H_
#define _TEST_H_
#include "task.h"

enum REST_STATE {REST_INIT=0,REST_START,REST_SAMPLEDISH,REST_END};
enum IED_STATE{
 IEDS_LOG_BIT        =0,     //1:ϵͳ��ע��
 IEDS_REST_BIT       ,       //1:ϵͳ�Ѹ�λ
 
 IEDS_WEIGHTED_POT_BIT     ,       	//1:ȫ����Ʒ�ѳ�����
 IEDS_WEIGHTED_SAMPLE_BIT  ,       	//1:ȫ����Ʒ�������
 IEDS_DRYPOT_BIT     ,       		//1:ȫ���������Ѻ���
 IEDS_OK_BIT         };       		//1:OK,0:BAD

//�������״̬
enum SAMPLE_TEST_STATE{
 CS_WEIGHTED_POT_BIT = 0,       	//�ѳ�����
 CS_WEIGHTED_SAMPLE_BIT,      		//�ѳ���
 CS_TESTED_WATER_BIT ,       		//ˮ���������
 CS_TESTED_VOL_BIT   ,       		//�ӷ����������
 CS_WEIGHT_COVERT,BIT,
 CS_TESTED_ASH_BIT   ,       		//�ҷ��������
 CS_DRYPOT_BIT       ,       		//���������
 CS_COMPLETE_BIT     };       		//�����������

//��������
enum SAMPLE_TEST_TYPE{
 TEST_WATER_BIT    = 0 ,      //ˮ������
 TEST_VOL_BIT   ,       	  //�ӷ�������
 TEST_ASH_BIT     };       	   //�ҷ�����

//���鷽����0-���귨��1-���ٷ���2-���귨2
enum TESTMODE {TEST_MODE_GB=0,TEST_MODE_QK};

//��ǰ����״̬��0-��������01-������02-ˮ�����飬03,��������,04-�ӷ������飬05-�ҷ����飬
enum STATE {S_WEIGHT_POT=0,S_WEIGHT_SAMPLE,S_TEST_WATER,S_WEIGHT_COVER,S_TEST_VOL,S_TEST_ASH};

void TaskTest(void);
void TaskReset(void);	 
void TaskManualKey(void);  
void TaskBurnSample(void);

#define MAX_WEIGHTING_CNT 6			    
class Metage{
	
	
	unsigned char m_cPos;             //����,����λ��
	unsigned char m_cMDelay;          //������ʱ������
	unsigned char m_cDelayCnt;		  //��ʱ�ȴ�����;
	unsigned char metagType;		  //��������
	unsigned char metagSpeed;		  //��������

	static char M_DELAY;             	//s,������ʱ
	static char	BALANCE_ZERO;
	static char TARE_DELAY;
	static char METAGE_MAXERR;         	//0.3mg �г����ȶ�����
	static char TARE_MAXERR;         	//0.1mg ��ȥƤ�ȶ�����
	BYTE* MAX_SAMPLE;					//�����Ʒ��Ŀ
	

public:	
	long *m_cSchedule;        //��������,�����������
	enum METAGE_STATE {M_INIT=0,M_POS,M_TARE,M_METAGE,M_SAVE,M_NEXTPOS,M_END};
	enum METAGE_TYPE{T_POT=0,T_SAMPLE,T_WATER,T_COVER,T_VOL,T_ASH};	  	//��������
	enum METAGE_SPEED{T_NORMAL=0,T_QUICK};
	volatile long* MetagePtr;         	 							//1:��������������
	Metage()
	{ 
		metagSpeed  = T_NORMAL; 
	}
	void TaskMetage(void);
	void StartMetage(METAGE_TYPE type,METAGE_SPEED speed);
    void ClearMetageFlag(void);
}  ;
extern Metage metage; 
void TaskMetage(void);
void ResetSystem(void);
#define BURN_SAMPLE_OFFSET 7L //����λ1ʱ,¯���µ�����Ϊ6,7 18,19
                              //          1,2    
class Test
{
public:
	enum TEST_STATE {T_INIT=0,
                    T_WARM_WATER,T_TEST_WATER,T_WATERCOOL,T_WEIGHT_WATER,T_WATER_END,
					T_WARM_HIGH1,T_TEST_VOL,T_BURN_VOL,T_WEIGHT_VOL,T_VOL_END,
                    T_WARM_HIGH2,T_TEST_ASH,T_BURN_ASH,T_WEIGHT_ASH,T_ASH_END,	
                    T_END};	
private:
    ////////////////////////////////////////////////////////////////////////                    
    enum BURN_SAMPLE_TYPE{D_WATER=0,D_VOL,D_ASH,D_POT}; //����Ʒ���� 
    enum BURN_STATE{B_INIT=0,B_CHECK,B_POS,B_BURN,B_COOL,B_END};
    
    uint8_t          m_burnState;    //����Ʒ״̬;
    uint16_t         m_BurnTime;     //����ʱ��
    BURN_SAMPLE_TYPE m_burnType;     //��������
    void BurnSample(BURN_SAMPLE_TYPE sampleType);
    
    int GetWillBeTestedType(void); //�õ���һ����Ҫ���Ե���Ŀ
    ////////////////////////////////////////
    uint32_t m_MDelay;
	uint8_t	m_Pos;
	//uint8_t b_Wait;
	signed char m_TestTimer;
public:	
	void TaskTest(void);
	void TaskTestGB(void);
	void TaskTestQK(void);
	void StartTest(void);
	void StopTest(void);
    ///////////////////////////////////////////////////////
    void StartCool(void);
    void StopCool(void);
    ///////////////////////////////////////////////////////
    void TestWater(void);
    void TestVol(void);
    void TestAsh(void);
    void TaskBurnSample(void);
    ///////////////////////////////////////////////////////
    Test(){m_Pos=1;}
};
extern Test test;
#endif
