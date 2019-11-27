#include "Master.h"
#include "PCCom.h"
#include "Sys.h"
#include "DigitalLED.h"
#include "IR.h"
#include "DispLoop.h"
#include "Wireless.h"
#include "WiredProto.h"
#include "Menu.h"
#include "DeviceData.h"
#include "DataManager.h"

typedef struct
{
    uint8_t packId;
    uint8_t remain;
    uint8_t data[];
}PCReportData_t;

static uint32_t g_dataReportTime = 0;

static uint8_t deviceDataToBuff(DeviceDataReport_t *devData, uint8_t *buff)
{
    uint8_t i = 0;

    buff[i++] = devData->addr;
    buff[i++] = devData->devType;
    buff[i++] = devData->errcode;
    buff[i++] = devData->power;
    memcpy(&buff[i], &devData->utctime, 4);
    i += 4;
    memcpy(&buff[i], devData->data, devData->dataNum * 2);
    i += devData->dataNum * 2;
    return i;
}

static void pcDataReport(void)
{
    uint16_t dlen = 0;
    uint8_t packetnum = 1;
    DeviceDataReport_t *node;
    uint8_t buff[255 - sizeof(PCComProto_t) - 1];
    DeviceDataReport_t *dataHead = DeviceDataGetHead();
    PCReportData_t *reportData = (PCReportData_t *)buff;

    if(dataHead && VTListFirst(dataHead))
    {
        VTListForeach(dataHead, node)
        {
            if(DEVICE_DATA_PACKET_LEN(node->dataNum * 2) + dlen + sizeof(PCReportData_t) > sizeof(buff))
            {
                reportData->packId = packetnum++;
                reportData->remain = 1;
                PCComDataReport(buff, dlen + sizeof(PCReportData_t));
                dlen = 0;
                HalWdgFeed();
            }
            else
            {
                dlen += deviceDataToBuff(node, &reportData->data[dlen]);
            #if 0
                reportData->data[dlen++] = node->addr;
                reportData->data[dlen++] = node->devType;
                reportData->data[dlen++] = node->errcode;
                reportData->data[dlen++] = node->power;
                memcpy(&reportData->data[dlen], &node->utctime, 4);
                dlen += 4;
                memcpy(&reportData->data[dlen], node->data, node->dataNum * 2);
                dlen += node->dataNum * 2;
            #endif
                VTListDel(node);
                free(node->data);
                free(node);
            }
        }

        if(dlen) //last packet
        {
            reportData->packId = packetnum++;
            reportData->remain = 0;
            if(DataManagerGetStoragedLength())
            {
                reportData->remain = 0x80;
            }
            PCComDataReport(buff, dlen + sizeof(PCReportData_t));
            dlen = 0;
        }

    }
    else
    {
        reportData->packId = packetnum++;
        reportData->remain = 0;
        if(DataManagerGetStoragedLength())
        {
            reportData->remain = 0x80;
        }
        PCComDataReport(buff, sizeof(PCReportData_t));
    }
}

static void historyReport(void)
{
    uint8_t buff[255 - sizeof(PCComProto_t) - 1] = {0};
    uint8_t maxPacketlen, packetlen;
    uint32_t lastLength = DataManagerGetStoragedLength();
    
    if(lastLength)
    {
        packetlen = sizeof(buff) - 4;
        maxPacketlen = packetlen - (packetlen % HAL_DEVICE_DATA_PACKET_LENGTH);
        if(lastLength > maxPacketlen)
        {
            lastLength -= maxPacketlen;
            memcpy(buff, &lastLength, 4);
            DataManagerLoad(buff + 4, maxPacketlen);
            PCComHistoryDataReport(buff, maxPacketlen + 4);
        }
        else
        {
            DataManagerLoad(buff + 4, lastLength);
            PCComHistoryDataReport(buff, lastLength + 4);
        }
    }
    else
    {
        PCComHistoryDataReport(buff, 4);
    }
}

