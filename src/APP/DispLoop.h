#ifndef DISPLOOP_H
#define DISPLOOP_H

typedef enum
{
    DISPLOOP_ID_ADDR = 0, //485��ַ
    DISPLOOP_ID_RFCHN,    //����ͨ��
    DISPLOOP_ID_INTERVAL, //�ϱ����
    DISPLOOP_ID_DEVTYPE,  //�豸����
    DISPLOOP_ID_ERRCODE,  //�쳣����
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

