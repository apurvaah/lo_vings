#include <M5StickCPlus.h>
#include "BLEDevice.h"
#include "esp32-hal-ledc.h"
#include <BLEUtils.h>
#include <BLEServer.h>

#define COUNT_LOW   1500
#define COUNT_HIGH  8500
#define TIMER_WIDTH 16

// The remote service we wish to create
#define CHARACTERISTIC_UUID "71f41d28-a26a-48e1-8dc1-eba2ffd0143f"
// The characteristic of the remote service we are creating.
#define SERVICE_UUID "cdcfcdb1-02a5-4870-8c53-c9f261b865a3"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("b952a134-a33b-4a39-bc07-cd88c1bf3ef0");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("8ac406f9-584e-435f-b0dc-613e83c84361");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLECharacteristic *pCharacteristic;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static boolean doConnect_1 = false;
static boolean connected_1 = false;
static boolean doScan_1 = false;
static BLECharacteristic *pCharacteristic_1;
static BLERemoteCharacteristic* pRemoteCharacteristic_1;
static BLEAdvertisedDevice* myDevice_1;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    M5.Lcd.print("Notify callback for characteristic ");
    M5.Lcd.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    M5.Lcd.print(" of data length ");
    M5.Lcd.print(length);
    M5.Lcd.print("data: ");
    M5.Lcd.print((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    M5.Lcd.print("onDisconnect");
  }
};

bool connectToServer() {
    M5.Lcd.print("Forming a connection to ");
    M5.Lcd.print(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    M5.Lcd.print(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    M5.Lcd.print(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      M5.Lcd.print("Failed to find our service UUID: ");
      M5.Lcd.print(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    M5.Lcd.print(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      M5.Lcd.print("Failed to find our characteristic UUID: ");
      M5.Lcd.print(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    M5.Lcd.print(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      M5.Lcd.print("The characteristic value was: ");
      M5.Lcd.print(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      M5.Lcd.print("BLE Advertised Device found: ");
      M5.Lcd.print(advertisedDevice.toString().c_str());

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }
  }
}; // MyAdvertisedDeviceCallbacks

void setup() {
  M5.begin();
  BLEDevice::init("Servo_M5");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  M5.Lcd.print(doConnect);
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic_1 = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE
                                        );

  pCharacteristic_1->setValue("Hello World says Servo");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  M5.Lcd.print("Characteristic defined! Now you can read it in your phone!");

  ledcSetup(1, 50, TIMER_WIDTH);
  ledcAttachPin(26, 1);
}

// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    M5.Lcd.print(doConnect);
    if (connectToServer()) {
      M5.Lcd.print("We are now connected to the BLE Server.");
    } else {
      M5.Lcd.print("We have failed to connect to the server; there is nothing more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(25, 80, 4);
      M5.Lcd.print(value.c_str());
      if(value =="ALERT"){
        pCharacteristic_1->setValue("ALERT");
        for (int i = COUNT_LOW; i < COUNT_HIGH; i = i + 100) {
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(25, 80, 4);
          M5.Lcd.print("SERVO");
          ledcWrite(1, i);
          delay(50);
        }
      }
      else{
        pCharacteristic_1->setValue("All good");
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(25, 80, 4);
        M5.Lcd.print("No Alert");
      }
    }
    String newValue = "Time since boot: " + String(millis()/1000);
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }
} // End of loop