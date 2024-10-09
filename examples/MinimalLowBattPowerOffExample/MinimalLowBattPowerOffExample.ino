/**
 * @file      MinimalLowBattPowerOffExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-05-02
 *
 */
#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU  PMU;

bool  pmu_flag = false;
uint32_t loopMillis;

void setFlag(void)
{
    pmu_flag = true;
}

void setup()
{

    Serial.begin(115200);

    //Start while waiting for Serial monitoring
    while (!Serial);

    delay(3000);

    Serial.println();

    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    bool res;
    //Use Wire1
    res = PMU.begin(Wire1, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
    if (!res) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
    }
    

    // I2C sensor call example
    int sda = 13;  // You can also use other IO ports
    int scl = 21;  // You can also use other IO ports
    Wire.begin(sda, scl);

    //**\

    //Other i2c sensors can be externally connected to 13,21

    //**\


    Serial.printf("getID:0x%x\n", PMU.getChipID());

    // Set the minimum common working voltage of the PMU VBUS input,
    // below this value will turn off the PMU
    PMU.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);

    // Set the maximum current of the PMU VBUS input,
    // higher than this value will turn off the PMU
    PMU.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);

    // Set VSY off voltage as 2600mV , Adjustment range 2600mV ~ 3300mV
    PMU.setSysPowerDownVoltage(2600);

    //Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    //SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    //Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      //The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();



    /*********************************
     * step 2 : Enable internal ADC detection
    ***********************************/
    PMU.enableBattDetection();
    PMU.enableVbusVoltageMeasure();
    PMU.enableBattVoltageMeasure();
    PMU.enableSystemVoltageMeasure();


    /*********************************
     * step 3 : Set PMU interrupt
    ***********************************/
    pinMode(PMU_INPUT_PIN, INPUT);
    attachInterrupt(PMU_INPUT_PIN, setFlag, FALLING);

    // Disable all interrupts
    PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    // Clear all interrupt flags
    PMU.clearIrqStatus();
    // Enable the required interrupt function
    PMU.enableIRQ(
        XPOWERS_AXP2101_BAT_INSERT_IRQ    | XPOWERS_AXP2101_BAT_REMOVE_IRQ      |   //BATTERY
        XPOWERS_AXP2101_VBUS_INSERT_IRQ   | XPOWERS_AXP2101_VBUS_REMOVE_IRQ     |   //VBUS
        XPOWERS_AXP2101_PKEY_SHORT_IRQ    | XPOWERS_AXP2101_PKEY_LONG_IRQ       |   //POWER KEY
        XPOWERS_AXP2101_BAT_CHG_DONE_IRQ  | XPOWERS_AXP2101_BAT_CHG_START_IRQ    |   //CHARGE
        XPOWERS_AXP2101_WARNING_LEVEL1_IRQ | XPOWERS_AXP2101_WARNING_LEVEL2_IRQ      // LOW BATTERY
    );

    /*********************************
     * step 4 : Set PMU Charger params
    ***********************************/
    // Set the precharge charging current
    PMU.setPrechargeCurr(XPOWERS_AXP2101_PRECHARGE_50MA);
    // Set constant current charge current limit
    PMU.setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_200MA);
    // Set stop charging termination current
    PMU.setChargerTerminationCurr(XPOWERS_AXP2101_CHG_ITERM_25MA);

    // Set charge cut-off voltage
    PMU.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V1);


    /*
    The default setting is CHGLED is automatically controlled by the PMU.
    - XPOWERS_CHG_LED_OFF,
    - XPOWERS_CHG_LED_BLINK_1HZ,
    - XPOWERS_CHG_LED_BLINK_4HZ,
    - XPOWERS_CHG_LED_ON,
    - XPOWERS_CHG_LED_CTRL_CHG,
    * */
    PMU.setChargingLedMode(XPOWERS_CHG_LED_CTRL_CHG);



    /*********************************
     * step 4 : Set PMU low battery params
    ***********************************/

    // Get the default low pressure warning percentage setting
    uint8_t low_warn_per = PMU.getLowBatWarnThreshold();
    Serial.printf("Default low battery warning threshold is %d percentage\n", low_warn_per);

    //
    // setLowBatWarnThreshold Range:  5% ~ 20%
    // The following data is obtained from actual testing , Please see the description below for the test method.
    // 20% ~= 3.7v
    // 15% ~= 3.6v
    // 10% ~= 3.55V
    // 5%  ~= 3.5V
    // 1%  ~= 3.4V
    PMU.setLowBatWarnThreshold(5); // Set to trigger interrupt when reaching 5%

    // Get the low voltage warning percentage setting
    low_warn_per = PMU.getLowBatWarnThreshold();
    Serial.printf("Set low battery warning threshold is %d percentage\n", low_warn_per);

    // Get the default low voltage shutdown percentage setting
    uint8_t low_shutdown_per = PMU.getLowBatShutdownThreshold();
    Serial.printf("Default low battery shutdown threshold is %d percentage\n", low_shutdown_per);

    // setLowBatShutdownThreshold Range:  0% ~ 15%
    // The following data is obtained from actual testing , Please see the description below for the test method.
    // 15% ~= 3.6v
    // 10% ~= 3.55V
    // 5%  ~= 3.5V
    // 1%  ~= 3.4V
    PMU.setLowBatShutdownThreshold(1);  // Set to trigger interrupt when reaching 1%

    // Get the low voltage shutdown percentage setting
    low_shutdown_per = PMU.getLowBatShutdownThreshold();
    Serial.printf("Set low battery shutdown threshold is %d percentage\n", low_shutdown_per);



    /*
    *
    *    Measurement methods:
    *    1. Connect the battery terminal to a voltage stabilizing source
    *    2. Set voltage test voltage
    *    3. Press PWR to boot
    *    4. Read the serial output voltage percentage
    *
    *   If a voltage regulator is connected during testing and the voltage is slowly reduced,
    *   the voltage percentage will not change immediately. It will take a while to slowly decrease.
    *   In actual production, it needs to be adjusted according to the actual situation.
    * * * */
}

