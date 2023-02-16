#include "Arduino.h"
#include "imu.h"
#include "cube_leds.h"
#include "buzzer.h"
#include "key.h"
#include "bluetooth.h"

IMU mpu;
CubeLeds cube;
Buzzer buzzer;
Key key;
CubeBLE cubeBLE;

int16_t gx, gy, gz;
int16_t _gx, _gy, _gz;
uint8_t sides[3];
float batVol = 0;
uint16_t usbAd = 0;
bool isUsbIn = false; //是否接入USB
String bleRecv;

#define BATTERY_PIN (4)
#define USB_PIN (13)

/***
 * @description: 蓝牙通信任务
 * @param {void} *pvParameters
 * @return {*}
 */
void BluetoothTask(void *pvParameters)
{
  while (1)
  {
    cubeBLE.checkState();
    vTaskDelay(500);
    if (isUsbIn)
      cubeBLE.readVol(batVol, usbAd);
  }
}

/***
 * @description: 姿态解算任务
 * @param {void} *pvParameters
 * @return {*}
 */
void PoseSolveTask(void *pvParameters)
{
  while (1)
  {
    vTaskDelay(25); //每25毫秒计算一次姿态
    if (!isUsbIn)   //充电时不计算姿态
    {
      if (!mpu.dmpReady)
      {
        if (!mpu.init()) //初始化姿态传感器
        {
          Serial.println("mpu failed to init!");
          while (1)
            ;
        }
        cube.reset(); //初始完后复位魔方图案
      }
      mpu.enable(true); //姿态传感器打开
      mpu.rawUpdate();  //获取原始六轴数据
      mpu.dmpUpdate();  // dmp解算姿态
    }
    else
    {
      mpu.enable(false); //姿态传感器进入低功耗模式
    }
  }
}

/***
 * @description: 按键扫描任务
 * @param {void} *pvParameters
 * @return {*}
 */
void KeyScanTask(void *pvParameters)
{
  while (1)
  {
    vTaskDelay(5);                //每5毫秒扫描一次按键
    if (!isUsbIn && mpu.dmpReady) //充电或姿态传感器初始化时不扫描按键
    {
      key.scan();
      if (key.event == Event_CubeReset)
      {
        mpu.dmpReady = false;
        cube.act = Act_RUF;
      }
      else if (key.event == Event_ChangeAct)
      {
        cube.act = (ActionTypesEnum)((cube.act + 1) % 3);
        if (cube.act == Act_RUF)
        {
          cube.showHint(mpu.sideNow[0], mpu.sideNow[3], mpu.sideNow[4]);
        }
        else if (cube.act == Act_LDB)
        {
          cube.showHint(mpu.sideNow[1], mpu.sideNow[2], mpu.sideNow[5]);
        }
        else if (cube.act == Act_MES)
        {
          cube.showHint(0, 1, 2);
        }
      }
      else if (key.event == Event_CubeShuffle)
      {
        buzzer.setTone(6, 1);
        buzzer.enable(true);
        vTaskDelay(100);
        buzzer.enable(false);
        vTaskDelay(100);
        buzzer.setTone(6, 1);
        buzzer.enable(true);
        vTaskDelay(100);
        buzzer.enable(false);
        cube.randomShuffle(20);
        buzzer.setTone(6, 1);
        buzzer.enable(true);
        vTaskDelay(100);
        buzzer.enable(false);
      }
    }
  }
}

void setup()
{
  vTaskDelay(300); //等待上电完成

  randomSeed(2333); //初始化随机数发生器

  Serial.begin(115200); //初始化串口

  cubeBLE.init();
  pinMode(USB_PIN, INPUT);     //初始化USB检测引脚
  pinMode(BATTERY_PIN, INPUT); //初始化电源电压测量引脚
  key.init();                  //初始化按键
  buzzer.init();               //初始化蜂鸣器
  cube.init();                 //初始化灯组

  xTaskCreatePinnedToCore(BluetoothTask, "Bluetooth", 1024 * 5, NULL, 3, NULL, 0);  //启动蓝牙通信线程
  xTaskCreatePinnedToCore(KeyScanTask, "KeyScan", 1024 * 5, NULL, 1, NULL, 1);      //启动按键扫描线程
  xTaskCreatePinnedToCore(PoseSolveTask, "PoseSolve", 1024 * 20, NULL, 3, NULL, 1); //启动姿态解算线程
}

