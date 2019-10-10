#ifndef WIRELESS_H
#define WIRELESS_H

#include "Sys.h"

void WirelessReportData(uint8_t err, uint8_t power, uint16_t *data, uint16_t dataNum);
void WirelessSetChannel(uint8_t chn);
void WirelessSetInterval(uint16_t seconds);
void WirelessDataParse(char *data);
void WirelessInit(void);
void WirelessPoll(void);

#endif


