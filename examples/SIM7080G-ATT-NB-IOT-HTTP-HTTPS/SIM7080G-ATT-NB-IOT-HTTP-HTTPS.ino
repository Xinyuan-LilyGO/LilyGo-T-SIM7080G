/**
 * Important claim -
 *      I am not the original author of this code.
 *      This repository includes codes that was created using examples from https://github.com/Xinyuan-LilyGO/LilyGo-T-SIM7080G/tree/master/examples
 *      I have adapted and modified this code to suit my specific needs, but I want to acknowledge and give credit to the original authors Lewis He (lewishe@outlook.com) for his work.
 *      Thank Lewis He (lewishe@outlook.com) for providing these helpful examples.
 *      Please note that my use of this code falls under MIT license or terms of use of the original source,
 *      I am not responsible for any damages or issues that may arise from the use of this code.
 * @brief     Support T-SIM7080G to access a testing HTTP/HTTPS server through AT&T NB-IOT radio
 * @license   MIT
 * @date      2023-04-10
 * @version   1.0.0
 */

#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"
#include <SPI.h>

char buffer[1024] = {0};

XPowersPMU PMU;

// See all AT commands, if wanted

#define TINY_GSM_RX_BUFFER 1024
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register to.The GPRS service is disabled, the UE is allowed to attach for GPRS if requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an operator to register to. The GPRS service is enabled, but an allowable PLMN is currently not available. The UE will start a GPRS attach as soon as an allowable PLMN is available.",
    "Registration denied, The GPRS service is disabled, the UE is not allowed to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

/* MODEM_CATM = 1: Represents a Cat-M (LTE-M or eMTC) modem, which is a type of low-power wide-area network (LPWAN) designed for Internet of Things (IoT) devices. The constant value is set to 1.
MODEM_NB_IOT: Represents an NB-IoT (Narrowband IoT) modem, another type of LPWAN that is designed for IoT devices. The constant value is implicitly set to 2, as it follows MODEM_CATM.
MODEM_CATM_NBIOT: Represents a modem that supports both Cat-M and NB-IoT connectivity. The constant value is implicitly set to 3, as it follows MODEM_NB_IOT. */
enum
{
    MODEM_CATM = 1,
    MODEM_NB_IOT,
    MODEM_CATM_NBIOT,
};

bool level = false;

