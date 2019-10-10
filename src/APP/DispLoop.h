#ifndef DISPLOOP_H
#define DISPLOOP_H

typedef enum
{
    DISPLOOP_ID_ADDR = 0, //485地址
    DISPLOOP_ID_RFCHN,    //无线通道
    DISPLOOP_ID_INTERVAL, //上报间隔
    DISPLOOP_ID_DEVTYPE,  //设备类型
    DISPLOOP_ID_ERRCODE,  //异常代码
    DISPLOOP_ID_COUNT,
}DispLoopID_t;

typedef struct
{
    unsigned char flag;
    unsigned char contents[3];
}DispLoopValue_t;

typedef DispLoopValue_t *(*DispLoopGetValue_cb)(DispLoopID_t id);
typedef void (*DispLoopDone_cb)(void);

void DispLoopRegister(DispLoopID_t id, DispLoopGetValue_cb getvalue);
void DispLoopStop(void);
void DispLoopStart(unsigned char num);
void DispLoopInit(DispLoopDone_cb loopDone);
void DispLoopPoll(void);

#endif