void loop()
{
    if (pmu_flag) {

        pmu_flag = false;

        // Get PMU Interrupt Status Register
        uint32_t status = PMU.getIrqStatus();

        if (PMU.isVbusInsertIrq()) {
            Serial.println("isVbusInsert");
        }
        if (PMU.isVbusRemoveIrq()) {
            Serial.println("isVbusRemove");
        }
        if (PMU.isBatInsertIrq()) {
            Serial.println("isBatInsert");
        }
        if (PMU.isBatRemoveIrq()) {
            Serial.println("isBatRemove");
        }
        if (PMU.isPekeyShortPressIrq()) {
            Serial.println("isPekeyShortPress");
        }
        if (PMU.isPekeyLongPressIrq()) {
            Serial.println("isPekeyLongPress");
        }

        // When the set low-voltage battery percentage warning threshold is reached,
        // set the threshold through getLowBatWarnThreshold( 5% ~ 20% )
        if (PMU.isDropWarningLevel2Irq()) {
            Serial.println("The voltage percentage has reached the low voltage warning threshold!!!");
        }

        // When the set low-voltage battery percentage shutdown threshold is reached
        // set the threshold through setLowBatShutdownThreshold()
        if (PMU.isDropWarningLevel1Irq()) {
            int i = 4;
            while (i--) {
                Serial.printf("The voltage percentage has reached the low voltage shutdown threshold and will shut down in %d seconds.\n", i);
            }
            // Turn off all power supplies, leaving only the RTC power supply. The RTC power supply cannot be turned off.
            PMU.shutdown();
        }


        // For more interrupt sources, please check XPowersLib


        // Clear PMU Interrupt Status Register
        PMU.clearIrqStatus();

    }


    if (millis() - loopMillis > 3000) {
        Serial.print("getBattVoltage:"); Serial.print(PMU.getBattVoltage()); Serial.println("mV");
        Serial.print("getVbusVoltage:"); Serial.print(PMU.getVbusVoltage()); Serial.println("mV");
        Serial.print("getSystemVoltage:"); Serial.print(PMU.getSystemVoltage()); Serial.println("mV");

        // The battery percentage may be inaccurate at first use, the PMU will automatically
        // learn the battery curve and will automatically calibrate the battery percentage
        // after a charge and discharge cycle
        if (PMU.isBatteryConnect()) {
            Serial.print("getBatteryPercent:"); Serial.print(PMU.getBatteryPercent()); Serial.println("%");
        }
        Serial.println();
        loopMillis = millis();
    }
}

