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
    PCCOM_EVENT_HISTORY_REPORT,
}PCComEvent_t;

typedef void (*PCComEventHandle_t)(PCComEvent_t event, void *args);

//char PCComConnected(void);
void PCComDataReport(unsigned char *data, unsigned short len);
void PCComHistoryDataReport(unsigned char *data, unsigned short len);
void PCComInit(PCComEventHandle_t eventHandle);
void PCComPoll(void);

#endif
