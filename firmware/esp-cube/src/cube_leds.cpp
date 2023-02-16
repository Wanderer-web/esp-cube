/***
 * @Author: Wanderer-web
 * @Date: 2022-05-14 10:49:26
 * @LastEditors: Wanderer-web
 * @LastEditTime: 2022-05-16 15:14:59
 * @FilePath: \esp-cube-arduino\src\leds.cpp
 * @Description: 六面灯组管理
 */

#include "cube_leds.h"

#define WS2812_DATA_PIN (25)
#define BRIGHTNESS (8)

RgbColor Red(BRIGHTNESS, 0, 0);
RgbColor Green(0, BRIGHTNESS, 0);
RgbColor Blue(0, 0, BRIGHTNESS);
RgbColor Yellow(BRIGHTNESS, BRIGHTNESS, 0);
RgbColor Orange(BRIGHTNESS, BRIGHTNESS / 4, 0);
RgbColor White(BRIGHTNESS);
RgbColor Black(0);

const uint8_t neighborIdxTable[6][4] = {{1, 2, 3, 4},
                                        {4, 5, 2, 0},
                                        {1, 5, 3, 0},
                                        {2, 5, 4, 0},
                                        {3, 5, 1, 0},
                                        {1, 4, 3, 2}};

const uint8_t neighborPixelIdxTable[6][12] = {
    {2, 3, 8, 2, 3, 8, 2, 3, 8, 2, 3, 8},
    {8, 7, 6, 0, 1, 2, 0, 1, 2, 0, 1, 2},
    {8, 7, 6, 2, 3, 8, 0, 1, 2, 6, 5, 0},
    {8, 7, 6, 8, 7, 6, 0, 1, 2, 8, 7, 6},
    {8, 7, 6, 6, 5, 0, 0, 1, 2, 2, 3, 8},
    {6, 5, 0, 6, 5, 0, 6, 5, 0, 6, 5, 0},
};

const uint8_t oppositeSideTable[6] = {5, 3, 4, 1, 2, 0}; //每面对面的编号

const uint8_t sideCenterTable[6] = {0, 1, 2, 1, 2, 0}; //每面夹着的中间层编号

uint8_t centerSideIdxTable[3][4] = {
    {3, 4, 1, 2},
    {2, 5, 4, 0},
    {3, 5, 1, 0},
};

uint8_t centerPixelIdxTable[3][12] = {
    {1, 4, 7, 1, 4, 7, 1, 4, 7, 1, 4, 7},
    {3, 4, 5, 3, 4, 5, 5, 4, 3, 3, 4, 5},
    {3, 4, 5, 7, 4, 1, 5, 4, 3, 1, 4, 7},
};

/***
 * @description: 单个面灯组构造函数
 * @param {uint8_t} neighbor 该面相邻四面编号
 * @param {uint8_t} neighborPixel 该面相邻12个元素编号
 * @return {*}
 */
SideLeds::SideLeds(const uint8_t neighbor[4], const uint8_t neighborPixel[12])
{
    for (uint8_t i = 0; i < 4; i++)
    {
        neighborIndex[i] = neighbor[i];
    }
    for (uint8_t i = 0; i < 12; i++)
    {
        neighborPixelIndex[i] = neighborPixel[i];
    }
}

/***
 * @description: 单个面旋转
 * @param {uint8_t} direction 旋转方向
 * @param {uint8_t} step 步进次数，2次为90度旋转
 * @return {*}
 */
