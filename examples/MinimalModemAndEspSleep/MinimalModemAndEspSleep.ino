/**
 * @file      MinimalModemAndEspSleep.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2025  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2025-03-26
 *
 */
#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU  PMU;

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register to. The GPRS service is disabled, the UE is allowed to attach for GPRS if requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an operator to register to. The GPRS service is enabled, but an allowable PLMN is currently not available. The UE will start a GPRS attach as soon as an allowable PLMN is available.",
    "Registration denied, the GPRS service is disabled, the UE is not allowed to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

#define uS_TO_S_FACTOR 1000000ULL                       /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  120                              /* Time ESP32 will go to sleep (in seconds) */

void setup()
{

    Serial.begin(115200);

    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }

    // UNUSED POWER CHANNEL
    PMU.disableDC2();
    PMU.disableDC4();
    PMU.disableDC5();
    PMU.disableALDO1();
    PMU.disableBLDO1();
    PMU.disableCPUSLDO();
    PMU.disableDLDO1();
    PMU.disableDLDO2();

    //! Do not turn off BLDO1, which controls the 3.3V power supply for level conversion.
    //! If it is turned off, it will not be able to communicate with the modem normally
    PMU.setBLDO1Voltage(3300);    // Set the power supply for level conversion to 3300mV
    PMU.enableBLDO1();

    // Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    // SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    // Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      // The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();


    /*********************************
     * step 2 : start modem
    ***********************************/

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);


    bool first_boot = false;
    // When powered on for the first time, enable PWR to control the modem to start
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        first_boot = true;
    } else {
        // If it is a sleep wake-up, cancel the DTR high level
        gpio_hold_dis((gpio_num_t)BOARD_MODEM_DTR_PIN);
        gpio_reset_pin((gpio_num_t)BOARD_MODEM_DTR_PIN);
    }


    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);

    /**
     *  When the user sets "AT+CSCLK=1", pull up the DTR pin,
     *  the module will automatically enter the sleep mode.
     *  At this time, the serial port function cannot communicate normally.
     *  Pulling DTR low in this mode can wake up the module
     */
    digitalWrite(BOARD_MODEM_DTR_PIN, LOW);

    int retry = 0;
    while (!modem.testAT(1000)) {
        Serial.print(".");
        if (first_boot) {
            if (retry++ > 6) {
                // Pull down PWRKEY for more than 1 second according to manual requirements
                digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
                delay(100);
                digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
                delay(1000);
                digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
                retry = 0;
                Serial.println("Retry start modem .");
            }
        }
    }
    Serial.println();
    Serial.print("Modem started!");

    // Turn on net-light led
    modem.sendAT("+CNETLIGHT=1");
    if (modem.waitResponse() != 1) {
        Serial.println("Failed!");
    }

    // Turn off sleep first
    modem.sendAT("+CSCLK=0");
    if (modem.waitResponse() != 1) {
        Serial.println("Failed!");
    }

    int delaySeconds = 30;
    while (delaySeconds--) {
        Serial.println("will set the modem to sleep after ");
        Serial.print(delaySeconds);
        Serial.println(" seconds");
        delay(1000);
    }

    /*********************************
    * step 3 : Set Modem to sleep mode
    ***********************************/

    // Turn off net-light led
    modem.sendAT("+CNETLIGHT=0");
    if (modem.waitResponse() != 1) {
        Serial.println("Failed!");
    }

    // Status Led can't turn off

    Serial.print("Set Modem to sleep mode");

    modem.sendAT("+CSCLK=1");
    if (modem.waitResponse() != 1) {
        Serial.println("Failed!"); return;
    }
    Serial.println("Success!");

    // Pulling up DTR pin, module will go to normal sleep mode
    // After level conversion, set the DTR Pin output to low, then the module DTR pin is high
    digitalWrite(BOARD_MODEM_DTR_PIN, HIGH);
    gpio_hold_en((gpio_num_t)BOARD_MODEM_DTR_PIN);

    // /*********************************
    // * step 4 : Check if the modem is unresponsive
    // ***********************************/

    Serial.println("At this time, the serial communication will not have any feedback. After the modem wakes up, the communication will be normal.");
    uint32_t timeout = millis() + 30000;
    while (millis() < timeout) {
        if (modem.testAT(1000)) {
            Serial.println("Modem response , modem not enter sleep mode");
        } else {
            Serial.println("Modem not response , modem has enter sleep mode");
        }
        delay(1000);
    }

    // /*********************************
    // * step 5 : Turn off the power to save electricity
    // ***********************************/
    PMU.disableBattVoltageMeasure();
    PMU.disableTemperatureMeasure();
    PMU.disableVbusVoltageMeasure();
    PMU.disableSystemVoltageMeasure();
    PMU.disableBLDO1(); // level conversion
    PMU.disableBLDO2(); // GPS

    Wire.end();
    pinMode(I2C_SDA, OPEN_DRAIN);
    pinMode(I2C_SCL, OPEN_DRAIN);
    pinMode(PMU_INPUT_PIN, OPEN_DRAIN);

    Serial1.end();
    Serial.end();



    // /*********************************
    // * step 6 : Putting the ESP32 into Deep Sleep
    // ***********************************/

    /*
    First we configure the wake up source
    We set our ESP32 to wake up every 120 seconds
    */
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                   " Seconds");

    /*
    Next we decide what all peripherals to shut down/keep on
    By default, ESP32 will automatically power down the peripherals
    not needed by the wakeup source, but if you want to be a poweruser
    this is for you. Read in detail at the API docs
    http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
    Left the line commented as an example of how to configure peripherals.
    The line below turns off all RTC peripherals in deep sleep.
    */
    // esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    // Serial.println("Configured all RTC Peripherals to be powered down in sleep");

    /*
    Now that we have setup a wake cause and if needed setup the
    peripherals state in deep sleep, we can now start going to
    deep sleep.
    In the case that no wake up sources were provided but deep
    sleep was started, it will sleep forever unless hardware
    reset occurs.
    */
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
    Serial.println("This will never be printed");


    // The current consumption is about 1.7 ~ 2mA
}

void loop()
{
}
