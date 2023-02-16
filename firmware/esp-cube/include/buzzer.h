#ifndef _BUZZER_H__
#define _BUZZER_H__

#include <Arduino.h>

class Buzzer
{
private:
public:
    void init();
    void enable(bool en);
    void setTone(uint8_t tone, uint8_t level);
    void test();
};

#endif