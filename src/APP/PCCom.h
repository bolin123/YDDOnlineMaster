#ifndef PCCOM_H
#define PCCOM_H

typedef struct
{
    unsigned char head[2];
    unsigned char addr;
    unsigned char cmd;
    unsigned char length;
    unsigned char data[];
}PCComProto_t;

typedef enum
{
    PCCOM_EVENT_DATA_REPORT,
    PCCOM_EVENT_TIMING,
    PCCOM_EVENT_SLEEP_INTERVAL,
    PCCOM_EVENT_REBOOT,
}PCComEvent_t;

typedef void (*PCComEventHandle_t)(PCComEvent_t event, void *args);

void PCComDataReport(unsigned char *data, unsigned short len);
void PCComInit(PCComEventHandle_t eventHandle);
void PCComPoll(void);

#endif