/* Initialization modem */
void setup()
{
    Serial.begin(115200);

    // Start while waiting for Serial monitoring
    while (!Serial)
        ;
    delay(3000);
    Serial.println();

    /***********************************
     *  step 1 : Initialize power chip, turn on modem and gps antenna power channel
     ***********************************/

    Serial.println("............................................................................Step 1");
    Serial.println("start to initialize power chip, turn on modem and gps antenna power channel ");
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL))
    {
        Serial.println("Failed to initialize power.....");
        while (1)
        {
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
    PMU.setDC3Voltage(3000); // SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    // Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2(); // The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();

    Serial.println("Step 1 done !");
    /***********************************
     * step 2 : start modem
     ***********************************/

    Serial.println("............................................................................Step 2");
    Serial.println("start to initialize T-SIM7080G modem now ");

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);

    int retry = 0;
    while (!modem.testAT(1000))
    {
        Serial.print(".");
        if (retry++ > 6)
        {
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
    Serial.println("T-SIM7080G modem well started!");
    Serial.println("Step 2 done !");
    /***********************************
     * step 3 : Check if the SIM card is inserted
     ***********************************/

    Serial.println("............................................................................Step 3");
    Serial.println("start to check if the SIM card is inserted ");

    String result;

    if (modem.getSimStatus() != SIM_READY)
    {
        Serial.println("SIM Card is not insert!!!");
        return;
    }
    else
    {
        Serial.println("SIM Card is insert !");
    }
    Serial.println("Step 3 done !");
    /***********************************
     * step 4 : Set the network mode to NB-IOT
     ***********************************/

    Serial.println("............................................................................Step 4");
    Serial.println("start to set the network mode to NB-IOT ");

    modem.setNetworkMode(38); // 2 for Auto, 38 for LTE only
    
    modem.setPreferredMode(MODEM_NB_IOT);

    uint8_t pre = modem.getPreferredMode();

    uint8_t mode = modem.getNetworkMode();

    Serial.printf("getNetworkMode:%u getPreferredMode:%u\n", mode, pre);

    Serial.println("Step 4 done !");
    /***********************************
     * step 5 : Start network registration
     ***********************************/

    Serial.println("............................................................................Step 5");
    Serial.println("Start to perform network registration, configure APN and ping 8.8.8.8");

    modem.sendAT("+COPS=0,0,\"AT&T\",9");
    modem.waitResponse();

    modem.sendAT("+CBANDCFG=\"NB-IoT\",12");  // 4: 700MHz for T-mobile, 12: 850MHz for AT&T  
    modem.waitResponse();

    modem.sendAT("+CNBS=1");

    // Important ! 
    // To use AT&T NB-IOT network, you must correctly configure as below ATT NB-IoT OneRate data plan APN "m2mNB16.com.attz",
    // Otherwise ATT will assign a general APN like "m2mglobal" which seems blocks 443,8883,8884 ports.
    Serial.println("Configuring APN...");
    modem.sendAT("+CGDCONT=1,\"IP\",\"m2mNB16.com.attz\"");
    modem.waitResponse();

    SIM70xxRegStatus s;
    do
    {
        s = modem.getRegistrationStatus();
        if (s != REG_OK_HOME && s != REG_OK_ROAMING)
        {
            Serial.print(".");
            PMU.setChargingLedMode(level ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
            level ^= 1;
            delay(1000);
        }

    } while (s != REG_OK_HOME && s != REG_OK_ROAMING);

    Serial.println();
    Serial.print("Network register info:");
    Serial.println(register_info[s]);

    modem.sendAT("+CGNAPN");
    if (modem.waitResponse(10000L) != 1)
    {
        Serial.println("Get APN Failed!");
        return;
    }

    Serial.println("Step 5 done !");
    /***********************************
     * step 6 : Check if the network bearer is activated
     ***********************************/

    Serial.println("............................................................................Step 6");
    Serial.println("start to activate the network bearer");

    // Check the status of the network bearer
    Serial.println("Checking the status of network bearer ...");
    modem.sendAT("+CNACT?"); // Send the AT command to query the network bearer status
    String response;
    int8_t ret = modem.waitResponse(10000UL, response); // Wait for the response with a 10-second timeout

    bool alreadyActivated = false;
    if (response.indexOf("+CNACT: 0,1") >= 0) // Check if the response contains "+CNACT: 0,1" indicating bearer is activated
    {
        Serial.println("Network bearer is already activated");
        alreadyActivated = true;
    }
    else if (response.indexOf("+CNACT: 0,0") >= 0) // Check if the response contains "+CNACT: 0,0" indicating bearer is deactivated
    {
        Serial.println("Network bearer is not activated");
    }

    if (!alreadyActivated)
    {
        // Activating network bearer
        Serial.println("Activating network bearer ...");
        modem.sendAT("+CNACT=0,1"); // Send the AT command to activate the network bearer
        response = "";
        ret = modem.waitResponse(10000UL, response); // Wait for the response with a 10-second timeout

        if (response.indexOf("ERROR") >= 0) // Check if the response contains "ERROR"
        {
            Serial.println("Network bearer activation failed");
        }
        else if (response.indexOf("OK") >= 0) // Check if the response contains "OK"
        {
            Serial.println("Activation in progress, waiting for network response...");

            // Wait for the "+APP PDP: 0,ACTIVE" response
            bool activationConfirmed = false;
            unsigned long startTime = millis();
            while (millis() - startTime < 60000UL) // Wait for 60 seconds
            {
                if (modem.stream.available())
                {
                    response = modem.stream.readString();
                    if (response.indexOf("+APP PDP: 0,ACTIVE") >= 0)
                    {
                        activationConfirmed = true;
                        break;
                    }
                }
                delay(100);
            }
            if (activationConfirmed)
            {
                Serial.println("Network bearer is activated successfully !");
            }
            else
            {
                Serial.println("No network response within the timeout");
            }
        }
        else
        {
            Serial.println("No valid response");
        }
    }
    // Ping the Google DNS server
    modem.sendAT("+SNPING4=\"8.8.8.8\",1,16,5000");
    if (modem.waitResponse(10000L) != 1)
    {
        Serial.println("Ping Failed!");
        return;
    }
    else
    {
        Serial.println(response);
    }

    Serial.println("Step 6 done !");
    /***********************************
     * step 7 : Get the modem, SIM and network information
     ***********************************/

    Serial.println("............................................................................Step 7");
    Serial.println("show the information of the Modem, SIM and network  ");

    Serial.println("T-SIM7080G Firmware Version: ");
    modem.sendAT("+CGMR");
    if (modem.waitResponse(10000L) != 1)
    {
        Serial.println("Get Firmware Version Failed!");
    }
    else
    {
        Serial.println(response);
    }
    String ccid = modem.getSimCCID();
    Serial.print("CCID:");
    Serial.println(ccid);

    String imei = modem.getIMEI();
    Serial.print("IMEI:");
    Serial.println(imei);

    String imsi = modem.getIMSI();
    Serial.print("IMSI:");
    Serial.println(imsi);

    String cop = modem.getOperator();
    Serial.print("Operator:");
    Serial.println(cop);

    IPAddress local = modem.localIP();
    Serial.print("Local IP:");
    Serial.println(local);

    int csq = modem.getSignalQuality();
    Serial.print("Signal quality:");
    Serial.println(csq);

    modem.sendAT("+CGNAPN");
    if (modem.waitResponse(10000L) != 1)
    {
        Serial.println("Get APN Failed!");
        return;
    }

    modem.sendAT("+CCLK?");
    if (modem.waitResponse(10000L) != 1)
    {
        Serial.println("Get time Failed!");
        return;
    }
    Serial.println("Step 7 done !");

    /***********************************
     * step 8 : Access HTTP Server at demo4570913.mockable.io (this is a test server created by my account on mockable.io)
     ***********************************/

    Serial.println("............................................................................Step 8");
    Serial.println("start to connect the HTTP server ");

    TinyGsmClient client(modem, 0);

    const int HTTP_Port = 80;
    const char *HTTP_Server = "demo4570913.mockable.io";

    const char *Resource = "/"; // Assuming you want to access the root path

    Serial.printf("Connecting to %s", HTTP_Server);
    if (!client.connect(HTTP_Server, HTTP_Port))
    {
        Serial.println("... failed");
    }
    else
    {
        // Make a HTTP GET request:
        client.print(String("GET ") + Resource + " HTTP/1.1\r\n");
        client.print(String("Host: ") + HTTP_Server + "\r\n");
        client.print("Connection: close\r\n\r\n");

        // Wait for data to arrive
        uint32_t start = millis();
        while (client.connected() && !client.available() &&
               millis() - start < 30000L)
        {
            delay(100);
        };

        // Read data
        start = millis();
        char logo[640] = {
            '\0',
        };
        int read_chars = 0;
        while (client.connected() && millis() - start < 10000L)
        {
            while (client.available())
            {
                logo[read_chars] = client.read();
                logo[read_chars + 1] = '\0';
                read_chars++;
                start = millis();
            }
        }
        Serial.println(logo);
        Serial.print("#####  RECEIVED:");
        Serial.print(strlen(logo));
        Serial.println("CHARACTERS");
        client.stop();
    }

    Serial.println("Step 8 done !");
    /***********************************
     * step 9 : Access HTTPS Server at demo4570913.mockable.io (this is a test server created by my account on mockable.io)
     ***********************************/

    Serial.println("............................................................................Step 9");
    Serial.println("start to connect the HTTPS server ");

    const int HTTPS_Port = 443;
    const char *HTTPS_Server = "demo4570913.mockable.io";

    TinyGsmClientSecure secureClient(modem, 0);
    Serial.printf("Connecting securely to %s", HTTPS_Server);
    if (!secureClient.connect(HTTPS_Server, HTTPS_Port))
    {
        Serial.println("... failed");
    }
    else
    {
        // Make a HTTPS GET request:
        secureClient.print(String("GET ") + Resource + " HTTP/1.1\r\n");
        secureClient.print(String("Host: ") + HTTPS_Server + "\r\n");
        secureClient.print("Connection: close\r\n\r\n");

        // Wait for data to arrive
        uint32_t startS = millis();
        while (secureClient.connected() && !secureClient.available() &&
               millis() - startS < 30000L)
        {
            delay(100);
        };

        // Read data
        startS = millis();
        char logoS[640] = {
            '\0',
        };
        int read_charsS = 0;
        while (secureClient.connected() && millis() - startS < 10000L)
        {
            while (secureClient.available())
            {
                logoS[read_charsS] = secureClient.read();
                logoS[read_charsS + 1] = '\0';
                read_charsS++;
                startS = millis();
            }
        }
        Serial.println(logoS);
        Serial.print("#####  RECEIVED:");
        Serial.print(strlen(logoS));
        Serial.println("CHARACTERS");
        secureClient.stop();
    }

    Serial.println("Step 9 done !");
}

void loop()
{
    while (1)
    {
        while (Serial1.available())
        {
            Serial.write(Serial1.read());
        }
        while (Serial.available())
        {
            Serial1.write(Serial.read());
        }
    }
}