void SideLeds::rotate(uint8_t direction, uint8_t step)
{
    RgbColor tmpColors[9];
    for (uint8_t count = 0; count < step; count++)
    {
        for (uint8_t i = 0; i < 9; i++)
            tmpColors[i] = colors[i];
        if (direction == 0) //逆时针
        {
            colors[0] = tmpColors[1];
            colors[1] = tmpColors[2];
            colors[2] = tmpColors[3];
            colors[3] = tmpColors[8];
            // colors[4] = tmpColors[4];
            colors[5] = tmpColors[0];
            colors[6] = tmpColors[5];
            colors[7] = tmpColors[6];
            colors[8] = tmpColors[7];
        }
        else if (direction == 1) //顺时针
        {
            colors[0] = tmpColors[5];
            colors[1] = tmpColors[0];
            colors[2] = tmpColors[1];
            colors[3] = tmpColors[2];
            // colors[4] = tmpColors[4];
            colors[5] = tmpColors[6];
            colors[6] = tmpColors[7];
            colors[7] = tmpColors[8];
            colors[8] = tmpColors[3];
        }
    }
}

/***
 * @description: 单个面初始化颜色
 * @param {RgbColor} color 颜色
 * @return {*}
 */
void SideLeds::resetColor(RgbColor color)
{
    for (uint8_t i = 0; i < 9; i++)
    {
        colors[i] = color;
    }
}

/***
 * @description: 初始化
 * @return {*}
 */
void CubeLeds::init()
{
    sideLeds[0] = new SideLeds(neighborIdxTable[0], neighborPixelIdxTable[0]);
    sideLeds[1] = new SideLeds(neighborIdxTable[1], neighborPixelIdxTable[1]);
    sideLeds[2] = new SideLeds(neighborIdxTable[2], neighborPixelIdxTable[2]);
    sideLeds[3] = new SideLeds(neighborIdxTable[3], neighborPixelIdxTable[3]);
    sideLeds[4] = new SideLeds(neighborIdxTable[4], neighborPixelIdxTable[4]);
    sideLeds[5] = new SideLeds(neighborIdxTable[5], neighborPixelIdxTable[5]);
    leds = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(54, WS2812_DATA_PIN);
    leds->Begin();
    leds->Show();
}

/***
 * @description: 灯组使能
 * @param {bool} en
 * @return {*}
 */
void CubeLeds::enable(bool en)
{
    if (en && !cubeEnabled)
    {
        vTaskDelay(300);
        reset();
        cubeEnabled = true;
    }
    else if (!en && cubeEnabled)
    {
        for (uint8_t i = 0; i < 54; i++)
        {
            leds->SetPixelColor(i, Black); //灭掉所有灯
        }
        leds->Show();
        cubeEnabled = false;
    }
}

/***
 * @description: 显示电量
 * @param {float} bat 电量
 * @return {*}
 */
void CubeLeds::showBat(float bat)
{
    if (bat > 4.1)
    {
        leds->SetPixelColor(4, Green);
    }
    else if (bat > 3.8)
    {
        leds->SetPixelColor(4, Orange);
    }
    else
    {
        leds->SetPixelColor(4, Red);
    }
    leds->Show();
}

/***
 * @description: 随机打乱
 * @param {uint8_t} count 打乱次数
 * @return {*}
 */
void CubeLeds::randomShuffle(uint8_t count)
{
    uint8_t randomIdx, randomDirection;
    for (uint8_t i = 0; i < count; i++)
    {
        randomIdx = (act == Act_MES) ? random(3) : random(6);
        randomDirection = random(2);
        respToAct(randomIdx, randomDirection);
        vTaskDelay(100);
    }
}

/***
 * @description: 测试
 * @return {*}
 */
void CubeLeds::test()
{
    static uint8_t i = 0;
    if (i)
    {
        rotateOneSide(1, 0);
    }
    else
    {
        rotateOneSide(0, 1);
    }
    i = 1 - i;
}

/***
 * @description: 复位魔方图案
 * @return {*}
 */
void CubeLeds::reset()
{
    sideLeds[0]->resetColor(Red);
    sideLeds[1]->resetColor(Green);
    sideLeds[2]->resetColor(Yellow);
    sideLeds[3]->resetColor(Blue);
    sideLeds[4]->resetColor(White);
    sideLeds[5]->resetColor(Orange);
    cubeShow();
}

/***
 * @description: 显示正在dmp初始化中
 * @return {*}
 */
