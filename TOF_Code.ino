#include <M5StickCPlus.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define VL53L0X_REG_IDENTIFICATION_MODEL_ID         0xc0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID      0xc2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   0x50
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS         0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14

#define ToF_ADDR 0x29  // the iic address of tof

// The remote service we wish to connect to.
#define CHARACTERISTIC_UUID "17afd00e-ac19-449e-90f7-c11d689c4517"
// The characteristic of the remote service we are interested in.
#define SERVICE_UUID "e6cfac45-5796-4497-86cf-0faeb67c2b40"

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLECharacteristic *pCharacteristic;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

byte gbuf[16];

void setup() {
    // put your setup code here, to run once:
    Wire.begin(0, 26, 400000UL);  // join i2c bus (address optional for master)
    Serial.begin(115200);         // start serial for output
    Serial.println("VLX53LOX test started.");

    //---osmar
    M5.begin(true, true, true);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);

    BLEDevice::init("Proximity_M5");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE
                                        );

    pCharacteristic->setValue("Hello World says Proximity");
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    M5.Lcd.print("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
    // if(BLEDevice::getPeerDevices(true).size()==1){
    //     M5.Lcd.print("connected to 1 device");
    //     // continue statement
    //   }
    if (proximityFunc()<80){
      pCharacteristic->setValue("ALERT");
    }
    else{
      pCharacteristic->setValue("All good");
    }
}

int proximityFunc(){
    delay(100);
    return measure_distance();
}

int measure_distance() {
    write_byte_data_at(VL53L0X_REG_SYSRANGE_START, 0x01);

    read_block_data_at(VL53L0X_REG_RESULT_RANGE_STATUS,
                       12);  // read 12 bytes once

    uint16_t dist =
        makeuint16(gbuf[11], gbuf[10]);  // split distance data to "dist"
    byte DeviceRangeStatusInternal = ((gbuf[0] & 0x78) >> 3);

    Serial.print("distance ");
    Serial.println(dist);    

    M5.Lcd.setCursor(20, 80);
    M5.Lcd.fillRect(20, 80, 60, 25, BLACK);
    M5.Lcd.print(dist);
    return dist;
}

uint16_t bswap(byte b[]) {
    // Big Endian unsigned short to little endian unsigned short
    uint16_t val = ((b[0] << 8) & b[1]);
    return val;
}

uint16_t makeuint16(int lsb, int msb) {
    return ((msb & 0xFF) << 8) | (lsb & 0xFF);
}

uint16_t VL53L0X_decode_vcsel_period(short vcsel_period_reg) {
    // Converts the encoded VCSEL period register value into the real
    // period in PLL clocks
    uint16_t vcsel_period_pclks = (vcsel_period_reg + 1) << 1;
    return vcsel_period_pclks;
}

/*
 * IIC Functions
 */
/* function description: write one byte data */
void write_byte_data(byte data) {
    Wire.beginTransmission(ToF_ADDR);
    Wire.write(data);
    Wire.endTransmission();
}

/* function description: write one byte data to specifical register */
void write_byte_data_at(byte reg, byte data) {
    Wire.beginTransmission(ToF_ADDR);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

/* function description: read two bytes data to specifical register */
void write_word_data_at(byte reg, uint16_t data) {
    byte b0 = (data & 0xFF);
    byte b1 = ((data >> 8) && 0xFF);

    Wire.beginTransmission(ToF_ADDR);
    Wire.write(reg);
    Wire.write(b0);
    Wire.write(b1);
    Wire.endTransmission();
}

/* function description: read one byte data */
byte read_byte_data() {
    Wire.requestFrom(ToF_ADDR, 1);
    while (Wire.available() < 1) delay(1);
    byte b = Wire.read();
    return b;
}

/* function description: read one byte data from specifical register */
byte read_byte_data_at(byte reg) {
    // write_byte_data((byte)0x00);
    write_byte_data(reg);
    Wire.requestFrom(ToF_ADDR, 1);
    while (Wire.available() < 1) delay(1);
    byte b = Wire.read();
    return b;
}

/* function description: read two bytes data from specifical register */
uint16_t read_word_data_at(byte reg) {
    write_byte_data(reg);
    Wire.requestFrom(ToF_ADDR, 2);
    while (Wire.available() < 2) delay(1);
    gbuf[0] = Wire.read();
    gbuf[1] = Wire.read();
    return bswap(gbuf);
}

/* function description: read multiple bytes data from specifical register */
void read_block_data_at(byte reg, int sz) {
    int i = 0;
    write_byte_data(reg);
    Wire.requestFrom(ToF_ADDR, sz);
    for (i = 0; i < sz; i++) {
        while (Wire.available() < 1) delay(1);
        gbuf[i] = Wire.read();
    }
}