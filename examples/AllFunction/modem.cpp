/**
 * @file      modem.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-21
 *
 */
// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>
#include "utilities.h"

#if defined(USING_MODEM)

const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register to. The GPRS service is disabled, the UE is allowed to attach for GPRS if requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an operator to register to. The GPRS service is enabled, but an allowable PLMN is currently not available. The UE will start a GPRS attach as soon as an allowable PLMN is available.",
    "Registration denied, the GPRS service is disabled, the UE is not allowed to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

// Your GPRS credentials, if any
const char apn[]      = "YourAPN";
const char gprsUser[] = "";
const char gprsPass[] = "";

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(Serial1);
#endif

bool getLocation();
void locationTask(void *);

void setupModem()
{
    Serial.println("Initializing modem...");

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);

    // digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    // delay(100);
    // digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    // delay(1000);
    // digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    int retryCount = 0;
    int retry = 0;
    while (!modem.testAT(1000))
    {
        Serial.print(".");
        if (retry++ > 5)
        {
            Serial.println("Warn : try reinit modem!");
            // Pull down PWRKEY for more than 1 second according to manual requirements
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            delay(100);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            delay(1000);
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            // modem.sendAT("+CRESET");
            retry = 0;
        }
    }

    Serial.print("Modem started!");

    if (modem.getSimStatus() != SIM_READY)
    {
        Serial.println("SIM Card is not insert!!!");
        return;
    }

    SIM70xxRegStatus s;
    do
    {
        s = modem.getRegistrationStatus();
        int16_t sq = modem.getSignalQuality();

        if (s == REG_SEARCHING)
        {
            Serial.print("Searching...");
        }
        else
        {
            Serial.print("Other code:");
            Serial.print(s);
            break;
        }
        Serial.print("  Signal:");
        Serial.println(sq);
        delay(1000);
    } while (s != REG_OK_HOME && s != REG_OK_ROAMING);

    Serial.println();
    Serial.print("Network register info:");
    if (s >= sizeof(register_info) / sizeof(*register_info))
    {
        Serial.print("Other result = ");
        Serial.println(s);
    }
    else
    {
        Serial.println(register_info[s]);
    }

    if (modem.enableGPS() == false)
    {
        Serial.println("Enable gps failed!");
    }

    xTaskCreate(locationTask, "gps", 4096, NULL, 10, NULL);
}

void locationTask(void *)
{
    Serial.println("Get location");
    while (1)
    {
        if (getLocation())
        {
            Serial.println();
            break;
        }
        Serial.print(".");
        delay(2000);
    }
    vTaskDelete(NULL);
}

bool getLocation()
{
    float lat      = 0;
    float lon      = 0;
    float speed    = 0;
    float alt      = 0;
    int   vsat     = 0;
    int   usat     = 0;
    float accuracy = 0;
    int   year     = 0;
    int   month    = 0;
    int   day      = 0;
    int   hour     = 0;
    int   min      = 0;
    int   sec      = 0;

    if (modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy,
                     &year, &month, &day, &hour, &min, &sec))
    {
        Serial.println();
        Serial.printf("lat:%.8f\tlon:%.8f\n", lat, lon);
        Serial.printf("speed:%f\talt:%f\n", speed, alt);
        Serial.printf("year:%d month:%d day:%d hour:%d min:%d sec:%d\n", year, month, day, hour, min, sec);
        Serial.println();
        return true;
    }
    return false;
}

#else
void setupModem()
{
}
#endif
