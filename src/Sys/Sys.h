#ifndef SYS_H
#define SYS_H

#include "HalCommon.h"

#define SYS_TIME_COUNT_1S 1000 //1s
#define SysTime() HalGetSysTimeCount()
#define SysTimeHasPast(oldTime, past) (SysTime() - (oldTime) > past)

#define SysDateTime_t HalRTCTime_t
#define SysDateTime() HalRTCGetTime()

#define Syslog(...) do{printf("[%s]: ", __FUNCTION__); printf(__VA_ARGS__); printf("\r\n");}while(0)

#define SYS_MAC_ADDR_LEN 3


void SysReboot(void);
uint8_t SysErrcodeGet(void);

uint8_t SysRfChannelGet(void);
uint8_t SysCommAddressGet(void);
uint16_t SysReportIntervalGet(void);

void SysRfChannelSet(uint8_t chn);
void SysCommAddressSet(uint8_t addr);
void SysReportIntervalSet(uint16_t interval);

int SysDateTimeSet(SysDateTime_t *dateTime);

void SysRawDataRead(uint32_t address, uint8_t *buff, uint16_t length);
void SysRawDataWrite(uint32_t addresss, uint8_t *buff, uint16_t length);

void SysInitalize(void);
void SysPoll(void);

#endif

