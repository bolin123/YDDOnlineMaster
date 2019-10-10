#include "HalPWM.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"

void HalPWMEnable(char enable)
{
    if(enable)
    {
    	TIM_Cmd(TIM3, ENABLE);		//�򿪶�ʱ��������
    }
    else
    {
    	TIM_Cmd(TIM3, DISABLE);		//�򿪶�ʱ��������
    }
}

void HalPWMInitialize(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; 
	
	//��ʼ��PC8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 	//����Ϊ����ģʽ(PWM���)
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	// ��ʱ����ʼ��
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 3;		// Ԥ��Ƶ��������ֵ���Զ���1
	TIM_TimeBaseStructure.TIM_Period = 26;		// ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// ʱ�ӷָ�
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	// ���ϼ���
	TIM_TimeBaseInit(TIM3, & TIM_TimeBaseStructure);

    // PWM ��ʼ��
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_Pulse = 13;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;		// ���Ϊ��
	TIM_OC3Init(TIM3, & TIM_OCInitStructure);
	
	// ʹ�ܻ���ʧ��TIMx��CCR2�ϵ�Ԥװ�ؼĴ���
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
	
}