void loop()
{
  batVol = analogRead(BATTERY_PIN) / 4095.0 * 2.0 * 3.3;
  usbAd = analogRead(USB_PIN);
  if (usbAd == 4095 || usbAd == 2047)
    return;
  isUsbIn = (usbAd / 4095.0 * 3.0 * 3.3 > 4.0) ? true : false; //优先检测USB电源是否插入
  vTaskDelay(50);
  if (isUsbIn)
  {
    cube.enable(false); //灯组关闭
    cube.showBat(batVol);
    // Serial.printf("BAT : %fV\tUSB : %fV\n", batVol, usbVol);
    vTaskDelay(450);
    return;
  }
  else
  {
    cube.enable(true); //灯组打开
  }

  //再检测姿态传感器是否初始化成功
  if (!mpu.dmpReady)
  {
    cube.showDmpLoading();
    vTaskDelay(150);
    return;
  }

  mpu.getSideNow(sides); //获取右手操作面
  gx = mpu.getGyroX();   //获取x轴角速度
  gy = mpu.getGyroY();   //获取y轴角速度
  gz = mpu.getGyroZ();   //获取z轴角速度
  if (abs(gx) > 4000 || abs(gy) > 4000 || abs(gz) > 4000)
  {
    for (uint8_t i = 0; i < 12; i++)
    {
      vTaskDelay(25);
      _gx = mpu.getGyroX();
      _gy = mpu.getGyroY();
      _gz = mpu.getGyroZ();
      if (abs(gx) > 4000 && _gx * gx < 0 && abs(_gx) > 500)
      {
        if (sides[0] == 1 || sides[1] == 1 || sides[2] == 1)
        {
          buzzer.setTone(0, gx > 0);
          buzzer.enable(true);
          cube.respToAct(1, gx > 0);
          buzzer.enable(false);
          vTaskDelay(100);
          break;
        }
        else if (sides[0] == 3 || sides[1] == 3 || sides[2] == 3)
        {
          buzzer.setTone(1, gx < 0);
          buzzer.enable(true);
          cube.respToAct(3, gx < 0);
          buzzer.enable(false);
          vTaskDelay(100);
          break;
        }
      }
      if (abs(gy) > 4000 && _gy * gy < 0 && abs(_gy) > 500)
      {
        if (sides[0] == 2 || sides[1] == 2 || sides[2] == 2)
        {
          buzzer.setTone(2, gy > 0);
          buzzer.enable(true);
          cube.respToAct(2, gy > 0);
          buzzer.enable(false);
          vTaskDelay(100);
          break;
        }
        else if (sides[0] == 4 || sides[1] == 4 || sides[2] == 4)
        {
          buzzer.setTone(3, gy < 0);
          buzzer.enable(true);
          cube.respToAct(4, gy < 0);
          buzzer.enable(false);
          vTaskDelay(100);
          break;
        }
      }
      if (abs(gz) > 4000 && _gz * gz < 0 && abs(_gz) > 500)
      {
        if (sides[0] == 0 || sides[1] == 0 || sides[2] == 0)
        {
          buzzer.setTone(4, gz < 0);
          buzzer.enable(true);
          cube.respToAct(0, gz < 0);
          buzzer.enable(false);
          vTaskDelay(100);
          break;
        }
        else if (sides[0] == 5 || sides[1] == 5 || sides[2] == 5)
        {
          buzzer.setTone(5, gz > 0);
          buzzer.enable(true);
          cube.respToAct(5, gz > 0);
          buzzer.enable(false);
          vTaskDelay(100);
          break;
        }
      }
    }
  }
}