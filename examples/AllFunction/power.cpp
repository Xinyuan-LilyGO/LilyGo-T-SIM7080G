/**
 * @file      power.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#include "power.h"

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"
XPowersPMU PMU;


bool setupPower()
{
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("ERROR: Init PMU failed!");
        return false;
    }

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
    }
    
    // Set VSY off voltage as 2600mV , Adjustment range 2600mV ~ 3300mV
    PMU.setSysPowerDownVoltage(2600);

    //Turn off not use power channel
    PMU.disableDC2();
    PMU.disableDC4();
    PMU.disableDC5();

    PMU.disableALDO1();
    PMU.disableALDO2();
    PMU.disableALDO3();
    PMU.disableALDO4();
    PMU.disableBLDO1();
    PMU.disableBLDO2();

    PMU.disableCPUSLDO();
    PMU.disableDLDO1();
    PMU.disableDLDO2();
    PMU.enableBLDO1();

    //ESP32S3 Core VDD 3300mV Don't change,default turn on
    // PMU.setDC1Voltage(3300);
    // PMU.enableDC1();

    // CAM DVDD  1500~1800mV
    PMU.setALDO1Voltage(1800);
    PMU.enableALDO1();

    // CAM DVDD 2500~2800mV
    PMU.setALDO2Voltage(2800);
    PMU.enableALDO2();

    // CAM AVDD 2800~3000mV
    PMU.setALDO4Voltage(3000);
    PMU.enableALDO4();

    // SD Card VDD 3300mV
    PMU.setALDO3Voltage(3300);
    PMU.enableALDO3();

    //Modem 2700~3400mV VDD
    PMU.setDC3Voltage(3000);
    PMU.enableDC3();

    //Modem GPS Power
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();

    //External row needle , 1400~3700mV
    PMU.setDC5Voltage(3300);
    PMU.enableDC5();

    Serial.println("=========================================");
    Serial.printf("DC1  : %s   Voltage:%u mV \n",  PMU.isEnableDC1()  ? "+" : "-", PMU.getDC1Voltage());
    Serial.printf("DC2  : %s   Voltage:%u mV \n",  PMU.isEnableDC2()  ? "+" : "-", PMU.getDC2Voltage());
    Serial.printf("DC3  : %s   Voltage:%u mV \n",  PMU.isEnableDC3()  ? "+" : "-", PMU.getDC3Voltage());
    Serial.printf("DC4  : %s   Voltage:%u mV \n",  PMU.isEnableDC4()  ? "+" : "-", PMU.getDC4Voltage());
    Serial.printf("DC5  : %s   Voltage:%u mV \n",  PMU.isEnableDC5()  ? "+" : "-", PMU.getDC5Voltage());
    Serial.println("=========================================");
    Serial.printf("ALDO1: %s   Voltage:%u mV\n",  PMU.isEnableALDO1()  ? "+" : "-", PMU.getALDO1Voltage());
    Serial.printf("ALDO2: %s   Voltage:%u mV\n",  PMU.isEnableALDO2()  ? "+" : "-", PMU.getALDO2Voltage());
    Serial.printf("ALDO3: %s   Voltage:%u mV\n",  PMU.isEnableALDO3()  ? "+" : "-", PMU.getALDO3Voltage());
    Serial.printf("ALDO4: %s   Voltage:%u mV\n",  PMU.isEnableALDO4()  ? "+" : "-", PMU.getALDO4Voltage());
    Serial.println("=========================================");
    Serial.printf("BLDO1: %s   Voltage:%u mV\n",  PMU.isEnableBLDO1()  ? "+" : "-", PMU.getBLDO1Voltage());
    Serial.printf("BLDO2: %s   Voltage:%u mV\n",  PMU.isEnableBLDO2()  ? "+" : "-", PMU.getBLDO2Voltage());

    PMU.clearIrqStatus();

    PMU.enableVbusVoltageMeasure();
    PMU.enableBattVoltageMeasure();
    PMU.enableSystemVoltageMeasure();
    PMU.disableTemperatureMeasure();

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();

    // Disable all interrupts
    PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    // Clear all interrupt flags
    PMU.clearIrqStatus();
    // Enable the required interrupt function
    PMU.enableIRQ(
        XPOWERS_AXP2101_BAT_INSERT_IRQ    | XPOWERS_AXP2101_BAT_REMOVE_IRQ      |   //BATTERY
        XPOWERS_AXP2101_VBUS_INSERT_IRQ   | XPOWERS_AXP2101_VBUS_REMOVE_IRQ     |   //VBUS
        XPOWERS_AXP2101_PKEY_SHORT_IRQ    | XPOWERS_AXP2101_PKEY_LONG_IRQ       |   //POWER KEY
        XPOWERS_AXP2101_BAT_CHG_DONE_IRQ  | XPOWERS_AXP2101_BAT_CHG_START_IRQ       //CHARGE
        // XPOWERS_PKEY_NEGATIVE_IRQ | XPOWERS_PKEY_POSITIVE_IRQ   |   //POWER KEY
    );

    // Set the precharge charging current
    PMU.setPrechargeCurr(XPOWERS_AXP2101_PRECHARGE_50MA);
    // Set constant current charge current limit
    PMU.setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_300MA);
    // Set stop charging termination current
    PMU.setChargerTerminationCurr(XPOWERS_AXP2101_CHG_ITERM_25MA);

    // Set charge cut-off voltage
    PMU.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V1);

    // Set the time of pressing the button to turn off
    PMU.setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);
    uint8_t opt = PMU.getPowerKeyPressOffTime();
    Serial.print( "PowerKeyPressOffTime:");
    switch (opt) {
    case XPOWERS_POWEROFF_4S: Serial.println( "4 Second");
        break;
    case XPOWERS_POWEROFF_6S: Serial.println( "6 Second");
        break;
    case XPOWERS_POWEROFF_8S: Serial.println( "8 Second");
        break;
    case XPOWERS_POWEROFF_10S: Serial.println( "10 Second");
        break;
    default:
        break;
    }
    return true;
}

void loopPower()
{
    //todo:
}