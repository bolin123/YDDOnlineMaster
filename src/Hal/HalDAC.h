#ifndef HAL_DAC_H
#define HAL_DAC_H

#include "HalCtype.h"

void HalDACEnable(bool enable);
void HalDACSetValue(uint16_t value);
void HalDACInitialize(void);
void HalDACPoll(void);

#endif
