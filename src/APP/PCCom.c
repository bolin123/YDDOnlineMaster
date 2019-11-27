#include "PCCom.h"
#include "Sys.h"
#include "DeviceData.h"

#define PCCOM_UART_PORT HAL_UART_PORT_4

#define PCCOM_PROTO_HEAD1 'T'
#define PCCOM_PROTO_HEAD2 'H'
#define PCCOM_485DE_PIN 0x2c //pc12
#define PCCOM_RECV_MODE() HalGPIOSetLevel(PCCOM_485DE_PIN, 0)
#define PCCOM_SEND_MODE() HalGPIOSetLevel(PCCOM_485DE_PIN, 1)

typedef enum
{
    PCCOM_PROTO_CMD_ACK = 0,
    PCCOM_PROTO_CMD_TEST,
    PCCOM_PROTO_CMD_READ,
    PCCOM_PROTO_CMD_TIMING,
    PCCOM_PROTO_CMD_INTERVAL,
    PCCOM_PROTO_CMD_REBOOT,
    PCCOM_PROTO_CMD_HISTORY,
}PCComProtoCmd_t;

static uint8_t g_recvBuff[128];
static uint16_t g_buffcount = 0;
static volatile bool g_gotframe = false;
//static bool g_startReportData = false;
static PCComEventHandle_t g_eventHandle;

static uint8_t crc(uint8_t *data, uint32_t len)
{
    uint8_t i;
    uint8_t value;
    
    value = 0;
    while(len--)
    {
        value ^= *data++;
        for(i = 0; i < 8; i++)
        {
            if(value & 0x01)
            {
                value = (value >> 1) ^ 0x8C;
            }
            else 
            {
                value >>= 1;
            }
        }
    }
    return value;
}
static void pccomDataRecv(uint8_t *data, uint16_t len)
{
    uint16_t i;
    static uint8_t dataLength = 0;

    if(g_gotframe)
    {
        return;
    }
    
    for(i = 0; i < len; i++)
    {
        g_recvBuff[g_buffcount++] = data[i];

        if(g_buffcount == 1)
        {
            if(data[i] != PCCOM_PROTO_HEAD1)
            {
                g_buffcount = 0;
            }
        }
        else if(g_buffcount == 2)
        {
            if(data[i] != PCCOM_PROTO_HEAD2)
            {
                g_buffcount = 0;
            }
        }
        else if(g_buffcount == 3)
        {
            if(data[i] != SysCommAddressGet())
            {
                g_buffcount = 0;
            }
        }
        else if(g_buffcount == 5)
        {
            dataLength = data[i];
        }
        else if(g_buffcount == dataLength + sizeof(PCComProto_t) + 1)
        {

            PCComProto_t *proto = (PCComProto_t *)g_recvBuff;
            if(crc(proto->data, proto->length) == g_recvBuff[dataLength + sizeof(PCComProto_t)])
            {
                g_gotframe = true;
            }
            g_buffcount = 0;
        }
        
    }
}

static void pccomDataSend(PCComProtoCmd_t cmd, uint8_t *data, uint16_t length)
{
    uint8_t buff[255];
    uint8_t bufflen;
    PCComProto_t *proto = (PCComProto_t *)buff;
    
    PCCOM_SEND_MODE();

    proto->head[0] = PCCOM_PROTO_HEAD1;
    proto->head[1] = PCCOM_PROTO_HEAD2;
    proto->addr = SysCommAddressGet();
    proto->cmd = (uint8_t)cmd;
    proto->length = length;
    memcpy(proto->data, data, length);
    proto->data[length] = crc(data, length);

    bufflen = sizeof(PCComProto_t) + length + 1;
    HalUartWrite(PCCOM_UART_PORT, buff, bufflen);

    for(uint8_t i = 0; i < bufflen; i++)
    {
        printf("%02x ", buff[i]);
    }
    printf("\n");
    PCCOM_RECV_MODE();
}

static void sendACK(void)
{
    pccomDataSend(PCCOM_PROTO_CMD_ACK, NULL, 0);
}

void PCComDataReport(uint8_t *data, uint16_t len)
{
    pccomDataSend(PCCOM_PROTO_CMD_READ, data, len);
}

void PCComHistoryDataReport(uint8_t *data, uint16_t len)
{
    pccomDataSend(PCCOM_PROTO_CMD_HISTORY, data, len);
}

static void frameHandle(void)
{
    PCComProto_t *proto = (PCComProto_t *)g_recvBuff;
    uint16_t interval;
    uint32_t utc;
//    uint8_t buff[140];
//    uint32_t len;

    if(g_gotframe)
    {
        switch (proto->cmd)
        {
            case PCCOM_PROTO_CMD_TEST:
                Syslog("PCCOM_PROTO_CMD_TEST");
                sendACK();
                break;
            case PCCOM_PROTO_CMD_READ:
                memcpy(&utc, proto->data, 4);
                Syslog("PCCOM_PROTO_CMD_READ");
                g_eventHandle(PCCOM_EVENT_DATA_REPORT, (void *)utc);
                break;
            case PCCOM_PROTO_CMD_TIMING:
                memcpy(&utc, proto->data, 4);
                Syslog("PCCOM_PROTO_CMD_TIMING, value = %d", utc);
                sendACK();
                g_eventHandle(PCCOM_EVENT_TIMING, (void *)utc);
                break;
            case PCCOM_PROTO_CMD_INTERVAL:
                memcpy(&interval, proto->data, 2);
                Syslog("PCCOM_PROTO_CMD_INTERVAL, value = %d", interval);
                sendACK();
                g_eventHandle(PCCOM_EVENT_SLEEP_INTERVAL, (void *)(uint32_t)interval);
                break;
            case PCCOM_PROTO_CMD_REBOOT:
                Syslog("PCCOM_PROTO_CMD_REBOOT");
                sendACK();
                g_eventHandle(PCCOM_EVENT_REBOOT, NULL);
                break;
            case PCCOM_PROTO_CMD_HISTORY:
                Syslog("PCCOM_PROTO_CMD_HISTORY");
                g_eventHandle(PCCOM_EVENT_HISTORY_REPORT, NULL);
                break;
            default:
                break;
        }
        g_gotframe = false;
    }
}


void PCComInit(PCComEventHandle_t eventHandle)
{
    HalUartConfig_t config;
    config.enble = true;
    config.baudrate = 9600;
    config.flowControl = 0;
    config.parity = 0;
    config.wordLength = USART_WordLength_8b;
    config.recvCb = pccomDataRecv;
    HalUartConfig(PCCOM_UART_PORT, &config);
    g_eventHandle = eventHandle;

    //enable pin
    HalGPIOConfig(PCCOM_485DE_PIN, HAL_IO_OUTPUT);//pa12
    //HalGPIOSetLevel(PCCOM_485DE_PIN, 0);
    PCCOM_RECV_MODE();
}

void PCComPoll(void)
{   
    frameHandle();
}

