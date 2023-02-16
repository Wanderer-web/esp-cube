/***
 * @Author: Wanderer-web
 * @Date: 2022-05-17 19:21:59
 * @LastEditors: Wanderer-web
 * @LastEditTime: 2022-05-17 19:33:23
 * @FilePath: \esp-cube-arduino\src\buzzer.cpp
 * @Description: 无源蜂鸣器驱动
 */

#include "buzzer.h"

#define BUZZER_PIN (19)
#define CHANNEL (0)
#define RESOLUTION (8)

uint16_t toneTable[3][7] = {{277, 311, 340, 370, 415, 466, 524},
                            {554, 622, 682, 740, 831, 932, 1046},
                            {1109, 1245, 1356, 1480, 1661, 1865, 2066}};

void Buzzer::init()
{
    ledcSetup(CHANNEL, 277, RESOLUTION);
    ledcWrite(CHANNEL, 128);
}

/***
 * @description: 蜂鸣器使能
 * @param {bool} en
 * @return {*}
 */
void Buzzer::enable(bool en)
{
    if (en)
    {
        ledcAttachPin(BUZZER_PIN, CHANNEL);
    }
    else
    {
        ledcDetachPin(BUZZER_PIN);
        digitalWrite(BUZZER_PIN, HIGH);
    }
}

/***
 * @description: 修改蜂鸣器音调
 * @param {uint8_t} tone 音阶
 * @param {uint8_t} level 高多少个八度
 * @return {*}
 */
void Buzzer::setTone(uint8_t tone, uint8_t level)
{
    ledcWriteTone(CHANNEL, toneTable[level][tone]);
}

void Buzzer::test()
{
    enable(true);
    ledcWriteTone(CHANNEL, toneTable[1][0]);
    vTaskDelay(300);
    ledcWriteTone(CHANNEL, toneTable[1][0]);
    vTaskDelay(300);
    ledcWriteTone(CHANNEL, toneTable[1][4]);
    vTaskDelay(300);
    ledcWriteTone(CHANNEL, toneTable[1][4]);
    vTaskDelay(300);
    ledcWriteTone(CHANNEL, toneTable[1][5]);
    vTaskDelay(300);
    ledcWriteTone(CHANNEL, toneTable[1][5]);
    vTaskDelay(300);
    ledcWriteTone(CHANNEL, toneTable[1][4]);
    vTaskDelay(300);
    enable(false);
}