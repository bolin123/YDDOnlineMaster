#include "HalUart.h"

static HalUartConfig_t g_uartConfig[HAL_UART_COUNT];

void HalUartConfig(uint8_t uart, HalUartConfig_t *config)
{   
    USART_InitTypeDef uartcfg;
    USART_TypeDef *uartNo;
    
    USART_StructInit(&uartcfg);
    if(HAL_UART_PORT_1 == uart)
    {
        uartNo = USART1;
    }
    else if(HAL_UART_PORT_2 == uart)
    {
        uartNo = USART2;
    }
    else if(HAL_UART_PORT_3 == uart)
    {
        uartNo = USART3;
    }
    else if(HAL_UART_PORT_4 == uart)
    {
        uartNo = UART4;
    }
    else
    {
        return;
    }
    g_uartConfig[uart] = *config;

    if(config->enble)
    {
        USART_Cmd(uartNo, DISABLE);
        USART_ITConfig(uartNo, USART_IT_RXNE, DISABLE);
        
        uartcfg.USART_BaudRate = config->baudrate;
        uartcfg.USART_WordLength = config->wordLength;
        uartcfg.USART_Parity = config->parity;
        uartcfg.USART_StopBits = USART_StopBits_1;
        uartcfg.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        uartcfg.USART_HardwareFlowControl = config->flowControl;

        USART_Init(uartNo, &uartcfg);
        USART_ClearITPendingBit(uartNo, USART_IT_RXNE);
        USART_ITConfig(uartNo, USART_IT_RXNE, ENABLE);
        USART_Cmd(uartNo, ENABLE);
    }
    else
    {
        USART_Cmd(uartNo, DISABLE);
        USART_ITConfig(uartNo, USART_IT_RXNE, DISABLE);
    }
}

void HalUartInitialize(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    memset(g_uartConfig, 0, sizeof(HalUartConfig_t) * HAL_UART_COUNT);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    GPIO_InitTypeDef GPIO_InitStructure;
    //uart1 io
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;  //TX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
     
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;  //RX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    
#if defined(HAL_OLD_DEVICE)
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //uart2 io
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;  //TX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
     
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;  //RX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
#else //new
    
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //uart4 io
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;  //TX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
     
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;  //RX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

}

void HalUartPoll(void)
{
}

void HalUartWrite(uint8_t uart, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    USART_TypeDef *uartNo;
    
    if(HAL_UART_PORT_1 == uart)
    {
        uartNo = USART1;
    }
    else if(HAL_UART_PORT_2 == uart)
    {
        uartNo = USART2;
    }
    else if(HAL_UART_PORT_3 == uart)
    {
        uartNo = USART3;
    }
    else
    {
        uartNo = UART4;
    }

    for(i = 0; i < len; i++)
    {
        USART_SendData(uartNo, (uint16_t)data[i]);
        while(USART_GetFlagStatus(uartNo, USART_FLAG_TC) == RESET);
    }
    
}

void USART1_IRQHandler(void)
{
    uint8_t data;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        data = USART_ReceiveData(USART1);
        if(g_uartConfig[HAL_UART_PORT_1].recvCb != NULL)
        {
            g_uartConfig[HAL_UART_PORT_1].recvCb(&data, 1);
        }
    }
}

#if defined(HAL_OLD_DEVICE)
void USART2_IRQHandler(void)
{
    uint8_t data;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        data = USART_ReceiveData(USART2);
        if(g_uartConfig[HAL_UART_PORT_2].recvCb != NULL)
        {
            g_uartConfig[HAL_UART_PORT_2].recvCb(&data, 1);
        }
    }
}
#else
void UART4_IRQHandler(void)
{
    uint8_t data;
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        data = USART_ReceiveData(UART4);
        if(g_uartConfig[HAL_UART_PORT_4].recvCb != NULL)
        {
            g_uartConfig[HAL_UART_PORT_4].recvCb(&data, 1);
        }
    }
}

#endif

void USART3_IRQHandler(void)
{
    uint8_t data;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        data = USART_ReceiveData(USART3);
        if(g_uartConfig[HAL_UART_PORT_3].recvCb != NULL)
        {
            g_uartConfig[HAL_UART_PORT_3].recvCb(&data, 1);
        }
    }
}



