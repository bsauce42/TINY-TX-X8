/*
2017-8-14 : Ϊ��ֹ����жϼ��໥Ӱ�죬��Ӱ�췢��ʱ�����⡣��ֻ����һ���ж�(��������ʱ���⣬�����ж�)����ʱ��2 ��Ϊ���߷����Լ�1mSʱ����ʹ�á�
*/

#include "include.h"

volatile bool tbc_2ms_flag = false ; 						//ע�� : ���δ�ؾ���������ȷ�� 2mS 
static uint16_t TIM2_Cnt = 0 ; 
uint8_t TIM2_ErrorCnt = 0 ; 
////////////////////////////////////////////////////////////////////////////////
///                  TIM2:���߷���ʱ�������Լ���λ2mS��־λ  	        ////////
////////////////////////////////////////////////////////////////////////////////
void tbc_init (void)
{
  	//������ TIM3 TIM2 ʱ��
	CLK -> PCKENR1 |= (1<<1)|(1<<0);
	//���߷���
	TIM2 -> CR1  |= (1<<3);    						//�жϺ�ֹͣ����
	TIM2 -> PSCR  = 0x03 ;							//ע��: TIM2��Ƶ���� F = Fsc/(PSCR)=12M/8 = 1.5M
	TIM2 -> ARRH  = 0x05 ;							//��ʱ2MS
	TIM2 -> ARRL  = 0xDC ;
	TIM2 -> IER  |= (1<<0);							//��ʼ�� ����ж�
	TIM2 -> CR1  |= (1<<0);    						//��ʼ����
}


////////////////////////////////////////////////////////////////////////////////
/////				���Ź���ʼ��				////////
////////////////////////////////////////////////////////////////////////////////
void WatchDog_Init(void)
{
	//���Ź���ʼ��
  	IWDG -> KR  = 0xCC  ;    						// ʹ�ܿ��Ź�
	IWDG -> KR  = 0x55 ; 							// �������Ź��Ĵ���
	IWDG -> PR  = 0x03 ; 							// Divider/64 =>  f = 38K/32= 1.1875kHz
	IWDG ->	RLR = 200  ;    						// T = 200 / 1.1875kHz �� 170mS
	IWDG -> KR  = 0xAA ; 							// �������Ź��Ĵ���
}

////////////////////////////////////////////////////////////////////////////////
/////			   CC2500���ݷ����ж�				 ///////
/////	Ϊ�ϸ�֤���ͼ�� �����ö�ʱ�ж�����֤����ʱ��(�жϵȼ����)    ///////	
////////////////////////////////////////////////////////////////////////////////
void tbc_t2_Updatisr (void)
{	
	if(TIM2 -> SR1 & (1<<0))
	{
		TIM2 -> SR1 = ~(1<<0) ;
		tbc_2ms_flag = true;
		
		if(CommunicationError_flg == false) TIM2_Cnt = ReadFRSKYD16() ; 
		else  TIM2_Cnt = 0x0BB8 ; 
		
		//�������ֵΪ 0 , ˵�����͹���(ֹͣ ���� , ���ٴ򿪶�ʱ�ж�)
		if(TIM2_Cnt == 0)
		{
			CommunicationError_flg = true ; 
			if(TIM2_ErrorCnt < 250) ++TIM2_ErrorCnt ;
			Write_EE_Byte(TIM2_ErrorCnt , TIM2ErrorCnt_ADDR);
			while(1);
		}
		else
		{
			TIM2 -> ARRH  = TIM2_Cnt >> 8 ;				//��ʱ1MS(ÿ���жϣ�ֹͣ��������ֹ���ͼ����λ)
			TIM2 -> ARRL  = TIM2_Cnt & 0xFF ;
			TIM2 -> CR1  |= (1<<0);
		}
	}
}