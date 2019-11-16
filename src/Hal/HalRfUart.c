#include "HalRfUart.h"
#include "HalCtype.h"

static HalRfDataRecv_cb g_dataRecvHandle = NULL;
static uint16_t g_dmaLastNum;

void HalRfUartInit(int baudrate, unsigned char *dmaBuff, HalRfDataRecv_cb recvHandle)
{    
    GPIO_InitTypeDef GPIO_InitStructure;
    //uart3 io
    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;  //TX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
     
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;  //RX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    USART_InitTypeDef uartcfg;
    uartcfg.USART_BaudRate = baudrate;
    uartcfg.USART_WordLength = USART_WordLength_8b;
    uartcfg.USART_Parity = USART_Parity_No;
    uartcfg.USART_StopBits = USART_StopBits_1;
    uartcfg.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    uartcfg.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART3, &uartcfg);
    
    DMA_InitTypeDef DMA_InitStructure;

    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dmaBuff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = HAL_RF_UART_BUFF_LEN;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    
    DMA_ClearFlag(DMA1_FLAG_TC3);
    USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE); 
    
    g_dmaLastNum = HAL_RF_UART_BUFF_LEN;
    g_dataRecvHandle = recvHandle;
    
    DMA_Cmd(DMA1_Channel3, ENABLE); 

    USART_Cmd(USART3, ENABLE);
}

void HalRfUartPoll(void)
{
    uint16_t tmp;
    
    if(DMA_GetCurrDataCounter(DMA1_Channel3) != g_dmaLastNum)//剩余待传的数据
    {
        tmp = DMA_GetCurrDataCounter(DMA1_Channel3);

        if(g_dataRecvHandle != NULL)
        {
            g_dataRecvHandle(g_dmaLastNum - tmp);
        }
        g_dmaLastNum = tmp;
    }
    if(DMA_GetFlagStatus(DMA1_FLAG_TC3) == SET)
    {
        DMA_ClearFlag(DMA1_FLAG_TC3);

        if(g_dataRecvHandle != NULL)
        {
            g_dataRecvHandle(g_dmaLastNum);
        }
        g_dmaLastNum = HAL_RF_UART_BUFF_LEN;
    }
}

void HaRflUartWrite(const uint8_t *data, uint16_t len)
{
    uint16_t i;

    for(i = 0; i < len; i++)
    {
        USART_SendData(USART3, (uint16_t)data[i]);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
    }
    
}

