#include "RFModule.h"
#include "Wireless.h"
#include "PowerManager.h"
#include "VTStaticQueue.h"

#define RFMODULE_UART_PORT HAL_UART_PORT_3 //uart3

typedef enum
{
    RFMODULE_FRAME_TYPE_AT = 0,
    RFMODULE_FRAME_TYPE_DATA,
}RFModuleFrameType_t;

static bool g_rfModuleDetected = false;
static uint8_t g_rfRecvBuff[256];
static uint16_t g_rfBuffCount = 0;
static volatile bool g_gotFrame = false;
static RFModuleFrameType_t g_frameType;
static bool g_setIntervalEnable = false;
static uint16_t g_intervalNum;
static VTSQueueDef(uint8_t, g_uartBuff, 512);

static void intervalConfig(void)
{
    static uint32_t oldTime;
    char args[16] = "";
    
    if(g_setIntervalEnable && SysTimeHasPast(oldTime, 500))
    {
        sprintf(args, "ATCR %x\r", g_intervalNum);
        HalUartWrite(RFMODULE_UART_PORT, (const uint8_t *)args, strlen(args));
        oldTime = SysTime();
    }
}

static void dataRecvByte(uint8_t byte)
{   
    if(g_rfBuffCount == 1)
    {
        if(byte != '~')
        {
            g_rfBuffCount = 0;
        }
    }
    else
    {
        if(g_rfBuffCount >= sizeof(g_rfRecvBuff))
        {
            g_rfBuffCount = 0;
        }
        
        if(byte == '\r')
        {
            g_rfRecvBuff[g_rfBuffCount] = '\0';
            g_frameType = RFMODULE_FRAME_TYPE_DATA;
            g_gotFrame = true;
        }

    }
}

static void atcmdParse(char *atcmd)
{
    uint8_t i;
    uint8_t mac[SYS_MAC_ADDR_LEN];
    char *macstr;
    char tmp[3] = {0};

    Syslog("%s", atcmd);

    if(strstr(atcmd, "ATOK") != NULL)
    {
        g_setIntervalEnable = false;
    }
    else if(strstr(atcmd, "ATCi") != NULL)
    {
        macstr = &atcmd[72];
        for(i = 0; i < SYS_MAC_ADDR_LEN; i++)
        {
            memcpy(tmp, macstr + i * 2, 2);
            mac[i] = (uint8_t)strtol(tmp, NULL, 16);
        }
            
        SysMacAddrSet(mac);
        g_rfModuleDetected = true;
    }
    
}

//ATOK
//ATCi576176654D65736820414D522053746420434331313130204E203233333036000C0130464637354301BB119E9810010101000005050F00
static void atcmdRecvByte(uint8_t byte)
{
    if(g_rfBuffCount == 1)
    {
        if(byte != 'A')
        {
            g_rfBuffCount = 0;
        }
    }
    else if(g_rfBuffCount == 2)
    {
        if(byte != 'T')
        {
            g_rfBuffCount = 0;
        }
    }
    else
    {
        if(g_rfBuffCount >= sizeof(g_rfRecvBuff))
        {
            g_rfBuffCount = 0;
        }

        if(byte == '\r')
        {
            g_rfRecvBuff[g_rfBuffCount] = '\0';
            g_frameType = RFMODULE_FRAME_TYPE_AT;
            g_gotFrame = true;
        }
    }
}

static void frameParsePoll(void)
{
    if(g_gotFrame)
    {
        if(g_frameType == RFMODULE_FRAME_TYPE_DATA)
        {
            WirelessDataParse((char *)g_rfRecvBuff);
        }
        else
        {
            atcmdParse((char *)g_rfRecvBuff);
        }
        g_gotFrame = false;
        g_rfBuffCount = 0;
    }
}

static void uartDataRecv(uint8_t *data, uint16_t len)
{
    uint8_t i;
    for(i = 0; i < len; i++)
    {
        if(VTSQueueHasSpace(g_uartBuff))
        {
            VTSQueuePush(g_uartBuff, data[i]);
        }
    }
    
}
#if 1
static void uartDataHandle(void)
{
    uint8_t i;
    uint8_t data;
    static uint32_t recvTime;
    static bool atResponse = false;

    if(g_gotFrame)
    {
        return;
    }

    while(VTSQueueCount(g_uartBuff))
    {
        HalInterruptSet(false);
        data = VTSQueuePop(g_uartBuff);
        HalInterruptSet(true);
        if(SysTimeHasPast(recvTime, 100))
        {
            g_rfBuffCount = 0;
        }
        recvTime = SysTime();
        g_rfRecvBuff[g_rfBuffCount++] = data;

        if(g_rfBuffCount == 1)
        {
            if(data == '~')
            {
                atResponse = false;
            }
            else
            {
                atResponse = true;
            }
        }
        
        if(atResponse)
        {
            atcmdRecvByte(data);
        }
        else
        {
            dataRecvByte(data);
        }
    }
    
}
#endif

static void uartInit(void)
{
    HalUartConfig_t config;

    config.enble = true;
    config.baudrate = 115200;
    config.flowControl = 0;
    config.parity = 0;
    config.wordLength = USART_WordLength_8b;
    config.recvCb = uartDataRecv;
    HalUartConfig(RFMODULE_UART_PORT, &config);
}

static void moduleDetect(void)
{
    char *getId = "ATCi\r";
    static uint32_t getidTime;
    if(!g_rfModuleDetected && SysTimeHasPast(getidTime, 500))
    {
        Syslog("");
        HalUartWrite(RFMODULE_UART_PORT, (const uint8_t *)getId, strlen(getId));
        getidTime = SysTime();
    }
}

void RFModuleSetSleepInterval(uint16_t interval)
{
    g_setIntervalEnable = true;
    g_intervalNum = interval;
}

void RFModuleSendData(uint8_t *data, uint16_t len)
{
    Syslog("%s", data);
    HalUartWrite(RFMODULE_UART_PORT, (const unsigned char *)data, len);
}

bool RFModuleDetected(void)
{
    return g_rfModuleDetected;
}

void RFModuleInit(void)
{   
    uartInit();
}

void RFModulePoll(void)
{
    moduleDetect();
    uartDataHandle();
    frameParsePoll();
}

