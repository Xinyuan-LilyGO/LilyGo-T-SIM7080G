/**
 * Important claim -
 *      I am not the original author of this code.
 *      This repository includes codes that was created using examples from https://github.com/Xinyuan-LilyGO/LilyGo-T-SIM7080G/tree/master/examples
 *      I have adapted and modified this code to suit my specific needs, but I want to acknowledge and give credit to the original authors Lewis He (lewishe@outlook.com) for his work.
 *      Thank Lewis He (lewishe@outlook.com) for providing these helpful examples.
 *      Please note that my use of this code falls under MIT license or terms of use of the original source,
 *      I am not responsible for any damages or issues that may arise from the use of this code.
 * @brief     SIM7080G MQTT examples to access/publish/subscribe AWS IOT Core through AT&T NB-IOT SIM
 * @license   MIT
 * @date      2023-04-10
 * @version   1.0.0
 */

#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"
#include <SPI.h>
#include "./certs/AWS_root_CA.h"
#include "./certs/AWS_Client_CRT.h"
#include "./certs/AWS_Client_PSK.h"

// Define the clientID
String macStr = String(ESP.getEfuseMac(), HEX); // I use the ESP32 MAC address as the clientID
const char *clientID = macStr.c_str();

// ! Define the MQTT Server URL and Port for AWS IOT Core)
// Important learnings from https://github.com/botletics/SIM7000-LTE-Shield/issues/58
/* There are 3 important non-obvious steps. Whatever url AWS gives you for your IoT Core endpoint
(1) you have to strip out the "-ats" from it. So "xxxxxxx-ats.iot.us-east-1.amazonaws.com" becomes "xxxxxxx.iot.us-east-1.amazonaws.com"
(2) You have to use the legacy root certificate provided by AWS here under "VeriSign Endpoints (legacy) https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html".
(3) Some regions apparently don't support legacy certs,so please select the proper regions https://docs.aws.amazon.com/general/latest/gr/greengrass.html
such as xxxxxxx.iot.us-east-1.amazonaws.com and xxxxxxx.iot.us-west-2.amazonaws.com support the legacy certs.*/

// the URL of xxxxxxx-ats-iot.us-west-2.amazonaws.com can be found in the settings for endpoint in your AWS IOT Core account
const char MQTT_Server_URL[] = "xxxxxxxxx.iot.us-west-2.amazonaws.com"; // You must strip out the "-ats" from the endpoint original URL
const int MQTT_Server_Port = 8883;

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

enum
{
    MODEM_CATM = 1,
    MODEM_NB_IOT,
    MODEM_CATM_NBIOT,
};

bool level = false;

// retrieve the Power Saving Mode (PSM) timer value from the modem
void getPsmTimer()
{
    modem.sendAT("+CPSMS?"); // Send AT command to query PSM settings

    if (modem.waitResponse("+CPSMS:") != 1)
    {
        Serial.println("Failed to retrieve PSM timer");
        return;
    }

    String response = modem.stream.readStringUntil('\r');
    // Parse the response to extract PSM timer values (T3412, T3324)

    Serial.print("PSM Timer values: ");
    Serial.println(response);
}

