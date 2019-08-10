#include "Master.h"
#include "PCCom.h"
#include "Sys.h"
#include "DigitalLED.h"


void MasterInit(void)
{   
    DigitalLEDInit();
    PCComInit();
    uint8_t addr = SysGetComAddr();
    DigitalLEDSetChars(DIGITAL_LED_ID_CMD, 0x0a, false);
    DigitalLEDSetChars(DIGITAL_LED_ID_1, addr / 100, false);
    DigitalLEDSetChars(DIGITAL_LED_ID_2, addr % 100 / 10, false);
    DigitalLEDSetChars(DIGITAL_LED_ID_3, addr % 100 % 10, false);
    DigitalLEDOn();
}

void MasterPoll(void)
{
    PCComPoll();
    DigitalLEDPoll();
}

