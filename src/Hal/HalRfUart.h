#ifndef HAL_DMA_UART_H
#define HAL_DMA_UART_H

#define HAL_RF_UART_BUFF_LEN 1024

typedef void (*HalRfDataRecv_cb)(unsigned short length);

void HalRfUartInit(int baudrate, unsigned char *dmaBuff, HalRfDataRecv_cb recvHandle);
void HaRflUartWrite(const unsigned char *data, unsigned short len);
void HalRfUartPoll(void);
#endif