/* Define the writeCaFiles function in this C++ code snippet takes four parameters: an integer index, a const char pointer filename, a const char pointer data, and a size_t variable length.
The function is designed to write the provided data to a file with a specified name. */
void writeCaFiles(int index, const char *filename, const char *data, size_t length)
{
    modem.sendAT("+CFSTERM");
    modem.waitResponse();

    modem.sendAT("+CFSINIT");
    if (modem.waitResponse() != 1)
    {
        Serial.println("INITFS FAILED");
        return;
    }
    // AT+CFSWFILE=<index>,<filename>,<mode>,<filesize>,<input time>
    // <index>
    //      Directory of AP filesystem:
    //      0 "/custapp/" 1 "/fota/" 2 "/datatx/" 3 "/customer/"
    // <mode>
    //      0 If the file already existed, write the data at the beginning of the
    //      file. 1 If the file already existed, add the data at the end o
    // <file size>
    //      File size should be less than 10240 bytes. <input time> Millisecond,
    //      should send file during this period or you can’t send file when
    //      timeout. The value should be less
    // <input time> Millisecond, should send file during this period or you can’t
    // send file when timeout. The value should be less than 10000 ms.

    size_t payloadLength = length;
    size_t totalSize = payloadLength;
    size_t alreadyWrite = 0;

    while (totalSize > 0)
    {
        size_t writeSize = totalSize > 10000 ? 10000 : totalSize;

        modem.sendAT("+CFSWFILE=", index, ",", "\"", filename, "\"", ",", !(totalSize == payloadLength), ",", writeSize, ",", 10000);
        modem.waitResponse(30000UL, "DOWNLOAD");
    REWRITE:
        modem.stream.write(data + alreadyWrite, writeSize);
        if (modem.waitResponse(30000UL) == 1)
        {
            alreadyWrite += writeSize;
            totalSize -= writeSize;
            Serial.printf("Writing:%d overage:%d\n", writeSize, totalSize);
        }
        else
        {
            Serial.println("Write failed!");
            delay(1000);
            goto REWRITE;
        }
    }

    Serial.println("Wirte done!!!");

    modem.sendAT("+CFSTERM");
    if (modem.waitResponse() != 1)
    {
        Serial.println("CFSTERM FAILED");
        return;
    }
}

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

    modem.setNetworkMode(2); // use automatic

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

    // ! Important - to use AT&T NB-IOT network, you must correctly configure as below ATT NB-IoT OneRate APN "m2mNB16.com.attz",
    // Otherwise ATT will assign a general APN like "m2mglobal" which seems blocks 443,8883,8884 ports.
    Serial.println("Configuring APN...");
    modem.sendAT("+CGDCONT=1,\"IP\",\"m2mNB16.com.attz\"");
    modem.waitResponse();

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
     * step 8 : import  rootCA
     ***********************************/

    Serial.println("............................................................................Step 8");
    Serial.println("start to write the root CA, device certificate and device private key to the modem ");

    writeCaFiles(3, "rootCA.pem", root_CA, strlen(root_CA));                 // root_CA is retrieved from AWS_root_CA.h, which is the  "VeriSign Class 3 Public Primary G5 root CA certificate" from https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html"
    writeCaFiles(3, "deviceCert.crt", Client_CRT, strlen(Client_CRT));       // Client_CRT is retrieved from AWS_Client_CRT.h, please download the device certificate from AWS IoT Core when you create the thing
    writeCaFiles(3, "devicePrivateKey.pem", Client_PSK, strlen(Client_PSK)); // Client_PSK is retrieved from AWS_Client_PSK.h, please download the device private key from AWS IoT Core when you create the thing

    // enable server sni
    snprintf(buffer, 1024, "+CSSLCFG=\"sni\",0,\"%s\"", MQTT_Server_URL);
    modem.sendAT(buffer);
    modem.waitResponse();

    Serial.println("Step 8 done !");
    /***********************************
     * step 9 : Configure TLS/SSL before connecting to AWS IOT Core
     ***********************************/

    Serial.println("............................................................................Step 9");
    Serial.println("start to configure the TLS/SSL parameters ");

    // If it is already connected, disconnect it first
    modem.sendAT("+SMDISC");
    modem.waitResponse();

    snprintf(buffer, 1024, "+SMCONF=\"URL\",%s,%d", MQTT_Server_URL, MQTT_Server_Port);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1)
    {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"CLIENTID\",\"%s\"", clientID);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1)
    {
        return;
    }

    // configure the socket keep-alive timer
    modem.sendAT("+SMCONF=\"KEEPTIME\",60");
    if (modem.waitResponse() != 1)
    {
        return;
    }
    modem.sendAT("+SMCONF=\"CLEANSS\",1");
    if (modem.waitResponse() != 1)
    {
        return;
    }
    // configure the QOS level for the MQTT connection
    modem.sendAT("+SMCONF=\"QOS\", 0");
    if (modem.waitResponse() != 1)
    {
        return;
    }
    // configure the SSL/TLS version for a secure socket connection
    modem.sendAT("+CSSLCFG=\"SSLVERSION\",0,3");
    if (modem.waitResponse() != 1)
    {
        return;
    }
    // convert the rootCA to the format required by the modem
    // <ssltype>
    //      1 QAPI_NET_SSL_CERTIFICATE_E
    //      2 QAPI_NET_SSL_CA_LIST_E
    //      3 QAPI_NET_SSL_PSK_TABLE_E
    // AT+CSSLCFG="CONVERT",2,"rootCA.crt"
    modem.sendAT("+CSSLCFG=\"CONVERT\",2,\"rootCA.pem\"");
    if (modem.waitResponse() != 1)
    {
        Serial.println("Convert rootCA.crt failed!");
    }

    // convert the deviceCert.crt and devicePrivateKey.pem to the format required by the modem
    modem.sendAT("+CSSLCFG=\"CONVERT\",1,\"deviceCert.crt\",\"devicePrivateKey.pem\"");
    if (modem.waitResponse() != 1)
    {
        Serial.println("Convert deviceCert.crt and devicePrivateKey.pem failed!");
    }

    /* enable SSL/TLS for a specific socket and set the root certificate authority (CA) and device certificate for secure communication.
    <index> SSL status, range: 0-6
            0 Not support SSL
            1-6 Corresponding to AT+CSSLCFG command parameter <ctindex>
            range 0-5
    <ca list> CA_LIST file name, Max length is 20 bytes
    <cert name> CERT_NAME file name, Max length is 20 bytes
    <len_calist> Integer type. Maximum length of parameter <ca list>.
    <len_certname> Integer type. Maximum length of parameter <cert name>. */
    modem.sendAT("+SMSSL=1,\"rootCA.pem\",\"deviceCert.crt\"");
    if (modem.waitResponse() != 1)
    {
        Serial.println("SSL with root CA and device certificate set up failed!");
    }
    else
    {
        Serial.println("SSL with root CA and device certificate set up successfully!");
    }
    Serial.println("Step 9 done !");
    /***********************************
     * step 10 : Connect AWS IOT Core, Publish to and Subscribe a topic from AWS IOT Core
     ***********************************/

    Serial.println("............................................................................Step 10");
    Serial.println("start to connect AWS IOT Core ");

    Serial.println("Connecting to AWS IOT Core ...");
    while (true)
    {
        modem.sendAT("+SMCONN");
        String response;
        ret = modem.waitResponse(60000UL, response);

        if (response.indexOf("ERROR") >= 0) // Check if the response contains "ERROR"
        {
            Serial.println("Connect failed");
            break; // Stop attempting to connect
        }
        else if (response.indexOf("OK") >= 0) // Check if the response contains "OK"
        {
            Serial.println("Connect successfully");
            break; // Exit the loop
        }
        else
        {
            Serial.println("No valid response, retrying connect ...");
            delay(1000);
        }
    }

    // Publish topic, below is an example, you need to create your own topic
    String pub_topic = "$aws/things/" + String(clientID) + "/device_connected";

    // Publish payload (JSON format), below is an example, you need to create your own payload
    String pub_payload = "{create your own payload}";

    char buffer[1024];
    // AT+SMPUB=<topic>,<content length>,<qos>,<retain><CR>message is entered Quit edit mode if message length equals to <content length>
    snprintf(buffer, 1024, "+SMPUB=\"%s\",%d,0,0", pub_topic.c_str(), pub_payload.length()); // ! qos must be set to 0 since AWS IOT Core does not support QoS 1 SIM7080G

    modem.sendAT(buffer);
    if (modem.waitResponse(">") == 1)
    {
        modem.stream.write(pub_payload.c_str(), pub_payload.length());
        Serial.println("");
        Serial.println(".......................................");
        Serial.println("Publishing below topic and payload now: ");
        Serial.println(pub_topic.c_str());
        Serial.println(pub_payload);

        if (modem.waitResponse(3000))
        {
            Serial.println("Send Packet success!");
        }
        else
        {
            Serial.println("Send Packet failed!");
        }
    }

    // Subscribe to the topic from AWS IoT Core, below is an example, you need to create your own subscribe topic on AWS IoT Core
    String sub_topic = "$aws/things/job_to_be_done";
    snprintf(buffer, 1024, "+SMSUB=\"%s\",0", sub_topic.c_str());
    modem.sendAT(buffer);
    if (modem.waitResponse(1000, "OK") == 1)
    {
        String result = modem.stream.readStringUntil('\r');
        Serial.println();
        Serial.println(".......................................");
        Serial.println("Received message from AWS IoT Core: " + result);
    }
    else
    {
        Serial.println("Failed to subscribe message");
    }
    Serial.println("Step 10 done !");
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
