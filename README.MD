<h1 align = "center">✨ LilyGO T-SIM7080G  ✨</h1>

## **English | [中文](README_CN.MD)**

## News

- SIM7080G supports **NB-IoT** and **Cat-M** in global frequency bands, but does **not support 2G/3G/4G**. Please confirm that the SIM card used supports **NB-IoT** or **Cat-M** before use.
- The SIM card must be inserted first before starting the SIM7080. If the SIM card is inserted after the SIM7080 is turned on, the modem will not be able to detect the SIM card.


## 1️⃣ Examples preview

````

examples 
├─AllFunction                   # Full function test example
├─BLE5_extended_scan            # Official BLE5 Example
├─BLE5_multi_advertising        # Official BLE5 Example
├─BLE5_periodic_advertising     # Official BLE5 Example    
├─BLE5_periodic_sync            # Official BLE5 Example
├─BIGIOT_Gnss_Upload            # Upload positioning coordinates to BIGIOT IOT,Timed wake-up consumes about 1.2mA
├─MinimalCameraExample          # Example of camera use
├─MinimalModemGPSExample        # SIM7080G GPS example
├─MinimalModemNBIOTExample      # SIM7080G NBIOT example
├─MinimalModemPowerSaveMode     # SIM7080G power maintenance example
├─MinimalModemSleepMode         # SIM7080G sleep example
├─MinimalModemUpgrade           # Upgrade SIM7080 built-in firmware
├─MinimalPowersExample          # Example of PMU
├─MinimalPowersCurrentExample   # Example of PMU charge current debug
├─MinimalSDCardExample          # Example of using SDCard
├─ModemMqttPulishExample        # MQTT Pulish Example            
├─ModemMqttsExample             # MQTTS Example    
├─ModemMqttSubscribeExample     # MQTT Subscribe Example     
├─SIM7080G-ATT-NB-IOT-AWS-MQTT  # MQTTS AWS By @bootcampiot
├─SIM7080G-ATT-NB-IOT-HTTP-HTTPS # HTTP/HTTPS  By @bootcampiot
└─SIM7080G-ATT-NB-IOT-SSL-Mosquitto # MQTTS Mosquitto By @bootcampiot

````


## 2️⃣ Quick Start

### PlatformIO