void CubeLeds::showDmpLoading()
{
    static uint8_t dot = 0;
    for (uint8_t i = 0; i < 54; i++)
    {
        leds->SetPixelColor(i, Black);
    }
    leds->SetPixelColor(dot, White);
    leds->SetPixelColor(dot + 1, Red);
    leds->Show();
    dot = (dot + 1) % 8;
}

/***
 * @description: 旋转选定面相邻四面靠近选定面的十二个元素
 * @param {SideLeds} *side 选定面
 * @param {uint8_t} direction 旋转方向
 * @param {uint8_t} step 步进次数，3次为90度旋转
 * @return {*}
 */
void CubeLeds::rotateSideNeighbor(SideLeds *side, uint8_t direction, uint8_t step)
{
    RgbColor tmpColors[12];
    uint8_t *neighborIndex = side->neighborIndex;
    uint8_t *neighborPixelIndex = side->neighborPixelIndex;
    for (uint8_t count = 0; count < step; count++)
    {
        for (uint8_t i = 0; i < 12; i++)
        {
            tmpColors[i] = sideLeds[neighborIndex[i / 3]]->colors[neighborPixelIndex[i]]; //复制
        }
        if (direction == 0) //逆时针
        {
            for (uint8_t i = 0; i < 12; i++)
            {
                sideLeds[neighborIndex[i / 3]]->colors[neighborPixelIndex[i]] = tmpColors[((i == 0) ? (11) : (i - 1))];
            }
        }
        else if (direction == 1) //顺时针
        {
            for (uint8_t i = 0; i < 12; i++)
            {
                sideLeds[neighborIndex[i / 3]]->colors[neighborPixelIndex[i]] = tmpColors[((i == 11) ? (0) : (i + 1))];
            }
        }
    }
}

/***
 * @description:
 * @param {uint8_t} layerIdx
 * @param {uint8_t} direction
 * @param {uint8_t} step
 * @return {*}
 */
void CubeLeds::rotateCenterLayer(uint8_t layerIdx, uint8_t direction, uint8_t step)
{
    RgbColor tmpColors[12];
    uint8_t *sideIndex = centerSideIdxTable[layerIdx];
    uint8_t *PixelIndex = centerPixelIdxTable[layerIdx];
    for (uint8_t count = 0; count < step; count++)
    {
        for (uint8_t i = 0; i < 12; i++)
        {
            tmpColors[i] = sideLeds[sideIndex[i / 3]]->colors[PixelIndex[i]]; //复制
        }
        if (direction == 0) //逆时针
        {
            for (uint8_t i = 0; i < 12; i++)
            {
                sideLeds[sideIndex[i / 3]]->colors[PixelIndex[i]] = tmpColors[((i == 0) ? (11) : (i - 1))];
            }
        }
        else if (direction == 1) //顺时针
        {
            for (uint8_t i = 0; i < 12; i++)
            {
                sideLeds[sideIndex[i / 3]]->colors[PixelIndex[i]] = tmpColors[((i == 11) ? (0) : (i + 1))];
            }
        }
    }
}

/***
 * @description: 根据操作改变魔方图案，会阻塞当前进程
 * @param {uint8_t} side 操作面编号
 * @param {uint8_t} direction 旋转方向
 * @return {*}
 */
void CubeLeds::rotateOneSide(uint8_t side, uint8_t direction)
{
    SideLeds *selectSide = sideLeds[side];
    vTaskDelay(100);
    rotateSideNeighbor(selectSide, direction, 1);
    cubeShow();
    vTaskDelay(50);
    selectSide->rotate(direction, 1);
    cubeShow();
    vTaskDelay(50);
    rotateSideNeighbor(selectSide, direction, 1);
    cubeShow();
    vTaskDelay(100);
    selectSide->rotate(direction, 1);
    rotateSideNeighbor(selectSide, direction, 1);
    cubeShow();
}

/***
 * @description: 根据操作改变魔方图案，会阻塞当前进程
 * @param {uint8_t} side 操作面编号
 * @param {uint8_t} direction 旋转方向
 * @return {*}
 */
