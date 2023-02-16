#ifndef _BLUETOOTH_H__
#define _BLUETOOTH_H__

#include <Arduino.h>
#include <NimBLEDevice.h>

class CubeBLE
{
private:
    BLEServer *pServer = NULL;
    BLECharacteristic *pBatCharacteristic, *pUsbCharacteristic;
    bool oldDeviceConnected = false;
    char txBuffer[128];

public:
    void init();
    bool isConnected();
    void checkState();
    void readVol(float bat, uint16_t usb);
};

#endif