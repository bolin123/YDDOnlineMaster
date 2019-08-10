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

typedef struct
{
    //uint16_t id;      //�����,��1��ʼ
    uint16_t threshold; //��ֵ
    uint16_t times;     //����ʱ��
    SysDateTime_t date; //��ʼ����
    uint16_t size;      //�ֽ���
    uint32_t startAddr;
}SysDataInfo_t;

typedef struct
{
    uint16_t num;   //������
    uint32_t size;  //�ܳ���
}SysDataRecord_t;

//���ݲɼ�����
typedef struct
{
    uint8_t beep; //����������
    uint8_t brightness;
    uint16_t signalThreshold; //�ź���ֵ 0~300, step 50, default 150
    uint16_t runTime;         //����ʱ�� 20S��60S��120S��240S��480S��600S��800S��999S, default 120
    uint16_t intensityAlarm;  //ǿ�ȱ���ֵ, 500~1500, step 500, default 500
    uint16_t ringAlarm;       //���屨��ֵ��5K��20K��50K��80K��100K��120K��150K��180K��200K��default 200k
}SysCollectArgs_t; 

//SysDateTime_t *SysDateTime(void);
int SysDateTimeSet(SysDateTime_t *dateTime);
//uint8_t SysPowerPercent(void);
void SysReboot(void);
uint8_t SysGetComAddr(void);

void SysArgsGetRecord(SysDataRecord_t *record);
void SysArgsSetRecord(SysDataRecord_t *record);
void SysArgsGetPointInfo(uint16_t id, SysDataInfo_t *info);
void SysArgsSetPointInfo(uint16_t id, SysDataInfo_t *info);
void SysArgsClear(void);
//void SysBeepEnable(bool enable);
//void SysSignalThresholdSet(uint16_t value);

uint8_t SysErrorCode(void);
uint8_t *SysMacAddrGet(void);
void SysMacAddrSet(uint8_t *mac);

void SysCollectArgsGet(SysCollectArgs_t *args);
void SysCollectArgsSet(SysCollectArgs_t *args);

void SysRawDataRead(uint32_t address, uint8_t *buff, uint16_t length);
void SysRawDataWrite(uint32_t addresss, uint8_t *buff, uint16_t length);

void SysInitalize(void);
void SysPoll(void);

#endif

