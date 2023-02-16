#include "bluetooth.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "27f37b11-2364-4edb-bdc6-7aaccb2bb496"
#define CHARACTERISTIC_UUID_RX "27f37b12-2364-4edb-bdc6-7aaccb2bb496"
#define CHARACTERISTIC_UUID_BAT "27f37b13-2364-4edb-bdc6-7aaccb2bb496"
#define CHARACTERISTIC_UUID_USB "27f37b14-2364-4edb-bdc6-7aaccb2bb496"

bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
    /***************** New - Security handled here ********************
    ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        Serial.println("Server PassKeyRequest");
        return 123456;
    }

    bool onConfirmPIN(uint32_t pass_key)
    {
        Serial.print("The passkey YES/NO number: ");
        Serial.println(pass_key);
        return true;
    }

    void onAuthenticationComplete(ble_gap_conn_desc desc)
    {
        Serial.println("Starting BLE work!");
    }
    /*******************************************************************/
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            Serial.println("*********");
            Serial.print("Received Value: ");
            for (int i = 0; i < rxValue.length(); i++)
                Serial.print(rxValue[i]);

            Serial.println();
            Serial.println("*********");
        }
    }
};

void CubeBLE::init()
{
    // Create the BLE Device
    BLEDevice::init("esp-cube");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pBatCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_BAT,
        /******* Enum Type NIMBLE_PROPERTY now *******
            BLECharacteristic::PROPERTY_NOTIFY
            );
        **********************************************/
        NIMBLE_PROPERTY::READ);

    pUsbCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_USB,
        /******* Enum Type NIMBLE_PROPERTY now *******
            BLECharacteristic::PROPERTY_NOTIFY
            );
        **********************************************/
        NIMBLE_PROPERTY::READ);

    /***************************************************
     NOTE: DO NOT create a 2902 descriptor
     it will be created automatically if notifications
     or indications are enabled on a characteristic.

     pCharacteristic->addDescriptor(new BLE2902());
    ****************************************************/

    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        /******* Enum Type NIMBLE_PROPERTY now *******
                BLECharacteristic::PROPERTY_WRITE
                );
        *********************************************/
        NIMBLE_PROPERTY::WRITE);

    pRxCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("Waiting connection");
}

bool CubeBLE::isConnected()
{
    return deviceConnected;
}

void CubeBLE::checkState()
{
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        vTaskDelay(500);             // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}

void CubeBLE::readVol(float bat, uint16_t usb)
{
    sprintf(txBuffer, "BAT : %.4fV", bat);
    pBatCharacteristic->setValue((uint8_t *)txBuffer, strlen(txBuffer));
    sprintf(txBuffer, "USB : %d", usb);
    pUsbCharacteristic->setValue((uint8_t *)txBuffer, strlen(txBuffer));
}