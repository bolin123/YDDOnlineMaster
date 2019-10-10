#include "Sys.h"
#include "W25Q64.h"
#include "Master.h"

#define SYS_FIRMWARE_VERSION "1.0.0.1"
#define SYS_POWER_ADC_NUM 10

typedef struct
{
    uint8_t commAddr;  //上位机通信地址
    uint8_t rfChannel; //无线通道
    uint16_t interval; //上报间隔,秒
}SysConfigs_t;

static uint8_t g_errcode = 0;
static SysConfigs_t g_sysConfigs;

uint16_t SysReportIntervalGet(void)
{
    return g_sysConfigs.interval;
}

void SysReportIntervalSet(uint16_t interval)
{
    g_sysConfigs.interval = interval;
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (uint8_t *)&g_sysConfigs, sizeof(SysConfigs_t));
}

uint8_t SysErrcodeGet(void)
{
    return g_errcode;
}

uint8_t SysCommAddressGet(void)
{
    return g_sysConfigs.commAddr;
}

void SysCommAddressSet(uint8_t addr)
{
    g_sysConfigs.commAddr = addr;
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (uint8_t *)&g_sysConfigs, sizeof(SysConfigs_t));
}

void SysRfChannelSet(uint8_t chn)
{
    g_sysConfigs.rfChannel = chn;
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (uint8_t *)&g_sysConfigs, sizeof(SysConfigs_t));
}

uint8_t SysRfChannelGet(void)
{
    return g_sysConfigs.rfChannel;
}

int SysDateTimeSet(SysDateTime_t *dateTime)
{
    return HalRTCSetTime((HalRTCTime_t *)dateTime);
}
#if 0

void SysArgsGetRecord(SysDataRecord_t *record)
{
    if(record != NULL)
    {
        HalFlashRead(HAL_DATA_RECORD_ADDR, (uint8_t *)record, sizeof(SysDataRecord_t));
        if(record->num == 0xffff || record->size == 0xffffffff)
        {
            record->num = 0;
            record->size = 0;
            HalFlashWrite(HAL_DATA_RECORD_ADDR, (const uint8_t *)record, sizeof(SysDataRecord_t));
        }
    }
}

void SysArgsSetRecord(SysDataRecord_t *record)
{
    if(record != NULL)
    {
        Syslog("num = %d, size = %d", record->num, record->size);
        HalFlashWrite(HAL_DATA_RECORD_ADDR, (const uint8_t *)record, sizeof(SysDataRecord_t));
    }
}

//id start 1,2,3...
void SysArgsSetPointInfo(uint16_t id, SysDataInfo_t *info)
{
    uint32_t addr;
    //HAL_DATA_POINT_INFO_ADDR
    if(info != NULL)
    {
        Syslog("id = %d, threshold = %d, times = %d, size = %d, address = %d", id, info->threshold, 
                                                                    info->times, info->size, info->startAddr);
        addr = HAL_DATA_POINT_INFO_ADDR + ((id - 1) * sizeof(SysDataInfo_t));
        HalFlashWrite(addr, (const uint8_t *)info, sizeof(SysDataInfo_t));
    }
}

void SysArgsGetPointInfo(uint16_t id, SysDataInfo_t *info)
{
    uint32_t addr;
    //HAL_DATA_POINT_INFO_ADDR
    if(info != NULL)
    {
        addr = HAL_DATA_POINT_INFO_ADDR + ((id - 1) * sizeof(SysDataInfo_t));
        HalFlashRead(addr, (uint8_t *)info, sizeof(SysDataInfo_t));
    }
}

void SysSignalThresholdSet(uint16_t value)
{
    //soft step 50, from 50~300
    uint16_t daValue = (value / 50) * HAL_DAC_STEP_VALUE + HAL_DAC_BASE_VALUE;
    HalDACSetValue(daValue);
}

void SysBeepEnable(bool enable)
{
//    HalBeepEnable(enable);
}

void SysArgsClear(void)
{
    SysDataRecord_t record;

    Syslog("");
    record.num = 0;
    record.size = 0;
    SysArgsSetRecord(&record);
}

void SysCollectArgsGet(SysCollectArgs_t *args)
{
    if(args)
    {
        HalFlashRead(HAL_DEVICE_ARGS_ADDR, (uint8_t *)args, sizeof(SysCollectArgs_t));
        if(args->signalThreshold == 0xffff ||
            args->runTime == 0xffff ||
            args->intensityAlarm == 0xffff ||
            args->ringAlarm == 0xffff)
        {
            args->beep = 1;
            args->brightness = 50;
            args->signalThreshold = 150;
            args->runTime = 120;
            args->intensityAlarm = 500;
            args->ringAlarm = 200;
            HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)args, sizeof(SysCollectArgs_t));
        }
    }
}

void SysCollectArgsSet(SysCollectArgs_t *args)
{
    if(args)
    {
        Syslog("threshold = %d, runTime = %d, alarm = %d, ring = %d", args->signalThreshold, args->runTime, 
                                                                args->intensityAlarm, args->ringAlarm);
        HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)args, sizeof(SysCollectArgs_t));
    }
}

void SysRawDataRead(uint32_t addresss, uint8_t *buff, uint16_t length)
{
    if(addresss != HAL_FLASH_INVALID_ADDR)
    {
        W25Q64Read(buff, addresss, length);
    }
}

void SysRawDataWrite(uint32_t addresss, uint8_t *buff, uint16_t length)
{
    if(addresss != HAL_FLASH_INVALID_ADDR)
    {
        W25Q64Write(buff, addresss, length);
    }
}
#endif

static void startupInit(void)
{
    HalFlashRead(HAL_DEVICE_ARGS_ADDR, (uint8_t *)&g_sysConfigs, sizeof(SysConfigs_t));
    if(g_sysConfigs.commAddr == 0xff)
    {
        g_sysConfigs.commAddr  = 0x01;
        g_sysConfigs.rfChannel = 0x01;
        g_sysConfigs.interval = 60; //1minute
        HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (uint8_t *)&g_sysConfigs, sizeof(SysConfigs_t));
    }
    //log
    printf("\r\n-----------------------------------------------------------\r\n");
    printf("--Firmware version:%s\r\n", SYS_FIRMWARE_VERSION);
    printf("--Compile date:%s %s\r\n", __DATE__, __TIME__);
    
    printf("--Comm Address:%d\r\n", g_sysConfigs.commAddr);
    printf("--Rf channel:%d\r\n", g_sysConfigs.rfChannel);
    printf("--Report interval:%ds\r\n", g_sysConfigs.interval);
    SysDateTime_t *time = SysDateTime();
    printf("--Now: %d-%02d-%02d %02d:%02d:%02d\r\n", time->year, time->month, time->day, 
                                          time->hour, time->minute, time->second);
    printf("-----------------------------------------------------------\r\n");
}

void SysReboot(void)
{
    Syslog("");
    HalCommonReboot();
}

void SysInitalize(void)
{
    g_errcode = HalCommonInitialize();
    printf(".....Hardware init....\n");
    g_errcode |= W25Q64Initialize();
    printf("errcode = %d\r\n", g_errcode);
    startupInit();
    MasterInit();
}

void SysPoll(void)
{
    HalCommonPoll();
    MasterPoll();
}

