#include "HalExti.h"
//#include "PowerManager.h"

//extern void YDDOnlineSensorFreqTrigger(uint8_t ch);
//extern void YDDOnlineWakeup(void);
extern void MasterLightActive(void);
void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        //PMWakeup(PM_WAKEUP_TYPE_LIGHT);
        MasterLightActive();
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
}


extern void IRRecvBits(void);
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET) //IR_RX_OUT
	{
	    IRRecvBits();
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void HalExtiIRRecvEnable(bool enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    
    if(enable)
    {
        EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
        EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
        EXTI_Init(&EXTI_InitStructure);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line0);
        EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
        EXTI_Init(&EXTI_InitStructure);
    }
}

void HalExtiInitialize(void)
{
    //SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, GPIO_PinSource7 | GPIO_PinSource8 | GPIO_PinSource9);
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;


    //light irq pin pc6
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);

    //light irq enable
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line6; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure); 

    //IR input pd0
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource0);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure); 
}

void HalExtiPoll(void)
{
}


