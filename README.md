# lo_vings
Lo-VINGS: Low Visibility Navigation System: 3 m5Stick C Plus devices connected through BLE

Areas with dense fog conditions and  high-speed vehicular movement  experience a lot of accidents due to low visibility, a solution to reduce the number of accidents is Lo-VINGS: Low Visibility Navigation System. The PoC is implemented using 3 m5Stick C Plus devices, one attached to a proximity (TOF) sensor which would be installed on the car, it connects to the servo mounted device through Bluetooth Low Energy (BLE), which in turn is connected to the speaker mounted device through BLE as well. One of the device which is attached to the servo motor for PoC, should be used to connect and control the braking system and the last one just alerts the driver by playing a sound on the speaker (separate hat recommended but not required).

## References
BLE Connections: https://electropeak.com/learn/esp32-bluetooth-low-energy-ble-on-arduino-ide-tutorial/ <br>
Proximity Sensor code: https://github.com/m5stack/M5-ProductExampleCodes/blob/master/Hat/tof-hat/Arduino/ToF/ToF.ino <br>
Servo Hat Code: https://github.com/m5stack/M5StickC-Plus/tree/master/examples/Hat/SERVO <br>
Speaker Hat code: https://github.com/m5stack/M5StickC-Plus/tree/master/examples/Hat/SPEAKER <br>