static void pcEventHandle(PCComEvent_t event, void *args)
{
    switch(event)
    {
        case PCCOM_EVENT_DATA_REPORT:
            if(((uint32_t)args - HalRTCGetUtc()) > 300)
            {
                HalRTCSetUtc((uint32_t)args);
            }
            pcDataReport();
            g_dataReportTime = SysTime();
            break;
        case PCCOM_EVENT_TIMING:
            HalRTCSetUtc((uint32_t)args);
            break;
        case PCCOM_EVENT_SLEEP_INTERVAL:
            WirelessSetInterval((uint16_t)(uint32_t)args);
            SysReportIntervalSet((uint16_t)(uint32_t)args);
            break;
        case PCCOM_EVENT_REBOOT:
            SysReboot();
            break;
        case PCCOM_EVENT_HISTORY_REPORT:
            historyReport();
            break;
        default:
            break;
    }
}

static void deviceDataStoragePoll(void)
{
    uint8_t *buff = NULL;
    uint16_t bufflen = 0;
    uint16_t buffoffset = 0;
    DeviceDataReport_t *lastRecord, *node;

    if(SysTimeHasPast(g_dataReportTime, SysReportIntervalGet() * 1000 * 3)) //3倍时间未收到查询消息，则设备状态为未连接，开始存储数据
    {
        lastRecord = VTListLast(DeviceDataGetHead());
        if(lastRecord  && (HalRTCGetUtc() - lastRecord->utctime) > 5) //最后一个包超过5秒等数据稳定后再存储，，减少存储次数，延长e2prom使用寿命
        {
            VTListForeach(DeviceDataGetHead(), node)
            {
                if(bufflen == 0)
                {
                    bufflen = DEVICE_DATA_PACKET_LEN(node->dataNum * 2);
                    buff = (uint8_t *)malloc(bufflen);
                }
                else
                {
                    bufflen += DEVICE_DATA_PACKET_LEN(node->dataNum * 2);
                    buff = realloc(buff, bufflen);
                }
                
                if(buff)
                {
                    deviceDataToBuff(node, buff + buffoffset);
                    buffoffset = bufflen;
                }
                //释放历史数据
                VTListDel(node);
                if(node->data)
                {
                    free(node->data);
                }
                free(node);
            }
            DataManagerStorage(buff, bufflen);
            free(buff);
            bufflen = 0;
        }
    }
}

static void wiredDataQuery(void)
{
    static uint32_t oldTime = 0;

    if(SysTimeHasPast(oldTime, SysReportIntervalGet() * 1000))
    {
        WiredProtoQuery();
        oldTime = SysTime();
    }
}

static void disploopDoneHandle(void)
{
    Syslog("");
    DigitalLEDOff();
}

static DispLoopValue_t *disploopGetValue(DispLoopID_t id)
{
    static DispLoopValue_t displayValue;
    DigitalLEDFlag_t flag;
    uint16_t value;
    
    switch (id)
    {
    case DISPLOOP_ID_ADDR:
        flag = DIGITAL_FLAG_A;
        value = SysCommAddressGet();
        break;
    case DISPLOOP_ID_RFCHN:
        flag = DIGITAL_FLAG_C;
        value = SysRfChannelGet();
        break;
    case DISPLOOP_ID_INTERVAL:
        flag = DIGITAL_FLAG_S;
        value = SysReportIntervalGet();
        break;
    case DISPLOOP_ID_DEVTYPE:
        flag = DIGITAL_FLAG_D;
        value = 0;
        break;
    case DISPLOOP_ID_ERRCODE:
        flag = DIGITAL_FLAG_E;
        value = SysErrcodeGet();
        break;
    default:
        return NULL;
    }

    displayValue.flag = flag;
    displayValue.contents[0] = value / 100;
    displayValue.contents[1] = value % 100 / 10;
    displayValue.contents[2] = value % 10;
    return &displayValue;
}

static unsigned short menuGetValue(MenuID_t id)
{
    uint16_t value;
    
    switch (id)
    {
    case MENU_ID_ADDR:
        value = SysCommAddressGet();
    break;
    case MENU_ID_RFCHN:
        value = SysRfChannelGet();
    break;
    case MENU_ID_INTERVAL:
        value = SysReportIntervalGet();
    break;
    default:
    break;
    }

    return value;
}

