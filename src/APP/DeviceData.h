#ifndef DEVICEDATA_H
#define DEVICEDATA_H

#include "VTList.h"
#include "Sys.h"

#define DEVICE_DATA_PACKET_LEN(dlen) (SYS_MAC_ADDR_LEN + 7 + (dlen))

typedef struct DeviceDataReport_st
{
    char mac[SYS_MAC_ADDR_LEN];
    uint8_t devType;
    uint32_t utctime;
    uint8_t errcode;
    uint8_t power;
    uint8_t dataNum;
    uint16_t *data;
    VTLIST_ENTRY(struct DeviceDataReport_st);
}DeviceDataReport_t;

void DeviceDataInsert(char *mac, uint8_t devType, uint8_t err, uint8_t power, uint16_t *data, uint16_t dnum);
DeviceDataReport_t *DeviceDataGetHead(void);
void DeviceDataInit(void);
void DeviceDataPoll(void);
#endif


