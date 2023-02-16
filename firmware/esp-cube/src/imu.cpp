/***
 * @Author: Wanderer-web
 * @Date: 2022-05-14 10:49:05
 * @LastEditors: Wanderer-web
 * @LastEditTime: 2022-05-16 16:44:52
 * @FilePath: \esp-cube-arduino\src\imu.cpp
 * @Description: 姿态传感器管理
 */

#include "imu.h"
#include "math.h"

#define MATH_PI 3.141592653589793
#define IMU_I2C_SDA (27)
#define IMU_I2C_SCL (14)

#define DEBUG_RAW_PRINT (0)
#define DEBUG_DMP_PRINT (0)

float rawLocations[6][3] = {{0, 0, 1.0},
                            {-1.0, 0, 0},
                            {0, -1.0, 0},
                            {1, 0, 0},
                            {0, 1.0, 0},
                            {0, 0, -1.0}}; //六面中心原始位置（世界坐标系）

/***
 * @description: 判断面处于哪个位置
 * @param undefined
 * @return {*}
 */
static inline uint8_t judgeSidePos(float *location)
{
    uint8_t pos = 0;
    float MaxCosAngle = -999.0; //最大夹角余弦值（代表最小夹角）
    float tmpCosAngle;
    for (uint8_t i = 0; i < 6; i++)
    {
        tmpCosAngle = location[0] * rawLocations[i][0] +
                      location[1] * rawLocations[i][1] +
                      location[2] * rawLocations[i][2]; //点积，由于都是单位向量所以点积的值就是夹角余弦
        if (tmpCosAngle > MaxCosAngle)
        {
            MaxCosAngle = tmpCosAngle;
            pos = i;
        }
    }
    return pos;
}

/***
 * @description: 姿态传感器初始化
 * @return 是否初始化成功
 */
uint8_t IMU::init()
{
    Wire.begin(IMU_I2C_SDA, IMU_I2C_SCL);
    Wire.setClock(400000);
    while (!imu.testConnection())
    {
        Serial.print(".");
    }
    Serial.print("\n");

    // initialize device
    Serial.println("Initializing I2C devices...");
    imu.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(imu.testConnection() ? ("imu6050 connection successful") : ("imu6050 connection failed"));

    // load and configure the DMP
    Serial.println("Initializing DMP...");
    devStatus = imu.dmpInitialize();

    // make sure it worked (returns 0 if so)
    if (devStatus == 0)
    {
        // turn on the DMP, now that it's ready
        imu.CalibrateAccel(6);
        imu.CalibrateGyro(6);
        imu.PrintActiveOffsets();
        // turn on the DMP, now that it's ready
        Serial.println("Enabling DMP...");
        imu.setDMPEnabled(true);

        // get expected DMP packet size for later comparison
        packetSize = imu.dmpGetFIFOPacketSize();
        Serial.println("dmp init ok");
        dmpReady = true;
        imuEnabled = true;
        return 1;
    }
    else
    {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.printf("DMP Initialization failed %d\n", devStatus);
        return 0;
    }
}

/***
 * @description: 姿态传感器使能
 * @param {bool} en
 * @return {*}
 */
void IMU::enable(bool en)
{
    if (en && !imuEnabled)
    {
        vTaskDelay(300);
        imu.setSleepEnabled(false);
        imu.setDMPEnabled(true);
        imuEnabled = true;
    }
    else if (!en && imuEnabled)
    {
        imu.setDMPEnabled(false);
        imu.setSleepEnabled(true);
        imuEnabled = false;
    }
}

/***
 * @description:
 * @param undefined
 * @return {*}
 */
void IMU::rawUpdate()
{
    imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
#if DEBUG_RAW_PRINT
    Serial.print(ax);
    Serial.print("\t");
    Serial.print(ay);
    Serial.print("\t");
    Serial.print(az);
    Serial.print("\t");
    Serial.print(gx);
    Serial.print("\t");
    Serial.print(gy);
    Serial.print("\t");
    Serial.print(gz);
    Serial.print("\n");
#endif
}

void IMU::dmpUpdate()
{
    if (imu.dmpGetCurrentFIFOPacket(fifoBuffer)) // Get the Latest packet
    {
        imu.dmpGetQuaternion(&q, fifoBuffer);
        q0 = q.w;
        q1 = q.x;
        q2 = q.y;
        q3 = q.z;
        rotateMatrix[0][0] = 2 * (pow(q0, 2) + pow(q1, 2)) - 1;
        rotateMatrix[0][1] = 2 * (q1 * q2 + q0 * q3);
        rotateMatrix[0][2] = 2 * (q1 * q3 - q0 * q2);
        rotateMatrix[1][0] = 2 * (q1 * q2 - q0 * q3);
        rotateMatrix[1][1] = 2 * (pow(q0, 2) + pow(q2, 2)) - 1;
        rotateMatrix[1][2] = 2 * (q2 * q3 + q0 * q1);
        rotateMatrix[2][0] = 2 * (q0 * q2 + q1 * q3);
        rotateMatrix[2][1] = 2 * (q2 * q3 - q0 * q1);
        rotateMatrix[2][2] = 2 * (pow(q0, 2) + pow(q3, 2)) - 1;
        for (uint8_t i = 0; i < 6; i++)
        {
            for (uint8_t j = 0; j < 3; j++)
            {
                locations[i][j] = rotateMatrix[0][j] * rawLocations[i][0] +
                                  rotateMatrix[1][j] * rawLocations[i][1] +
                                  rotateMatrix[2][j] * rawLocations[i][2];
            }                                        //计算面中心坐标
            sideNow[judgeSidePos(locations[i])] = i; //计算面所处位置
        }

#if DEBUG_DMP_PRINT
        // Serial.print("quat\t");
        // Serial.print(q.w);
        // Serial.print("\t");
        // Serial.print(q.x);
        // Serial.print("\t");
        // Serial.print(q.y);
        // Serial.print("\t");
        // Serial.println(q.z);

        // Serial.print("location\t");
        // Serial.printf("%.2f %.2f %.2f | ", locations[0][0], locations[0][1], locations[0][2]);
        // Serial.printf("%.2f %.2f %.2f | ", locations[1][0], locations[1][1], locations[1][2]);
        // Serial.printf("%.2f %.2f %.2f\n", locations[2][0], locations[2][1], locations[2][2]);

        Serial.print("pos\t");
        Serial.printf("%d %d %d %d %d %d\n", sideNow[0], sideNow[1], sideNow[2], sideNow[3], sideNow[4], sideNow[5]);
#endif
    }
}

void IMU::getSideNow(uint8_t *sides)
{
    sides[0] = sideNow[0];
    sides[1] = sideNow[3];
    sides[2] = sideNow[4];
}

int16_t IMU::getAccelX()
{
    return ax;
}

int16_t IMU::getAccelY()
{
    return ay;
}

int16_t IMU::getAccelZ()
{
    return az;
}

int16_t IMU::getGyroX()
{
    return gx;
}

int16_t IMU::getGyroY()
{
    return gy;
}

int16_t IMU::getGyroZ()
{
    return gz;
}