static void menuSetValue(MenuID_t id, uint16_t value)
{   
    Syslog("id = %d, value = %d", id, value);
    switch (id)
    {
    case MENU_ID_ADDR:
        SysCommAddressSet((uint8_t)value);
    break;
    case MENU_ID_RFCHN:
        SysRfChannelSet((uint8_t)value);
        WirelessSetChannel((uint8_t)value);
    break;
    case MENU_ID_INTERVAL:
        WirelessSetInterval(value);
        SysReportIntervalSet(value);
    break;
    default:
    break;
    }
}

static void menuEventHandle(MenuEvent_t event)
{   
    Syslog("%d", event);
    switch (event)
    {
    case MENU_EVENT_ACTIVE:
        DigitalLEDOn();
    break;
    case MENU_EVENT_DEACTIVE:
    //break;
    case MENU_EVENT_TIMEOUT:
        MenuDeactive();
        DigitalLEDOff();
    break;
    }
}

void MasterLightActive(void)
{
    DigitalLEDOn();
    DispLoopStart(2);
}

static void displayConfig(void)
{
    DigitalLEDInit();
    
    DispLoopInit(disploopDoneHandle);
    DispLoopRegister(DISPLOOP_ID_ADDR,     disploopGetValue);
    DispLoopRegister(DISPLOOP_ID_RFCHN,    disploopGetValue);
    DispLoopRegister(DISPLOOP_ID_INTERVAL, disploopGetValue);
    //DispLoopRegister(DISPLOOP_ID_DEVTYPE,  disploopGetValue);
    //DispLoopRegister(DISPLOOP_ID_ERRCODE,  disploopGetValue);

    MenuInit(menuEventHandle);
    MenuItem_t item;
    //MENU_ID_ADDR
    item.flag     = DIGITAL_FLAG_A;
    item.digitNum = DIGITAL_LED_ID_1;
    item.step     = 1;
    item.max      = 254;
    item.min      = 1;
    item.getValue = menuGetValue;
    item.setValue = menuSetValue;
    MenuRegister(MENU_ID_ADDR, &item);

    //MENU_ID_RFCHN
    item.flag     = DIGITAL_FLAG_C;
    item.digitNum = DIGITAL_LED_ID_2;
    item.step     = 1;
    item.max      = HAL_RF_CHANNEL_NUM;
    item.min      = 1;
    item.getValue = menuGetValue;
    item.setValue = menuSetValue;
    MenuRegister(MENU_ID_RFCHN, &item);

    //MENU_ID_INTERVAL
    item.flag     = DIGITAL_FLAG_S;
    item.digitNum = DIGITAL_LED_ID_1;
    item.step     = 1;
    item.max      = 999;
    item.min      = 10;
    item.getValue = menuGetValue;
    item.setValue = menuSetValue;
    MenuRegister(MENU_ID_INTERVAL, &item);
    
    DigitalLEDOn();
    DispLoopStart(2);
}

static void irKeyEventHandle(IRKey_t key)
{
    Syslog("key = %d", key);
    DispLoopStop();
    MenuKeyHandle(key);
}

static void ledBlinkPoll(void)
{
    static uint32_t oldTime;
    static uint8_t level = 0;

    if(SysTimeHasPast(oldTime, 500))
    {
        level = !level;
        HalGPIOSetLevel(HAL_STATUS_LED_PIN, level);
        oldTime = SysTime();
    }
}

void MasterInit(void)
{   
    PCComInit(pcEventHandle);
    WirelessInit();
    WiredProtoInit();
    IRInit(irKeyEventHandle);
    DataManagerInit();
    displayConfig();
    //WirelessSetChannel(SysRfChannelGet());
    //WirelessSetInterval(SysReportIntervalGet());
}

void MasterPoll(void)
{
    PCComPoll();
    DigitalLEDPoll();
    WirelessPoll();
    IRPoll();
    wiredDataQuery();
    WiredProtoPoll();
    MenuPoll();
    ledBlinkPoll();
    DispLoopPoll();
    DataManagerPoll();
    deviceDataStoragePoll();
}

