/***
 * @Author: Wanderer-web
 * @Date: 2022-05-17 19:21:51
 * @LastEditors: Wanderer-web
 * @LastEditTime: 2022-05-17 19:31:15
 * @FilePath: \esp-cube-arduino\src\key.cpp
 * @Description: 按键检测驱动
 */

#include "key.h"

#define KEY_PIN (21)

void Key::init()
{
    pinMode(KEY_PIN, INPUT_PULLUP);
}

void Key::scan()
{
    if (!digitalRead(KEY_PIN))
    {
        longKey++;
    }
    else
    {
        if (longKey >= 200) //长按1s
        {
            event = Event_CubeReset;
            pressCount = 0;
        }
        else if (longKey >= 40) //中按200ms
        {
            pressCount++;
            if (pressCount >= 2) //两次中按
            {
                event = Event_CubeShuffle;
                pressCount = 0;
            }
        }
        else if (longKey >= 10) //短按50ms
        {

            event = Event_ChangeAct;
            pressCount = 0;
        }
        else
        {
            event = Event_None;
        }
        longKey = 0;
    }
}