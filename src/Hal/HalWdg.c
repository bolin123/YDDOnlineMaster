#include "HalWdg.h"
#include "stm32f10x_iwdg.h"

void HalWdgInitialize(void)
{
#if 1
	//���ϵͳ�Ƿ��ɶ������Ź�ι����λ
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{ 
		//�����λ��־λ 
		RCC_ClearFlag();
		//printf("Watchdog reset!\n");
	}
	//ʹ��д����IWDG_PR and IWDG_RLR �Ĵ���
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	//IWDG��������ʱ��: LSI/32 
	IWDG_SetPrescaler(IWDG_Prescaler_64);

	IWDG_SetReload(0xEA6);//3s
	//Reload IWDG counter 
	IWDG_ReloadCounter();
	//Enable IWDG (the LSI oscillator will be enabled by hardware) 
	IWDG_Enable();
#endif
}

void HalWdgFeed(void)
{
#if 1
	/* ���µ���IWDG������ */
	IWDG_ReloadCounter();  
#endif
}

