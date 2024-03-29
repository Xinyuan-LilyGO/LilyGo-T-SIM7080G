


<h1 align = "center">✨ LilyGO T-SIM7080G ✨</h1>

## **[English](README.MD) | 中文**


## 1️⃣ 示例预览

```

examples 
├─AllFunction                       # 全功能测试示例
├─BLE5_extended_scan                # 官方BLE5.0示例
├─BLE5_multi_advertising            # 官方BLE5.0示例
├─BLE5_periodic_advertising         # 官方BLE5.0示例    
├─BLE5_periodic_sync                # 官方BLE5.0示例
├─BIGIOT_Gnss_Upload                # 上传定位坐标到BIGIOT 平台，定时唤醒消耗电流约1.2mA
├─MinimalCameraExample              # 摄像头使用示例
├─MinimalModemGPSExample            # SIM7080G GPS 示例
├─MinimalModemNBIOTExample          # SIM7080G NBIOT 示例
├─MinimalModemPowerSaveMode         # SIM7080G 电源保持示例
├─MinimalModemSleepMode             # SIM7080G 睡眠示例
├─MinimalModemUpgrade               # SIM7080G 升级内置固件
├─MinimalPowersExample              # PMU使用示例
├─MinimalPowersCurrentExample       # PMU 充电电流调试
├─MinimalSDCardExample              # SDCard 使用示例
├─ModemMqttPulishExample            # MQTT Pulish Example 
├─ModemMqttsExample                 # MQTTS Example 
├─ModemMqttSubscribeExample         # MQTT Subscribe Example 
├─SIM7080G-ATT-NB-IOT-AWS-MQTT      # MQTTS AWS By @bootcampiot
├─SIM7080G-ATT-NB-IOT-HTTP-HTTPS    # HTTP/HTTPS  By @bootcampiot
└─SIM7080G-ATT-NB-IOT-SSL-Mosquitto # MQTTS Mosquitto By @bootcampiot
```


## 2️⃣ 快速开始

### PlatformIO

