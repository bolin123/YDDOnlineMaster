#include "DeviceData.h"

static DeviceDataReport_t g_reportData;
static uint16_t g_dataCount = 0;

void DeviceDataInsert(char *mac, uint8_t devType, uint8_t err, uint8_t power, uint16_t *data, uint16_t dnum)
{
    DeviceDataReport_t *reportData = (DeviceDataReport_t *)malloc(sizeof(DeviceDataReport_t));

    if(reportData)
    {
        memcpy(reportData->mac, mac, SYS_MAC_ADDR_LEN);
        reportData->devType = devType;
        reportData->utctime = HalRTCGetUtc();
        reportData->errcode = err;
        reportData->power = power;
        reportData->dataNum = dnum;
        reportData->data = (uint16_t *)malloc(dnum * 2);
        if(reportData->data)
        {
            memcpy(reportData->data, data, dnum * 2);
            VTListAdd(&g_reportData, reportData);
        }
        else
        {
            free(reportData);
        }
    }
    
}

DeviceDataReport_t *DeviceDataGetHead(void)
{
    return &g_reportData;
}

void DeviceDataInit(void)
{
    VTListInit(&g_reportData);
}

void DeviceDataPoll(void)
{
}

