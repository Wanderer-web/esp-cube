#ifndef _CUBE_LEDS_H__
#define _CUBE_LEDS_H__

#include <Arduino.h>
#include "NeoPixelBus.h"

typedef enum
{
    Act_RUF = 0,
    Act_MES = 1,
    Act_LDB = 2,

} ActionTypesEnum; //操作类别

class SideLeds
{
private:
public:
    uint8_t neighborIndex[4];
    uint8_t neighborPixelIndex[12];
    RgbColor colors[9];
    SideLeds(const uint8_t neighbor[4], const uint8_t neighborPixel[12]);
    void rotate(uint8_t direction, uint8_t step);
    void resetColor(RgbColor color);
};

class CubeLeds
{
private:
    SideLeds *sideLeds[6];
    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> *leds;
    bool cubeEnabled = true;
    void cubeShow();
    void rotateSideNeighbor(SideLeds *side, uint8_t direction, uint8_t step);

public:
    ActionTypesEnum act = Act_RUF;
    void init();
    void test();
    void reset();
    void showDmpLoading();
    void enable(bool en);
    void showBat(float bat);
    void randomShuffle(uint8_t count);
    void showHint(uint8_t side1, uint8_t side2, uint8_t side3);
    void rotateCenterLayer(uint8_t layerIdx, uint8_t direction, uint8_t step);
    void rotateOneSide(uint8_t side, uint8_t direction);
    void respToAct(uint8_t side, uint8_t direction);
};

#endif