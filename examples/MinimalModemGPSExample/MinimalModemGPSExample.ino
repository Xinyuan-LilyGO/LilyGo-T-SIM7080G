/**
 * @file      MinimalModemGPSExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU PMU;

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#define TINY_GSM_RX_BUFFER 1024

#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

float lat2      = 0;
float lon2      = 0;
float speed2    = 0;
float alt2      = 0;
int   vsat2     = 0;
int   usat2     = 0;
float accuracy2 = 0;
int   year2     = 0;
int   month2    = 0;
int   day2      = 0;
int   hour2     = 0;
int   min2      = 0;
int   sec2      = 0;
bool  level     = false;

void setup()
{

    Serial.begin(115200);

    // Start while waiting for Serial monitoring
    while (!Serial);

    delay(3000);

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

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
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

    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    int retry = 0;
    while (!modem.testAT(1000)) {
        Serial.print(".");
        if (retry++ > 15) {
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

    /*********************************
     * step 3 : start modem gps function
    ***********************************/

// Uncomment the following section for advanced GPS configuration
// #define ADVANCED_GPS_CONFIG

#ifdef ADVANCED_GPS_CONFIG
    // When configuring GNSS, you need to stop GPS first
    modem.disableGPS();
    delay(500);

    /*
    ! GNSS Work Mode Set
    <gps mode> GPS work mode.
        0 Stop GPS NMEA out.
        1 Start GPS NMEA out.
    <glo mode> GLONASS work mode.
        0 Stop GLONASS NMEA out.
        1 Start GLONASS NMEA out.
    <bd mode> BEIDOU work mode.
        0 Stop BEIDOU NMEA out.
        1 Start BEIDOU NMEA out.
    <gal mode> GALILEAN work mode.
        0 Stop GALILEAN NMEA out.
        1 Start GALILEAN NMEA out.
    <qzss mode> QZSS work mode.
        0 Stop QZSS NMEA out.
        1 Start QZSS NMEA out.
    */
    // GNSS Work Mode Set GPS+BEIDOU
    modem.sendAT("+CGNSMOD=1,0,1,0,0");
    modem.waitResponse();


    /*
    GNSS Command,For more parameters, see <SIM7070_SIM7080_SIM7090 Series_AT Command Manual> 212 page.
    <minInterval> range: 1000-60000 ms
     minInterval is the minimum time interval in milliseconds that must elapse between position reports. default value is 1000.
    <minDistance> range: 0-1000
     Minimum distance in meters that must be traversed between position reports. Setting this interval to 0 will be a pure time-based tracking/batching.
    <accuracy>:
        0 Accuracy is not specified, use default.
        1 Low Accuracy for location is acceptable.
        2 Medium Accuracy for location is acceptable.
        3 Only High Accuracy for location is acceptable.
    */
    // minInterval = 1000,minDistance = 0,accuracy = 0
    modem.sendAT("+SGNSCMD=2,1000,0,0");
    modem.waitResponse();

    // Turn off GNSS.
    modem.sendAT("+SGNSCMD=0");
    modem.waitResponse();

    delay(500);
#endif

    // GPS function needs to be enabled for the first use
    if (modem.enableGPS() == false) {
        Serial.println("Modem enable gps function failed!!");
        while (1) {
            delay(5000);
        }
    }
    
    Serial.println("GPS enabled, waiting for GPS fix...");
    Serial.println("This may take several minutes for first fix...");

}

void loop()
{
    static unsigned long lastPrint = 0;
    static int fixAttempts = 0;
    
    if (modem.getGPS(&lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2,
                     &year2, &month2, &day2, &hour2, &min2, &sec2)) {
        Serial.println();
        Serial.print("GPS Fix #"); Serial.print(++fixAttempts); Serial.println(":");
        Serial.print("Latitude: "); Serial.print(String(lat2, 8)); Serial.println();
        Serial.print("Longitude: "); Serial.print(String(lon2, 8)); Serial.println();
        Serial.print("Speed: "); Serial.print(speed2); Serial.println(" knots");
        Serial.print("Altitude: "); Serial.print(alt2); Serial.println(" meters");
        Serial.print("Satellites in view: "); Serial.print(vsat2); Serial.println();
        Serial.print("Satellites used: "); Serial.print(usat2); Serial.println();
        Serial.print("Accuracy: "); Serial.print(accuracy2); Serial.println();
        Serial.print("Date/Time: ");
        Serial.print(year2); Serial.print("-");
        Serial.print(month2); Serial.print("-");
        Serial.print(day2); Serial.print(" ");
        Serial.print(hour2); Serial.print(":");
        Serial.print(min2); Serial.print(":");
        Serial.print(sec2); Serial.println();
        Serial.println();

        // After successful positioning, the PMU charging indicator flashes quickly
        PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_4HZ);
        lastPrint = millis();
    } else {
        // Blinking PMU charging indicator
        PMU.setChargingLedMode(level ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
        level ^= 1;
        
        // Print status every 10 seconds
        if (millis() - lastPrint > 10000) {
            Serial.print("Waiting for GPS fix... Attempts: ");
            Serial.println(fixAttempts);
            lastPrint = millis();
        }
    }
    delay(1000);
}
