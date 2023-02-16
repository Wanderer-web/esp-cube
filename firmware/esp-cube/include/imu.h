#ifndef _IMU_H__
#define _IMU_H__

#include <Arduino.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

class IMU
{
private:
    MPU6050 imu;
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    bool imuEnabled = true;
    float rotateMatrix[3][3] = {{0}, {0}, {0}}; //旋转矩阵，左乘
    float locations[6][3] = {{0, 0, 1.0},
                             {-1.0, 0, 0},
                             {0, -1.0, 0},
                             {1, 0, 0},
                             {0, 1.0, 0},
                             {0, 0, -1.0}}; //六面中心点坐标（世界坐标系

    // MPU control/status vars

    uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
    uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
    uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
    uint16_t fifoCount;     // count of all bytes currently in FIFO
    uint8_t fifoBuffer[64]; // FIFO storage buffer

    // orientation/motion vars
    Quaternion q; // [w, x, y, z]         quaternion container
    float q0, q1, q2, q3;

public:
    bool dmpReady = false;                   // set true if DMP init was successful
    uint8_t sideNow[6] = {0, 1, 2, 3, 4, 5}; //六面当前对应的面（世界坐标系）
    uint8_t init();

    void enable(bool en);

    void rawUpdate();
    void dmpUpdate();

    void getSideNow(uint8_t *sides);

    int16_t getAccelX();
    int16_t getAccelY();
    int16_t getAccelZ();

    int16_t getGyroX();
    int16_t getGyroY();
    int16_t getGyroZ();
};

#endif