1. Install [VisualStudioCode](https://code.visualstudio.com/) and [Python](https://www.python.org/)
2. Search for the `PlatformIO` plugin in the `VisualStudioCode` extension and install it.
3. After the installation is complete, you need to restart `VisualStudioCode`
4. After restarting `VisualStudioCode`, select `File` in the upper left corner of `VisualStudioCode` -> `Open Folder` -> select the `LilyGo-T-SIM7080G` directory
5. Click on the `platformio.ini` file, and in the `platformio` column, cancel the sample line that needs to be used, please make sure that only one line is valid
6. Click the (✔) symbol in the lower left corner to compile
7. Connect the board to the computer USB
8. Click (→) to upload firmware
9. Click (plug symbol) to monitor serial output

### ArduinoIDE

1. Install [ArduinoIDE](https://www.arduino.cc/en/software)
2. Copy all the folders in the `LilyGo-T-SIM7080G/lib` directory to `<C:\Users\UserName\Documents\Arduino\libraries>`. If there is no `libraries` directory, please create a new one. Please note that you are not copying the `lib` directory, but copying the folders in the lib directory
3. Open Arduino IDE -> Tools
- Board -> ESP32S3 Dev Module
- USB CDC On Boot -> Enable  ## Note that you need to change Enable to Disable when USB is not connected, so USB CDC will not prevent the board from starting
- CPU Frequency -> 240MHz
- USB DFU On Boot -> Disable
- Flash Mode -> QIO 80MHz
- Flash Size -> 16MB(128Mb)
- USB Firmware MSC On Boot -> Disable
- PSRAM -> OPI PSRAM
- Partition Scheme -> 16M Flash(3MB APP/9.9MB FATFS)
- USB Mode -> Hardware CDC and JTAG
- Upload Mode -> UART0/Hardware CDC
- Upload Speed -> 921600
4. Insert USB into the PC and click Upload <If the upload fails, see Tips>

## 3️⃣ Tips

0. T-SIM7080 has two USB ports, the USB-C port is used to upload sketches and Serial output, and Micro-USB is used for SIM7080G firmware version upgrade
1. ⚠ SIM7080G cannot connect to cellular network and GPS positioning at the same time
2. Board integrated PMU (Power management chip), with short circuit and overload protection. By default,`PWRKEY`needs to be pressed for 6 seconds to turn off the board power supply, and press`PWRKEY`for 128 milliseconds to turn on the power supply. If you need to modify the power off pressing time, please refer to`MinimalPowersExample`
3. If the charging function is required, the`PMU` `TS Pin`detection function needs to be turned off. By default, there is no NTC sensor on the board, so the `TS Pin` detection needs to be disabled before the charging function can be used normally. If the`TS Pin`detection function is not turned off, the PMU charging indicator will flash after the battery is inserted, and charging will be disabled
4. The board `VSYS` is the internal 5V boost output of the PMU, which can only be output but not input. Please do not connect an external power supply load greater than `500mA`
5. The `DC5` power supply voltage can be adjusted, and the maximum output current should not exceed `1A`
5.`DC1` is the core power supply voltage, and the default output is `3.3V`. Please do not adjust the `DC1` voltage through software
1. When the sketch cannot be uploaded, please press and hold the `BOOT` button on the board, and then insert the USB. At this time, you should be able to see the port in the device manager of the computer, and then click Upload Sketch again
2. When the power supply channel of the `ESP32S3` is closed incorrectly, please insert USB, and then press and hold the `BOOT` button of the board, and then press and hold the `PWRKEY` button. At this time, the board enters the download mode, and the sketch can be uploaded normally
3. If the power supply of the board cannot be turned on when the battery is connected, please check that the mechanical switch beside the USB is turned to`ON`, and then press the board `PWR` for `2 seconds`. At this time, the board should start to work
4. The range of solar input voltage is `4.4~6V`. Please select an appropriate solar panel and set the maximum charging current as `500mA`
5.  If the camera is not connected, all IO ports are available. If the camera is used, the board has only `GPIO45` and `GPIO46` is idle
6.  `T-SIM7080G` uses `Octal SPI PSRAM`, so **GPIO35~GPIO37** is unavailable. Please do not use these IOs
7.  ⚠ **Please understand the risks to be faced before changing the voltage of peripherals, otherwise do not try to change the voltage of cameras and other on-board devices, which may cause permanent damage**
8.  ⚠ **Please do not turn off the BLDO1 power supply, otherwise ESP32S3 and SIM7080G cannot communicate normally**
9.  Please check the [Operating Instructions](./docs/sim7080_update_firmware.md) for updating the built-in firmware of the modem. Generally, upgrading the firmware is not recommended
10. The board uses USB as the JTAG upload port. When printing serial port information on USB_CDC_ON_BOOT configuration needs to be turned on. 
If the port cannot be found when uploading the program or the USB has been used for other functions, the port does not appear. 
Please enter the upload mode manually. 
    1. Connect the board via the USB cable
    2. Press and hold the BOOT button , While still pressing the BOOT button, press RST
    3. Release the RST
    4. Release the BOOT button
    5. Upload sketch

## 4️⃣ Where to buy:

1. [AliExpress](https://www.aliexpress.us/item/3256805002673427.html)
2. [LilyGo Store](https://www.lilygo.cc/products/t-sim7080-s3)



## 5️⃣ Pins:


| Camera     | PWDN   | Reset  | XCLK   | SDA    | SCL    | VSYNC  | HREF   | PCLK   |
| ---------- | ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
| GPIO       | N/A    | 18     | 8      | 2      | 1      | 16     | 17     | 12     |
| **Camera** | **D9** | **D8** | **D7** | **D6** | **D5** | **D4** | **D3** | **D2** |
| GPIO       | 9      | 10     | 11     | 13     | 21     | 48     | 47     | 14     |

* If you do not use a camera, you can freely allocate IO ports here. Assuming you need to allocate I2C, please use the display to call Wire and pass the required IO ports, such as
    ```
    int sda = 13;  // You can also use other IO ports
    int scl = 21;  // You can also use other IO ports
    Wire.begin(sda,scl)
    ```

| Modem | PWR | RXD | TXD | RI  | DTR |
| ----- | --- | --- | --- | --- | --- |
| GPIO  | 41  | 4   | 5   | 3   | 42  |


| PMU  | SDA | SCL | IRQ |
| ---- | --- | --- | --- |
| GPIO | 15  | 7   | 6   |


| SDCard | CMD | CLK | DATA |
| ------ | --- | --- | ---- |
| GPIO   | 39  | 38  | 40   |

## Power domain

| Modem          | Camera            | ESP32S3 | SDCard | Level conversion |
| -------------- | ----------------- | ------- | ------ | ---------------- |
| DC3/BLDO2(GPS) | ALDO1/ALDO2/ALDO4 | DC1     | ALDO3  | BLDO1            |