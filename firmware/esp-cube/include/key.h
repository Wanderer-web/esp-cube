#ifndef _KEY_H__
#define _KEY_H__

#include <Arduino.h>

typedef enum
{
    Event_None = 0,
    Event_ChangeAct = 1,   //切换操作面
    Event_CubeShuffle = 2, //打乱图案
    Event_CubeReset = 3,   //复位图案并重新进行姿态校准
} KeyEventEnum;

class Key
{
private:
    uint16_t longKey = 0;   //长按记录值
    uint8_t pressCount = 0; //连按次数
public:
    KeyEventEnum event = Event_None;
    void init();
    void scan();
};

#endif