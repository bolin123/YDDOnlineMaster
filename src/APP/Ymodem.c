#include "YModem.h"

#define YMODEM_FRAME_SOH 0x01 //128字节数据帧，协议类型
#define YMODEM_FRAME_EOT 0x04 //结束传输，发送者发送
#define YMODEM_FRAME_ACK 0x06 //接收者处理成功回应，发送者发现下一包数据
#define YMODEM_FRAME_NAK 0x15 //接收者处理失败回应，发送者需要重发
#define YMODEM_FRAME_CA  0x18 //传输中止
#define YMODEM_FRAME_C   0x43 //接收者准备接收时会发连续的 C，发送者接收到 C开始发送

typedef enum
{
    YMODEM_STATUS_NONE,
    YMODEM_STATUS_START,
    YMODEM_STATUS_TRANSFER,
    YMODEM_STATUS_FINISH,
}YModemStatus_t;

typedef struct
{
    uint8_t flag;
    uint8_t frameID;
    uint8_t complement;
    uint8_t data[128];
    uint8_t crc16[2];
}YModemFrame_t;

static YModemEvent_cb g_ymodemEvent;
static bool g_sendSync = false;
static uint8_t g_dataBuff[sizeof(YModemFrame_t)];
static uint8_t g_frameID = 0;
static volatile bool g_gotframe = false;
static YModemStatus_t g_status = YMODEM_STATUS_NONE;

static uint16_t crc16(uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint16_t value;
    uint16_t stat;
    uint8_t * in_ptr; 
    
    //指向要计算CRC的缓冲区开头
    in_ptr = data;
    value = 0;
    for (stat = len ; stat > 0; stat--) //len是所要计算的长度
    {
        value = value^(uint16_t)(*in_ptr++) << 8;
        for (i=8; i!=0; i--) 
        {
            if (value & 0x8000)
            {
                value = value << 1 ^ 0x1021;
            } 
            else 
            {
                value = value << 1;
            }
        }
    }
    return value;
}


static void modemDataSend(uint8_t *data, uint16_t len)
{
}

static void modemDataRecv(uint8_t *data, uint16_t len)
{
    uint16_t i;
    YModemFrame_t *frame = (YModemFrame_t *)g_dataBuff;
    static uint8_t buffcount = 0;
    static uint32_t oldTime;

    for(i = 0; i < len; i++)
    {
        if(SysTimeHasPast(oldTime, 200))
        {
            buffcount = 0;
        }
        oldTime = SysTime();
        g_dataBuff[buffcount++] = data[i];

        if(buffcount == 1)
        {
            if(data[i] != YMODEM_FRAME_SOH)
            {
                buffcount = 0;
            }
            if(data[i] == YMODEM_FRAME_EOT)
            {
                g_gotframe = true;
            }
        }
        else if(buffcount == 2)
        {
            if(g_frameID != data[i])
            {
                buffcount = 0;
            }
        }
        else if(buffcount == 3)
        {
            if(data[i] != ~frame->frameID)
            {
                buffcount = 0;
            }
        }
        else
        {
            if(buffcount == sizeof(YModemFrame_t))
            {
                uint16_t crc = frame->crc16[0];
                crc = (crc << 8) + frame->crc16[1];
                if(crc16(frame->data, sizeof(frame->data)) == crc)
                {
                    g_frameID++;
                    g_gotframe = true;
                }
                buffcount = 0;
            }
        }
    }
}

static void modemFrameParse(void)
{
    YModemFrame_t *frame;
    if(g_gotframe)
    {
        frame = (YModemFrame_t *)g_dataBuff;
        switch (g_status)
        {
        case YMODEM_STATUS_NONE:
        break;
        case YMODEM_STATUS_START:
        break;
        case YMODEM_STATUS_TRANSFER:
        break;
        case YMODEM_STATUS_FINISH:
        break;
        default:
        break;
        }
        
        g_gotframe = false;
    }
}

static void syncPoll(void)
{
    static uint32_t lastTime;

    if(g_sendSync && SysTimeHasPast(lastTime, 500))
    {
        modemDataSend('C', 1);
        lastTime = SysTime();
    }
}

void YModemStartSync(void)
{
    g_sendSync = true;
}

static void modemUartInit(void)
{
}

void YModemInit(YModemEvent_cb eventcb)
{
    g_ymodemEvent = eventcb;
    modemUartInit();
}

void YModemPoll(void)
{
}


