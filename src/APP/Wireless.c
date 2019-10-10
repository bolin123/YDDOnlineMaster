#include "Wireless.h"
#include "RFModule.h"
#include "DeviceData.h"

#define WIRELESS_WAIT_TIMES 0xE803D007 //设置外设开机时间1000ms+响应超时2000ms

//引导码~(1) + 地址(3) + 消息指令(2) + 长度(4) + 设备类型(2) + Err(2) + power(3) +  value1(n) + value2(n) +...+
typedef enum
{
    WIRELESS_DEVICE_TYPE_GEO_NOISE = 0, //地音地磁
    WIRELESS_DEVICE_TYPE_PRESS,     //应力
}WirelessDeviceType_t;

typedef enum
{
    WIRELESS_CMD_REPORT = 1,
}WirelessCmd_t;

typedef struct
{
    char preamble;
    char mac[SYS_MAC_ADDR_LEN];
    char cmd[2];
    char length[4];
    char data[];
}WirelessHead;

typedef struct
{
    char devType[2];
    char err[2];
    char power[3];
    char data[];
}WirelessReport_t;

static uint16_t string2num(char *str, uint8_t len)
{
    char temp[8] = "";

    memcpy(temp, str, len);
    return (uint16_t)strtol(temp, NULL, 10);
}

void WirelessDataParse(char *data)
{
    WirelessHead *head = (WirelessHead *)data;
    WirelessReport_t *report;
    uint16_t length;
    uint16_t devData[16];
    
    Syslog("%s", data);
    length = string2num(head->length, sizeof(head->length));
    uint16_t dataNum = (length - sizeof(WirelessReport_t)) / 4;//一个data长度为4字节
    if((head->cmd[1] - '0') == WIRELESS_CMD_REPORT)
    {    
        report = (WirelessReport_t *)head->data;
        uint8_t errcode = string2num(report->err, sizeof(report->err));
        uint8_t devtype = string2num(report->devType, sizeof(report->devType));
        uint8_t power = string2num(report->power, sizeof(report->power));

        printf("err:%d, type:%d, power:%d\n", errcode, devtype, power);

        for(int i = 0; i < dataNum; i++)
        {
            devData[i] = string2num(&report->data[i*4], 4);
            printf("%d ", devData[i]);
        }
        printf("\n");

        DeviceDataInsert(head->mac, devtype, errcode, power, devData, dataNum);
    }
}

void WirelessSetChannel(uint8_t chn)
{
    RFMoudleSetChannel(chn);
}

void WirelessSetInterval(uint16_t seconds)
{
    RFModuleSetReportInterval(seconds);
}

static void rfEventHandle(RFModuleEvent_t event, void *arg)
{
    uint8_t chnl;
    uint16_t interval;
    uint32_t times;
    if(event == RFMODULE_EVENT_GET_RFCHNL)
    {
        chnl = (uint8_t)(uint32_t)arg;
        Syslog("got chnl = %d", chnl);
        if(chnl != SysRfChannelGet())
        {
            RFMoudleSetChannel(SysRfChannelGet());
        }
    }
    else if(event == RFMODULE_EVENT_GET_INTERVAL)
    {
        interval = (uint16_t)(uint32_t)arg;
        
        Syslog("got interval = %d", interval);
        if(SysReportIntervalGet() != interval)
        {
            RFModuleSetReportInterval(SysReportIntervalGet());
        }
    }
    else //RFMODULE_EVENT_GET_WAITTIME
    {
        times = (uint32_t)arg;
        Syslog("got wait times = %04x", times);
        if(times != WIRELESS_WAIT_TIMES)
        {
            RFModuleSetWaitTimes(WIRELESS_WAIT_TIMES);
        }
    }
}


void WirelessInit(void)
{
    RFModuleInit(rfEventHandle);
    RFModuleGetChannel();
    RFModuleGetReportInterval();
    RFModuleGetWaitTimes();
}

void WirelessPoll(void)
{
    RFModulePoll();
}