1. 安装[VisualStudioCode](https://code.visualstudio.com/) 和 [Python](https://www.python.org/)
2. 在`VisualStudioCode`扩展中搜索`PlatformIO`插件并安装.
3. 安装完成后需要将`VisualStudioCode`重新启动
4. 重新开启`VisualStudioCode`后,选择 `VisualStudioCode` 左上角的 `文件` -> `打开文件夹` ->选择 `LilyGo-T-SIM7080G` 目录
5. 点击 `platformio.ini` 文件,在`platformio` 栏目中取消需要使用的示例行,请确保仅仅一行有效
6. 点击左下角的（✔）符号编译
7. 将板子与电脑USB进行连接
8. 点击（→）上传固件
9. 点击 (插头符号) 监视串行输出


### ArduinoIDE


1. 安装 [ArduinoIDE](https://www.arduino.cc/en/software)
2. 将 `LilyGo-T-SIM7080G/lib` 目录内的所有文件夹拷贝到`<C:\Users\UserName\Documents\Arduino\libraries>`,如果没有`libraries`目录,请新建,请注意,不是拷贝`lib`目录,而是拷贝lib目录里面的文件夹
3. 打开ArduinoIDE -> Tools 
   - Board -> ESP32S3 Dev Module
   - USB CDC On Boot -> Enable   # 注意，在不连接USB的时候你需要将Enable改为Disable，这样USB CDC 不会阻止板子的启动
   - CPU Frequency -> 240MHz
   - USB DFU On Boot -> Disable
   - Flash Mode -> QIO 80MHz
   - Flash Size -> 16MB(128Mb)
   - USB Firmware MSC On Boot -> Disable
   - PSRAM -> OPI PSRAM
   - Partition Scheme -> 16M Flash(3MB APP/9.9MB FATFS)
   - USB Mode -> Hardware CDC and JIAG
   - Upload Mode -> UART0/Hardware CDC
   - Upload Speed -> 921600
4. 插入USB到PC,点击上传<如果无法顺利上传,请保持按压BOOT按键,然后单击RST,然后再点击上传,上传完成时需要点击RST退出下载模式>


## 3️⃣ 提示:

0. ⚠ SIM7080G 不能同时连接蜂窝网和GPS定位
1. 板子集成PMU(Power management chip),具有短路和过载保护,默认情况下`PWRKEY`需要按压6秒才能关闭板子供电,按压`PWRKEY`128毫秒开启电源,如需要修改关机按压时间请参考`MinimalPowersExample`
2. 需要充电功能需要关闭`PMU` `TS Pin`检测功能,默认板载没有NTC传感器,所以需要禁用`TS Pin`检测才能正常使用充电功能,如果不关闭`TS Pin`检测功能,插入电池后PMU充电指示灯将会闪烁,此时充电将被禁用
3. 板子 `VSYS`为PMU内部5V升压输出，只能输出不能输入,请不要外接大于500mA的外部供电负载
4. `DC5`供电电压可以调节，最大输出电流请不要超过 `1A`
5. `DC1`为核心供电电压,默认输出`3.3V`,请不要通过软件调节`DC1`电压
6. 当无法上传草图时,请按住板子上的`BOOT`按键,然后插入USB,此时你应该能在电脑的设备管理器中看到端口,然后再次点击上传草图
7. 当错误的关闭`ESP32S3`的供电通道时,请插入USB，然后按住板子的`BOOT`按键,再按住`PWRKEY`按键,此时板子进入下载模式,这时候可以正常上传草图
8. 如果在接入电池下无法开启板子供电,请检查USB旁边的机械开关拨到了 `ON`,然后按压板子PWR 2秒,此时板子应开始工作
9. 太阳能输入电压范围为`4.4~6V`，请选择合适的太阳能面板,充电最大电流设定为`500mA`
10. 如果在不接摄像头的情况下,所有IO口均可用,如果使用摄像头则板子则只有GPIO45,GPIO46为空闲
11. `T-SIM7080G` 使用 `Octal SPI` ,所以 **GPIO35 ~ GPIO37** 为不可用,请不要使用这几个IO
12. ⚠ **请在更改外设电压之前明白需要面临的风险,否则请不要尝试更改摄像头和其他板载设备的电压,可能会面临永久性的损坏**
13. ⚠ **请不要关闭BLDO1电源，否则ESP32S3与SIM7080G无法正常通讯.**
14. 更新调制解调器内置固件请查看[操作说明](./docs/sim7080_update_firmware.md),一般情况下不建议升级固件.


## 4️⃣ 在哪购买:

1. [AliExpress](https://www.aliexpress.us/item/3256805002673427.html)
2. [LilyGo Store](https://www.lilygo.cc/products/t-sim7080-s3)

## 5️⃣ 引脚分布:

| Camera     | PWDN   | Reset  | XCLK   | SDA    | SCL    | VSYNC  | HREF   | PCLK   |
| ---------- | ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
| GPIO       | N/A    | 18     | 8      | 2      | 1      | 16     | 17     | 12     |
| **Camera** | **D9** | **D8** | **D7** | **D6** | **D5** | **D4** | **D3** | **D2** |
| GPIO       | 9      | 10     | 11     | 13     | 21     | 48     | 47     | 14     |

* 如果不使用摄像头,那么你可以随意的分配这里IO口,假设你需要分配I2C，那么请使用显示调用Wire并且传递需要使用的IO口例如
   ```
   int sda = 13;  //也可以用别的IO口
   int scl = 21;  //也可以用别的IO口
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

## 电源域

| Modem          | Camera            | ESP32S3 | SDCard | Level conversion |
| -------------- | ----------------- | ------- | ------ | ---------------- |
| DC3/BLDO2(GPS) | ALDO1/ALDO2/ALDO4 | DC1     | ALDO3  | BLDO1            |
