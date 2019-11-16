#include "WiredProto.h"
#include "VTStaticQueue.h"
#include "DeviceData.h"

#define WIRED_PROTO_UART_PORT HAL_UART_PORT_1
#define WIRED_PROTO_CONTEXT_HEAD 0xf5
#define WIRED_PROTO_MASTER_ADDR 0x00

#define WIRED_PROTO_CMD_QUERY 0x01

#define WIRED_PROTO_485DE_PIN 0x0b //
#define WIRED_PROTO_RECV_MODE() HalGPIOSetLevel(WIRED_PROTO_485DE_PIN, 0)
#define WIRED_PROTO_SEND_MODE() HalGPIOSetLevel(WIRED_PROTO_485DE_PIN, 1)

typedef struct
{
    uint8_t head;
    uint8_t addr;
    uint8_t cmd;
    uint8_t dlen;
    uint8_t data[];
}WiredProtoContext_t;

typedef struct
{
    uint8_t address;
    uint8_t devType;
    uint8_t errcode;
    uint8_t power;
    uint8_t data[];
}WiredProtoReportData_t;

//static uint8_t g_wiredBuff[128];
static VTSQueueDef(uint8_t, g_wiredQueue, 512);
static volatile bool g_wiredFrameGot = false;

static uint8_t checkSum(uint8_t *data, uint8_t len)
{
    uint8_t i;
    uint8_t sum = 0;

    for(i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum;
}

static void wiredDataRecv(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++)
    {
        if(VTSQueueHasSpace(g_wiredQueue))
        {
            VTSQueuePush(g_wiredQueue, data[i]);
        }
    }

}

static void framePrase(WiredProtoContext_t *context)
{
//    char mac[4] = "";
    uint16_t data[8];
    uint16_t dlen;
    
    WiredProtoReportData_t *reportData;
    if(context)
    {
        if(context->cmd == WIRED_PROTO_CMD_QUERY)
        {
            reportData = (WiredProtoReportData_t *)context->data;

            //sprintf(mac, "%03d", reportData->address);
            dlen = context->dlen - sizeof(WiredProtoReportData_t);
            memcpy(data, reportData->data, dlen);
            DeviceDataInsert(reportData->address, reportData->devType, reportData->errcode, reportData->power, data, dlen / 2);
        }
    }
}

static void wiredFrameHandle(void)
{
//    uint16_t i;
    uint8_t byte;
    static uint32_t lastTime;
    static uint8_t wiredBuff[128];
    static uint8_t wiredBuffCount = 0;
    static uint8_t dataLength = 0;

    while(VTSQueueCount(g_wiredQueue))
    {
        if(SysTimeHasPast(lastTime, 200))
        {
            wiredBuffCount = 0;
        }
        lastTime = SysTime();

        byte = VTSQueueFront(g_wiredQueue);
        VTSQueuePop(g_wiredQueue);
        wiredBuff[wiredBuffCount++] = byte;

        if(wiredBuffCount == 1)
        {
            if(byte != WIRED_PROTO_CONTEXT_HEAD)
            {
                wiredBuffCount = 0;
            }
        }
        else if(wiredBuffCount == 2)
        {
            if(byte != WIRED_PROTO_MASTER_ADDR)
            {
                wiredBuffCount = 0;
            }
        }
        else if(wiredBuffCount == 4)
        {
            dataLength = byte;
        }
        else if(sizeof(WiredProtoContext_t) + dataLength + 1 == wiredBuffCount)
        {
            WiredProtoContext_t *context = (WiredProtoContext_t *)wiredBuff;
            if(checkSum(context->data, context->dlen) == wiredBuff[sizeof(WiredProtoContext_t) + dataLength])
            {
                framePrase(context);
                break;
            }
            dataLength = 0;
            wiredBuffCount = 0;
        }
    }
}

void WiredProtoQuery(void)
{
    uint8_t i = 0;
    uint8_t buff[128];
    Syslog("");
    WIRED_PROTO_SEND_MODE();
    buff[i++] = WIRED_PROTO_CONTEXT_HEAD;
    buff[i++] = 0xff; //¹ã²¥
    buff[i++] = WIRED_PROTO_CMD_QUERY;
    buff[i++] = 0;
    buff[i++] = 0;//sum
    HalUartWrite(WIRED_PROTO_UART_PORT, buff, i);
    WIRED_PROTO_RECV_MODE();
}

void WiredProtoInit(void)
{
    HalUartConfig_t config;
    
    config.enble = true;
    config.baudrate = 9600;
    config.flowControl = 0;
    config.parity = 0;
    config.wordLength = USART_WordLength_8b;
    config.recvCb = wiredDataRecv;
    HalUartConfig(WIRED_PROTO_UART_PORT, &config);

    //485 recv mode default
    HalGPIOConfig(WIRED_PROTO_485DE_PIN, HAL_IO_OUTPUT);//pa11
    WIRED_PROTO_RECV_MODE();
}

void WiredProtoPoll(void)
{
    wiredFrameHandle();
}


