/**
 * @file      ATDebug.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-06-26
 *
 */

#include <Arduino.h>
#include "utilities.h"
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS
#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif


#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
XPowersPMU  PMU;




void setup()
{
    Serial.begin(115200);

    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("Failed to initialize power.....");
        assert(0);
    }

    // Set the led light on to indicate that the board has been turned on
    PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);


    // Turn off other unused power domains
    PMU.disableDC2();
    PMU.disableDC4();
    PMU.disableDC5();
    PMU.disableALDO1();

    PMU.disableALDO2();
    PMU.disableALDO3();
    PMU.disableALDO4();
    PMU.disableBLDO2();
    PMU.disableCPUSLDO();
    PMU.disableDLDO1();
    PMU.disableDLDO2();

    // ESP32S3 power supply cannot be turned off
    // PMU.disableDC1();

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
    }

    //! Do not turn off BLDO1, which controls the 3.3V power supply for level conversion.
    //! If it is turned off, it will not be able to communicate with the modem normally
    PMU.setBLDO1Voltage(3300);    //Set the power supply for level conversion to 3300mV
    PMU.enableBLDO1();

    //Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    //SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    //Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      //The antenna power must be turned on to use the GPS function

    /*********************************
     * step 2 : start modem
    ***********************************/

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);


    int retry = 0;
    while (!modem.testAT(1000)) {
        Serial.print(".");
        if (retry++ > 10) {
            // Pull down PWRKEY for more than 1 second according to manual requirements
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            delay(100);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            delay(1000);
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            retry = 0;
            Serial.println(F("***********************************************************"));
            Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
            Serial.println(F("***********************************************************\n"));
        }
    }
    Serial.println();
    Serial.println(F("***********************************************************"));
    Serial.println(F(" You can now send AT commands"));
    Serial.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
    Serial.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
    Serial.println(F(" DISCLAIMER: Entering AT commands without knowing what they do"));
    Serial.println(F(" can have undesired consiquinces..."));
    Serial.println(F(" AT Command datasheet : https://github.com/Xinyuan-LilyGO/LilyGo-T-SIM7080G/blob/master/datasheet/SIM7070_SIM7080_SIM7090%20Series_AT%20Command%20Manual_V1.05.pdf"));
    Serial.println(F("***********************************************************\n"));
    Serial.println();
}

void loop()
{
    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
    while (Serial.available()) {
        SerialAT.write(Serial.read());
    }
    delay(1);
}


















