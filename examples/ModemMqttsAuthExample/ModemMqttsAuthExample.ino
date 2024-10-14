/**
 * @file      ModemMqttsAuthExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-03-31
 *
 */

// ! It is only used for writing and converting demonstration certificates, and the connection has not been fully tested for the time being


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
#include "utilities.h"


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register to.The GPRS service is disabled, the UE is allowed to attach for GPRS if requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an operator to register to. The GPRS service is enabled, but an allowable PLMN is currently not available. The UE will start a GPRS attach as soon as an allowable PLMN is available.",
    "Registration denied, The GPRS service is disabled, the UE is not allowed to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

enum {
    MODEM_CATM = 1,
    MODEM_NB_IOT,
    MODEM_CATM_NBIOT,
};


const char *client_cert_key =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"\
    "MIIEpAIBAAKCAQEAznRwIp3H0QWGbSAcYylJMLXreSHjWGzqZqulH0iHyWpmaWuc\r\n"\
    "Ys+nXfpwEMQqo7DLuLNPDeE4LAUtT/gF2v8vFwdeMadwJfUHsHJJT9lY/F+EICxU\r\n"\
    "sLFtz67YNTAof4mM9R+58aHLhvPPCVBnF4lW2chnCFlQq/duyGf8E+e5h4DenCPP\r\n"\
    "S+YOnl0ZqZmLf9z49BAuLHeQOYb+/IdLCQ4T6xzmCCxAVopYojyYAIH+HuEEzHsl\r\n"\
    "M7Bk5PUa94cRjbW6pFoEIv20HOoefDE0Ss3jlFIcLoUSj7wg7uurjdnamDyPYSKP\r\n"\
    "TTISgSZTMeepFJRuulV/TDcJXlN16LcdkphdswIDAQABAoIBAQCAEk1JvBUrfkDw\r\n"\
    "yx2xCfiERiYoA7SzswUJ4erR6D2L3fxf40ilJ2oM64G/dOF6n/8QufMNiiw9aoy8\r\n"\
    "cgC35NuCbqipJtR0R3eYpp0B8rO4A0zEZJu47KhRUIaBIDnPPDX8Dc1cruDB+9bw\r\n"\
    "UTdSq3j8Ksx3qmhX9Wt82LzQYEyAWHy+V0+ARy59uYReff5h651kUB2mIUDIu5sF\r\n"\
    "Yj+vPJs3iU1rfewG6oz1fZ4SHX93l6dc3oVhG+e2Ta9+c//VKE7wKc5rAlrj6QTL\r\n"\
    "j8tJIkqRP8PPWobCzxGsCTudlhXnTsdz9fZKTaaNQ3/TYH1uPtoRAG6dlMJFvnA1\r\n"\
    "ryEyqs75AoGBAPA37knS7pa2dWZalApW3Ee5tn6lIUdmvWDQKfxbcIWeMCZdPIUV\r\n"\
    "i+K2PeUsta1xcZuvZZocGofyRd1w6dMnqBmbpXuFVA90I3yzfiPkAMyQXIETyaXo\r\n"\
    "Lkttw+o1jL9p3Oqsa7b2iZWuJ1rhIt2YPdgxFXjvXmXUGW78KKdVdksPAoGBANwE\r\n"\
    "qOTapWCWzrHkTkwfPHqyhFdNKSSCYu4Eldi/O3kA5jGkh8XaQbF73m+g7aJ1/YZ9\r\n"\
    "2WM4+dvZ3BTtzfYCNXmjQFiabw0hGPDBgtF6Kq60QFSnclK4yIypVH2UNjBPn4h6\r\n"\
    "hab2gucCWSdI5uTLaA9XxbXQE7LBoOZMkb5IZFMdAoGAF/XsPc7dX4kZkrkMNS/O\r\n"\
    "zxS2IFHGTQHxd9urpHFWeu15bgo0xC2PA3EcIWThRkifhWDsaH/PIapHz7u4hwhY\r\n"\
    "mx1MV1LIPLZf58tblKbkcYMgxvs7TOIo4/sx5IWs4Vbk4z+JivlyZcy2PjlgqevK\r\n"\
    "l2rl8mcl6lBKrShXwcEjiH8CgYEAzcs9/vHUhkgJBbO62NDOzSV1TANMXG3pAyEe\r\n"\
    "2CHnCwOgTQbMSHAhylVGdbtdCvy6KrZEQ97jNpTMmnbkkxr10dS1NyscfHdc0LTw\r\n"\
    "G+fdTJQlKAmHkYBtdcRc1yluljmjyxBvOwCQ6Gr14Rz7ez4XE2LR94GtKyKZ0VAF\r\n"\
    "cqpbzLECgYAUh1n0qiw6eNnhZnRsTJqfO4oS2XhBd19ah4Q8x92JsWaYViVeX97f\r\n"\
    "pTvCRl/QGIGsbi4SSraC+4ZWMauSHf5YLD12/PjoG8QnkJT4dOF31aW4sVTBOcah\r\n"\
    "XuYTiQs9udhCN6BL/5EzWa8TCXZxK6VXKExn+fttKPiYkA/Jx4vppg==\r\n"\
    "-----END RSA PRIVATE KEY-----\r\n";