void CubeLeds::respToAct(uint8_t side, uint8_t direction)
{
    if (act == Act_RUF)
    {
        rotateOneSide(side, direction);
    }
    else if (act == Act_MES)
    {
        if (side != 0 && side != 3 && side != 4)
            direction = !direction;
        vTaskDelay(100);
        rotateCenterLayer(sideCenterTable[side], direction, 1);
        cubeShow();
        vTaskDelay(100);
        rotateCenterLayer(sideCenterTable[side], direction, 1);
        cubeShow();
        vTaskDelay(100);
        rotateCenterLayer(sideCenterTable[side], direction, 1);
        cubeShow();
    }
    else if (act == Act_LDB)
    {
        rotateOneSide(oppositeSideTable[side], !direction);
    }
}

/***
 * @description: 提示操作面
 * @param {uint8_t} side
 * @return {*}
 */
void CubeLeds::showHint(uint8_t side1, uint8_t side2, uint8_t side3)
{
    uint8_t hintIdx1[12];
    uint8_t hintIdx2[12];
    uint8_t hintIdx3[12];
    if (act == Act_MES)
    {
        uint8_t *sideIndex = centerSideIdxTable[side1];
        uint8_t *PixelIndex = centerPixelIdxTable[side1];
        for (uint8_t i = 0; i < 12; i++)
        {
            hintIdx1[i] = sideIndex[i / 3] * 9 + PixelIndex[i]; //复制
        }
        sideIndex = centerSideIdxTable[side2];
        PixelIndex = centerPixelIdxTable[side2];
        for (uint8_t i = 0; i < 12; i++)
        {
            hintIdx2[i] = sideIndex[i / 3] * 9 + PixelIndex[i]; //复制
        }
        sideIndex = centerSideIdxTable[side3];
        PixelIndex = centerPixelIdxTable[side3];
        for (uint8_t i = 0; i < 12; i++)
        {
            hintIdx3[i] = sideIndex[i / 3] * 9 + PixelIndex[i]; //复制
        }
    }
    else
    {
        uint8_t *neighborIndex = sideLeds[side1]->neighborIndex;
        uint8_t *neighborPixelIndex = sideLeds[side1]->neighborPixelIndex;
        for (uint8_t i = 0; i < 12; i++)
        {
            hintIdx1[i] = neighborIndex[i / 3] * 9 + neighborPixelIndex[i];
        }
        neighborIndex = sideLeds[side2]->neighborIndex;
        neighborPixelIndex = sideLeds[side2]->neighborPixelIndex;
        for (uint8_t i = 0; i < 12; i++)
        {
            hintIdx2[i] = neighborIndex[i / 3] * 9 + neighborPixelIndex[i];
        }
        neighborIndex = sideLeds[side3]->neighborIndex;
        neighborPixelIndex = sideLeds[side3]->neighborPixelIndex;
        for (uint8_t i = 0; i < 12; i++)
        {
            hintIdx3[i] = neighborIndex[i / 3] * 9 + neighborPixelIndex[i];
        }
    }
    for (uint8_t count = 0; count < 3; count++) // 3次闪烁
    {
        for (uint8_t i = 0; i < 12; i++)
        {
            leds->SetPixelColor(hintIdx1[i], Black);
            leds->SetPixelColor(hintIdx2[i], Black);
            leds->SetPixelColor(hintIdx3[i], Black);
        }
        leds->Show();
        vTaskDelay(50);
        cubeShow();
        vTaskDelay(50);
    }
}

/***
 * @description: 显示魔方图案
 * @return {*}
 */
void CubeLeds::cubeShow()
{
    RgbColor *tmpColors;
    for (uint8_t side = 0; side < 6; side++)
    {
        tmpColors = sideLeds[side]->colors;
        for (uint8_t pixel = 0; pixel < 9; pixel++)
        {
            leds->SetPixelColor(side * 9 + pixel, tmpColors[pixel]);
        }
    }
    leds->Show();
}