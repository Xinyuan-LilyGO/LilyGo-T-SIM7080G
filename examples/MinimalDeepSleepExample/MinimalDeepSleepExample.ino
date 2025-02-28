/**
 * @file      MinimalDeepSleepExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2025  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2025-01-22
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

// Set to wake up every 5 minutes to send positioning coordinates to the platform
#define uS_TO_S_FACTOR 1000000ULL                       /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  20                             /* Time ESP32 will go to sleep (in seconds) */


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

void getWakeupReason();


void setup()
{

    Serial.begin(115200);

    // Start while waiting for Serial monitoring
    while (!Serial);

    delay(3000);

    getWakeupReason();

    Serial.println();

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
    Serial.println();
    Serial.print("Modem started!");

    // Keep running for 10 seconds
    uint32_t runAt = millis() + 10000;
    while (runAt > millis()) {
        while (Serial1.available()) {
            Serial.write(Serial1.read());
        }
        while (Serial.available()) {
            Serial1.write(Serial.read());
        }
    }

    // You can turn off the modem or PMU power supply voltage
    // It can also keep the SIM7080G powered
    // modem.poweroff();

    // PMU.disableDC3();
    // PMU.disableBLDO2();

    int delaySeconds = 5;
    while (delaySeconds--) {
        Serial.println("Will enter to deep sleep after ");
        Serial.print(delaySeconds);
        Serial.println(" seconds");
        delay(1000);
    }


    /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
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

}

void loop()
{
}



void getWakeupReason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_UNDEFINED:
        //!< In case of deep sleep, reset was not caused by exit from deep sleep
        Serial.println("In case of deep sleep, reset was not caused by exit from deep sleep");
        break;
    case ESP_SLEEP_WAKEUP_ALL:
        //!< Not a wakeup cause: used to disable all wakeup sources with esp_sleep_disable_wakeup_source
        Serial.println("Not a wakeup cause: used to disable all wakeup sources with esp_sleep_disable_wakeup_source");
        break;
    case ESP_SLEEP_WAKEUP_EXT0:
        //!< Wakeup caused by external signal using RTC_IO
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        //!< Wakeup caused by external signal using RTC_CNTL
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        //!< Wakeup caused by timer
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        //!< Wakeup caused by touchpad
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        //!< Wakeup caused by ULP program
        Serial.println("Wakeup caused by ULP program");
        break;
    case  ESP_SLEEP_WAKEUP_GPIO:
        //!< Wakeup caused by GPIO (light sleep only)
        Serial.println("Wakeup caused by GPIO (light sleep only)");
        break;
    case ESP_SLEEP_WAKEUP_UART:
        //!< Wakeup caused by UART (light sleep only)
        Serial.println("Wakeup caused by UART (light sleep only)");
        break;
    case ESP_SLEEP_WAKEUP_WIFI:
        //!< Wakeup caused by WIFI (light sleep only)
        Serial.println("Wakeup caused by WIFI (light sleep only)");
        break;
    case ESP_SLEEP_WAKEUP_COCPU:
        //!< Wakeup caused by COCPU int
        Serial.println("Wakeup caused by COCPU int");
        break;
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
        //!< Wakeup caused by COCPU crash
        Serial.println("Wakeup caused by COCPU crash");
        break;
    case  ESP_SLEEP_WAKEUP_BT:
        //!< Wakeup caused by BT (light sleep only)
        Serial.println("Wakeup caused by BT (light sleep only)");
        break;
    default :
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;

    }
}