const char *client_cert =
    "-----BEGIN CERTIFICATE-----\r\n"\
    "MIIDjjCCAnagAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix\r\n"\
    "FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE\r\n"\
    "CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y\r\n"\
    "ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMzAzMzExMTAy\r\n"\
    "MzVaFw0yMzA2MjkxMTAyMzVaMGgxCzAJBgNVBAYTAkFVMQ8wDQYDVQQIDAZMaWx5\r\n"\
    "R28xDjAMBgNVBAcMBUNoaW5hMQ4wDAYDVQQKDAVDaGluYTEOMAwGA1UEAwwFQ2hp\r\n"\
    "bmExGDAWBgkqhkiG9w0BCQEWCWxpbHlnby5jYzCCASIwDQYJKoZIhvcNAQEBBQAD\r\n"\
    "ggEPADCCAQoCggEBAM50cCKdx9EFhm0gHGMpSTC163kh41hs6marpR9Ih8lqZmlr\r\n"\
    "nGLPp136cBDEKqOwy7izTw3hOCwFLU/4Bdr/LxcHXjGncCX1B7BySU/ZWPxfhCAs\r\n"\
    "VLCxbc+u2DUwKH+JjPUfufGhy4bzzwlQZxeJVtnIZwhZUKv3bshn/BPnuYeA3pwj\r\n"\
    "z0vmDp5dGamZi3/c+PQQLix3kDmG/vyHSwkOE+sc5ggsQFaKWKI8mACB/h7hBMx7\r\n"\
    "JTOwZOT1GveHEY21uqRaBCL9tBzqHnwxNErN45RSHC6FEo+8IO7rq43Z2pg8j2Ei\r\n"\
    "j00yEoEmUzHnqRSUbrpVf0w3CV5Tdei3HZKYXbMCAwEAAaMaMBgwCQYDVR0TBAIw\r\n"\
    "ADALBgNVHQ8EBAMCBeAwDQYJKoZIhvcNAQELBQADggEBAA9L45aImACmKyyftO9I\r\n"\
    "xWIIGMoeKVXVH2LBPPsgElKY73PNe19p4NqRe65+V1lre4TxtWvlPbA1BSfLtzgi\r\n"\
    "jMYZHxQi7Bm7mgU1prSgsJw028x4SkWBhC50Y7KNL95IFOCGQvwSgcbnmtviRwrR\r\n"\
    "dQ2zuAd3f55nIOmUNUKlvuNfPZgaTdQjCAjYLAA0QnVgd5bUOZRsO1BrDQz3stua\r\n"\
    "ttQq1rKvzr3nRfLBSv8MD8HpMyvxOEQH1qUUNoN+bK7GzSVqB4wHkN/BEGNzXHah\r\n"\
    "Nz9+lxEBcBzJK2NbVxvkhpkXLhH6t+rTqdQmbTJ6OZcaUdNWI3MHnu35NMxCVewH\r\n"\
    "EQE=\r\n"\
    "-----END CERTIFICATE-----\r\n";

