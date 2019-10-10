#ifndef MENU_H
#define MENU_H

#include "IR.h"

typedef enum
{
    MENU_ID_ADDR = 0,
    MENU_ID_RFCHN,
    MENU_ID_INTERVAL,
    //MENU_ID_THRESHOLD,
    MENU_ID_COUNT,
}MenuID_t;

typedef enum
{
    MENU_EVENT_ACTIVE,
    MENU_EVENT_DEACTIVE,
    MENU_EVENT_TIMEOUT,
}MenuEvent_t;

typedef unsigned short (*MenuGetValue_cb)(MenuID_t id);
typedef void (*MenuSetValue_cb)(MenuID_t id, unsigned short value);
typedef void (*MenuEventHandle_cb)(MenuEvent_t event);

typedef struct
{
    char enable;
    char flag;     //��ʾ��־
    char digitNum; //λ��
    char step;     //������
    unsigned short max;     //���ֵ
    unsigned short min;     //��Сֵ
    MenuGetValue_cb getValue;
    MenuSetValue_cb setValue;
}MenuItem_t;

void MenuDeactive(void);
void MenuKeyHandle(IRKey_t key);
void MenuRegister(MenuID_t id, MenuItem_t *item);
void MenuInit(MenuEventHandle_cb eventHandle);
void MenuPoll(void);
#endif
