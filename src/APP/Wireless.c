#include "Wireless.h"
#include "RFModule.h"


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

        for(int i = 0; i < dataNum; i++)
        {
            devData[i] = string2num(&report->data[i*4], 4);
        }

        DeviceDataInsert(head->mac, errcode, devtype, power, devData, dataNum);
    }
}

void WirelessInit(void)
{
}

void WirelessPoll(void)
{
}