const char *ca_cert =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\r\n"
    "BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\r\n"
    "A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\r\n"
    "BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\r\n"
    "by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\r\n"
    "BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\r\n"
    "MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\r\n"
    "dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\r\n"
    "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\r\n"
    "UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\r\n"
    "Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\r\n"
    "s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\r\n"
    "3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\r\n"
    "E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\r\n"
    "MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\r\n"
    "6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\r\n"
    "BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\r\n"
    "6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\r\n"
    "+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\r\n"
    "sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\r\n"
    "LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\r\n"
    "m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\r\n"
    "-----END CERTIFICATE-----\r\n";


// Your GPRS credentials, if any
const char apn[] = "CMNBIOT";
const char gprsUser[] = "";
const char gprsPass[] = "";

// cayenne server address and port
const char server[]   = "test.mosquitto.org";
const int  port       = 8884;
char buffer[1024]     = {0};


char username[] = "";
char password[] = "";
char clientID[] = "SIM7080_ClientID";

bool isConnect()
{
    modem.sendAT("+SMSTATE?");
    if (modem.waitResponse("+SMSTATE: ")) {
        String res =  modem.stream.readStringUntil('\r');
        return res.toInt();
    }
    return false;
}


bool setCertificate(const char *cert_pem,
                    const char *client_cert_pem = NULL,
                    const char *client_key_pem = NULL)
{
    modem.sendAT("+CFSINIT");
    modem.waitResponse();

    if (cert_pem) {
        modem.sendAT("+CFSWFILE=3,\"ca.crt\",0,", strlen(cert_pem), ",", 10000UL);
        if (modem.waitResponse(10000UL, "DOWNLOAD") == 1) {
            modem.stream.write(cert_pem);
        }
        if (modem.waitResponse(5000UL) != 1) {
            DBG("Write ca_cert pem failed!");
            return false;
        }

    }


    if (client_cert_pem) {
        modem.sendAT("+CFSWFILE=3,\"cert.pem\",0,", strlen(client_cert_pem ), ",", 10000UL);
        if (modem.waitResponse(10000UL, "DOWNLOAD") == 1) {
            modem.stream.write(client_cert_pem );
        }
        if (modem.waitResponse(5000) != 1) {
            DBG("Write cert pem failed!"); return false;
        }
    }

    if (client_key_pem) {
        modem.sendAT("+CFSWFILE=3,\"key_cert.pem\",0,", strlen(client_key_pem), ",", 10000UL);
        if (modem.waitResponse(10000UL, "DOWNLOAD") == 1) {
            modem.stream.write(client_key_pem);
        }
        if (modem.waitResponse(5000) != 1) {
            DBG("Write key_cert failed!"); return false;
        }
    }

    if (cert_pem) {
        // <ssltype>
        // 1 QAPI_NET_SSL_CERTIFICATE_E
        // 2 QAPI_NET_SSL_CA_LIST_E
        // 3 QAPI_NET_SSL_PSK_TABLE_E
        // AT+CSSLCFG="CONVERT",2,"ca_cert.pem"
        modem.sendAT("+CSSLCFG=\"CONVERT\",2,\"ca.crt\"");
        if (modem.waitResponse(5000UL) != 1) {
            DBG("convert ca_cert pem failed!");
            return false;
        }
    }

    if (client_cert_pem && client_key_pem) {
        modem.sendAT("+CSSLCFG=\"CONVERT\",1,\"cert.pem\",\"key_cert.pem\"");
        if (modem.waitResponse(5000) != 1) {
            DBG("convert client_cert_pem&client_key_pem pem failed!");
            return false;
        }

        //! AT+SMSSL=<index>,<ca list>,<cert name> , depes AT+CSSLCFG
        //! <index>SSL status, range: 0-6
        //!     0 Not support SSL
        //!     1-6 Corresponding to AT+CSSLCFG command parameter <ctindex>
        //! <ca list>CA_LIST file name, length 20 byte
        //! <cert name>CERT_NAME file name, length 20 byte
        modem.sendAT("+SMSSL=1,\"ca_cert.pem\",\"cert.pem\"");
        modem.waitResponse(3000);
    }

    modem.sendAT("+CFSTERM");
    modem.waitResponse();

    return true;

}


