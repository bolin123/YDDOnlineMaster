#ifndef RFMODULE_H
#define RFMODULE_H

#include "Sys.h"

void RFModuleSetSleepInterval(uint16_t interval);
void RFModuleSendData(uint8_t *data, uint16_t len);
void RFModuleSleep(void);
void RFModuleWakeup(void);
bool RFModuleDetected(void);
void RFModuleInit(void);
void RFModulePoll(void);
#endif

