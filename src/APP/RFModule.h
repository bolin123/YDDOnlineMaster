#ifndef RFMODULE_H
#define RFMODULE_H

#include "Sys.h"

typedef enum
{
    RFMODULE_EVENT_GET_RFCHNL,
    RFMODULE_EVENT_GET_INTERVAL,
    RFMODULE_EVENT_GET_WAITTIME,
}RFModuleEvent_t;

typedef void (*RFModuleEvent_cb)(RFModuleEvent_t event, void *arg);

void RFModuleSendData(uint8_t *data, uint16_t len);
bool RFModuleDetected(void);
void RFMoudleSetChannel(uint8_t chn);
void RFModuleGetChannel(void);
void RFModuleGetReportInterval(void);
void RFModuleSetReportInterval(uint16_t second);
void RFModuleGetWaitTimes(void);
void RFModuleSetWaitTimes(unsigned int time);

void RFModuleInit(RFModuleEvent_cb eventcb);
void RFModulePoll(void);
#endif