void setup()
{
    bool res;

    Serial.begin(115200);

    //Start while waiting for Serial monitoring
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

    //Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    //SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    //Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      //The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();


    /*********************************
     * step 2 : start modem
    ***********************************/

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);

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

    delay(3000);

    /*********************************
     * step 3 : Check if the SIM card is inserted
    ***********************************/
    String result ;


    if (modem.getSimStatus() != SIM_READY) {
        Serial.println("SIM Card is not insert!!!");
        return ;
    }

    // Disable RF
    modem.sendAT("+CFUN=0");
    if (modem.waitResponse(20000UL) != 1) {
        Serial.println("Disable RF Failed!");
    }


    /*********************************
     * step 4 : Set the network mode to NB-IOT
    ***********************************/

    modem.setNetworkMode(2);    //use automatic

    modem.setPreferredMode(MODEM_NB_IOT);

    uint8_t pre = modem.getPreferredMode();

    uint8_t mode = modem.getNetworkMode();

    Serial.printf("getNetworkMode:%u getPreferredMode:%u\n", mode, pre);


    //Set the APN manually. Some operators need to set APN first when registering the network.
    modem.sendAT("+CGDCONT=1,\"IP\",\"", apn, "\"");
    if (modem.waitResponse() != 1) {
        Serial.println("Set operators apn Failed!");
        return;
    }

    // Enable RF
    modem.sendAT("+CFUN=1");
    if (modem.waitResponse(20000UL) != 1) {
        Serial.println("Enable RF Failed!");
    }


    /*********************************
    * step 5 : Wait for the network registration to succeed
    ***********************************/
    SIM70xxRegStatus s;
    do {
        s = modem.getRegistrationStatus();
        if (s != REG_OK_HOME && s != REG_OK_ROAMING) {
            Serial.print(".");
            delay(1000);
        }

    } while (s != REG_OK_HOME && s != REG_OK_ROAMING) ;

    Serial.println();
    Serial.print("Network register info:");
    Serial.println(register_info[s]);

    do {
        modem.sendAT("+CGATT?");
        res = modem.waitResponse(3000UL);
        if (res != 1) {
            Serial.println("Check PS service. 1 indicates PS has attached failed");
            delay(1500);
        }
    } while (res != 1);
    Serial.println("Check PS service. 1 indicates PS has attached successed!");


    //Before activation please use AT+CNCFG to set APN\user name\password if needed.
    modem.sendAT("+CNCFG=0,1,\"", apn, "\"");
    if (modem.waitResponse() != 1) {
        Serial.println("Set operators apn Failed!");
        return;
    }

    modem.sendAT("+CNACT=0,1");
    if (modem.waitResponse() != 1) {
        Serial.println("Activate network bearer Failed!");
        return;
    }


    modem.sendAT("+CNACT?");
    modem.waitResponse() ;


    /*********************************
    * step 6 : setup MQTT Client and Certificate
    ***********************************/

    setCertificate(ca_cert, client_cert, client_cert_key);



    // If it is already connected, disconnect it first
    modem.sendAT("+SMDISC");
    modem.waitResponse();


    snprintf(buffer, 1024, "+SMCONF=\"URL\",\"%s\",%d", server, port);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"KEEPTIME\",60");
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"CLEANSS\",1");
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"CLIENTID\",\"%s\"", clientID);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"USERNAME\",\"%s\"", username);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"PASSWORD\",\"%s\"", password);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    //todo : fix me . Can I connect normally?

    int8_t ret ;
    do {

        modem.sendAT("+SMCONN");
        ret  = modem.waitResponse(60000UL);
        if (ret != 1) {
            Serial.println("Connect failed, retry connect ..."); delay(1000);
        }

    } while (ret != 1);


    Serial.println("MQTT Client connected!");


    //todo:
}


void loop()
{
    if (!isConnect()) {
        Serial.println("MQTT Client disconnect!"); delay(1000);
        return ;
    }


    //todo:
}

