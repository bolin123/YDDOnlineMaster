#include "PCCom.h"
#include "Sys.h"
#include "DeviceData.h"

#define PCCOM_UART_PORT HAL_UART_PORT_2

#define PCCOM_PROTO_HEAD1 'T'
#define PCCOM_PROTO_HEAD2 'H'
#define PCCOM_485DE_PIN 0x2a //0x0b //
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
}PCComProtoCmd_t;

typedef struct
{
    uint8_t head[2];
    uint8_t addr;
    uint8_t cmd;
    uint8_t length;
    uint8_t data[];
}PCComProto_t;

static uint8_t g_recvBuff[128];
static uint16_t g_buffcount = 0;
static volatile bool g_gotframe = false;
static bool g_startReportData = false;

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
            if(data[i] != SysGetComAddr())
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
    proto->addr = SysGetComAddr();
    proto->cmd = (uint8_t)cmd;
    proto->length = length;
    memcpy(proto->data, data, length);
    proto->data[length] = crc(data, length);

    bufflen = sizeof(PCComProto_t) + length + 1;
    HalUartWrite(PCCOM_UART_PORT, buff, bufflen);
    PCCOM_RECV_MODE();
}

static void sendACK(void)
{
    pccomDataSend(PCCOM_PROTO_CMD_ACK, NULL, 0);
}

static void dataInsert(void)
{
    uint8_t i;
    uint16_t macnum = 1;
    char mac[SYS_MAC_ADDR_LEN + 1] ="";
    uint8_t devType;
    uint16_t data1[4] = {100, 50, 200, 50};
    uint16_t data2[2] = {125, 105};

    for(i = 0; i < 20; i++)
    {
        mac[0] = '\0';
        sprintf(mac, "%03d", macnum++);
        if(i % 2)
        {
            DeviceDataInsert(mac, 2, i / 10, 98, data1, 4);
        }
        else
        {
            DeviceDataInsert(mac, 1, i / 10, 98, data2, 2);
        }
    }
}

static void frameHandle(void)
{
    PCComProto_t *proto = (PCComProto_t *)g_recvBuff;
    uint16_t interval;
    uint32_t utc;

    if(g_gotframe)
    {
        switch (proto->cmd)
        {
            case PCCOM_PROTO_CMD_TEST:
                Syslog("PCCOM_PROTO_CMD_TEST");
                sendACK();
                break;
            case PCCOM_PROTO_CMD_READ:
                Syslog("PCCOM_PROTO_CMD_READ");
                dataInsert();
                g_startReportData = true;
                break;
            case PCCOM_PROTO_CMD_TIMING:
                memcpy(&utc, proto->data, 4);
                HalRTCSetUtc(utc);
                Syslog("PCCOM_PROTO_CMD_TIMING, value = %d", utc);
                sendACK();
                break;
            case PCCOM_PROTO_CMD_INTERVAL:
                memcpy(&interval, proto->data, 2);
                Syslog("PCCOM_PROTO_CMD_INTERVAL, value = %d", interval);
                sendACK();
                break;
            case PCCOM_PROTO_CMD_REBOOT:
                Syslog("PCCOM_PROTO_CMD_REBOOT");
                sendACK();
                break;
            default:
                break;
        }
        g_gotframe = false;
    }
}

static void dataReport(void)
{
    DeviceDataReport_t *dataHead;
    DeviceDataReport_t *node;
    uint16_t count, len;
    uint8_t buff[255];
    static uint8_t packetnum = 1;
    
    if(g_startReportData)
    {   
        count = 2;
        dataHead = DeviceDataGetHead();
        if(dataHead)
        {
            VTListForeach(dataHead, node)
            {
                if(DEVICE_DATA_PACKET_LEN(node->dataNum * 2) + count > sizeof(buff))
                {   
                    buff[0] = packetnum++;
                    buff[1] = 1;
                    pccomDataSend(PCCOM_PROTO_CMD_READ, buff, count);
                    count = 2;
                    break;
                }
                else
                {
                    memcpy(&buff[count], node->mac, SYS_MAC_ADDR_LEN);//mac
                    count += SYS_MAC_ADDR_LEN;
                    buff[count++] = node->devType;
                    buff[count++] = node->errcode;
                    buff[count++] = node->power;
                    memcpy(&buff[count], &node->utctime, 4);
                    count += 4;
                    memcpy(&buff[count], node->data, node->dataNum * 2);
                    count += node->dataNum * 2;
                    VTListDel(node);
                    free(node->data);
                    free(node);
                }
            }

            if(count > 2)
            {
                buff[0] = packetnum++;
                buff[1] = 0;
                pccomDataSend(PCCOM_PROTO_CMD_READ, buff, count);
                packetnum = 0;
                g_startReportData = false;
            }
        }
    }
}

static void testSend(void)
{
    uint8_t data[5] = {1,2,3,4,5};
    static uint32_t lastTime;

    if(SysTimeHasPast(lastTime, 1000))
    {
        PCCOM_SEND_MODE();
        HalUartWrite(PCCOM_UART_PORT, data, sizeof(data));
        //HalWaitMs(10);
        PCCOM_RECV_MODE();
        lastTime = SysTime();
    }
}

void PCComInit(void)
{
    HalUartConfig_t config;
    config.enble = true;
    config.baudrate = 9600;
    config.flowControl = 0;
    config.parity = 0;
    config.wordLength = USART_WordLength_8b;
    config.recvCb = pccomDataRecv;
    HalUartConfig(PCCOM_UART_PORT, &config);

    //enable pin
    HalGPIOConfig(PCCOM_485DE_PIN, HAL_IO_OUTPUT);//pa12
    //HalGPIOSetLevel(PCCOM_485DE_PIN, 0);
    PCCOM_RECV_MODE();
}

void PCComPoll(void)
{   
    frameHandle();
    dataReport();
}

