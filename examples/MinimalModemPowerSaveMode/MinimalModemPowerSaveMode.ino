/**
 * @file      MinimalModemPowerSaveMode.ino
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

void getPsmTimer();

// Your GPRS credentials, if any
const char apn[] = "CNNBIOT";
// const char apn[] = "ibasis.iot";
const char gprsUser[] = "";
const char gprsPass[] = "";
bool  level = false;



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


    /*********************************
     * step 3 : Check if the SIM card is inserted
    ***********************************/
    String result ;


    if (modem.getSimStatus() != SIM_READY) {
        Serial.println("SIM Card is not insert!!!");
        return ;
    }


    // Assuming that PSM mode is enabled, turn it off first.
    // modem.sendAT("+CPSMS=0,,,\"01011111\",\"00000001\"");
    // if (modem.waitResponse(5000) == 1) {
    //     Serial.println("PSM Mode disable OK!");
    //     if (modem.waitResponse(30000, "+CPSMSTATUS:") == 1) {
    //         result = modem.stream.readStringUntil('\r');
    //         Serial.println();
    //         Serial.print("Relust:");
    //         Serial.println(result);
    //     }
    // }

    /*********************************
     * step 4 : Set the network mode to NB-IOT
    ***********************************/

    modem.setNetworkMode(2);    //use automatic

    modem.setPreferredMode(MODEM_NB_IOT);

    uint8_t pre = modem.getPreferredMode();

    uint8_t mode = modem.getNetworkMode();

    Serial.printf("getNetworkMode:%u getPreferredMode:%u\n", mode, pre);


    /*********************************
    * step 5 : Wait for the network registration to succeed
    ***********************************/
    SIM70xxRegStatus s;
    do {
        s = modem.getRegistrationStatus();
        if (s != REG_OK_HOME && s != REG_OK_ROAMING) {
            Serial.print(".");
            PMU.setChargingLedMode(level ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
            level ^= 1;
            delay(1000);
        }

    } while (s != REG_OK_HOME && s != REG_OK_ROAMING) ;

    Serial.println();
    Serial.print("Network register info:");
    Serial.println(register_info[s]);

    // Activate network bearer, APN can not be configured by default,
    // if the SIM card is locked, please configure the correct APN and user password, use the gprsConnect() method
    modem.sendAT("+CNACT=0,1");
    if (modem.waitResponse() != 1) {
        Serial.println("Activate network bearer Failed!");
        return;
    }

    // if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    //     return ;
    // }

    bool res = modem.isGprsConnected();
    Serial.print("GPRS status:");
    Serial.println(res ? "connected" : "not connected");

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


    //Enable PSM Event report
    modem.sendAT("+CPSMSTATUS=1");
    if (modem.waitResponse() != 1) {
        Serial.println("Enable PSM Event report Failed!"); return;
    }

    // For a UE that wants to apply PSM, enable network registration and location
    modem.sendAT("+CEREG=4");
    if (modem.waitResponse() != 1) {
        Serial.println("PSM register Failed!"); return;
    }

    // Get default T3412, T3324 time
    getPsmTimer();

    /*
    * Change T3412, T3324 time, according to the operator,
    * some operators do not support changing this value,
    * please consult the communication operator
    * T3412, T3324 time Please check the manual or getPsmTimer description
    * */
    // modem.sendAT("+CPSMS=1,,,\"10100011\",\"00100001\"");
    // if (modem.waitResponse(5000) != 1) {
    //     Serial.println("PSM Mode enable failed!");
    //     return ;
    // }




    //AT+CSCLK=1 : Enable sleep mode 1.
#if 0
    // AT+CSCLK=1
    modem.sendAT("+CSCLK=1");
    if (modem.waitResponse() != 1) {
        Serial.println("Enable modem sleep failed!");
    }
    //Pulling up DTR pin, module will go to normal sleep mode
    digitalWrite(BOARD_MODEM_DTR_PIN, HIGH);



    //Pulling down DTR pin will wake module up from sleep mode.
    digitalWrite(BOARD_MODEM_DTR_PIN, LOW);
#endif
}

void loop()
{
    while (Serial1.available()) {
        Serial.write(Serial1.read());
    }
    while (Serial.available()) {
        Serial1.write(Serial.read());
    }
}


void getPsmTimer()
{

    // AT+CPSMS=[<mode>[,<Requested_Periodic-RAU>[,<Requested_GPRS-READY-timer>[,<Requested_Periodic-TAU>[,<Requested_Active-Time>]]]]]
    // <mode>
    //      0 - Disable the use of PSM  1 - Enable the use of PSM
    // <Requested_Periodic-RAU>
    //      Not supported
    // <Requested_GPRS-READY-timer>
    //      Not supported
    // <Requested_Periodic-TAU>
    //      ! T3412
    //      String type; one byte in an 8 bit format. Requested extended periodic TAU value (T3412) to be allocated to the UE in E-UTRAN.
    //      The requested extended periodic TAU value is coded as one byte (octet 3) of the GPRS Timer 3 information element coded as bit format (e.g. "01000111" equals 70 hours).
    //      For the coding and the value range, see the GPRS Timer 3 IE in 3GPP TS 24.008 [8] Table 10.5.163a/3GPP TS 24.008.
    //      See also 3GPP TS 23.682 [149] and 3GPP TS 23.401 [82]. The default value, if available, is manufacturer specific.
    //              GPRS Timer 3 value (octet 3)
    //              Bits 5 to 1 represent the binary coded timer value.
    //              Bits 6 to 8 defines the timer value unit for the GPRS timer as follows:
    //              Bits
    //              8 7 6
    //              0 0 0 value is incremented in multiples of 10 minutes
    //              0 0 1 value is incremented in multiples of 1 hour
    //              0 1 0 value is incremented in multiples of 10 hours
    //              0 1 1 value is incremented in multiples of 2 seconds
    //              1 0 0 value is incremented in multiples of 30 seconds
    //              1 0 1 value is incremented in multiples of 1 minute
    //              1 1 1 value indicates that the timer is deactivated.
    //              Other values shall be interpreted as multiples of 1 hour in this version of the protocol.
    //
    // <Requested_Active-Time>
    //      ! T3324
    //      String type; one byte in an 8 bit format.
    //      Requested Active Time valuen (T3324) to be allocated to the UE.
    //      The requested Active Time value is coded as one byte (octet 3) of the GPRS Timer 2 information element coded as bit format (e.g. "00100100" equals 4 minutes).
    //      For the coding and the value range,
    //      see the GPRS Timer 2 IE in 3GPP TS 24.008 [8] Table 10.5.163/3GPP TS 24.008.
    //      See also 3GPP TS 23.682 [149], 3GPP TS 23.060 [47] and 3GPP TS 23.401 [82]. The default value, if available, is manufacturer specific.
    //              GPRS Timer 3 value (octet 3)
    //              Bits 5 to 1 represent the binary coded timer value.
    //              Bits 6 to 8 defines the timer value unit for the GPRS timer as follows:
    //              Bits
    //              8 7 6
    //              0 0 0 value is incremented in multiples of 10 minutes
    //              0 0 1 value is incremented in multiples of 1 hour
    //              0 1 0 value is incremented in multiples of 10 hours
    //              0 1 1 value is incremented in multiples of 2 seconds
    //              1 0 0 value is incremented in multiples of 30 seconds
    //              1 0 1 value is incremented in multiples of 1 minute
    //              1 1 1 value indicates that the timer is deactivated.
    //              Other values shall be interpreted as multiples of 1 hour in this version of the protocol.
    //

    String result;

    modem.sendAT("+CEREG?");

    if (modem.waitResponse(5000, "+CEREG: ") != 1) {
        return;
    }

    result = modem.stream.readStringUntil('\r');
    for (int i = 0; i < 7; ++i) {
        int index =  result.indexOf(",");
        if (index >= 0) {
            result = result.substring(index + 1);
        }
    }

    result.replace("\"", "");
    String T3412, T3324;
    T3412 = result.substring(0, result.indexOf(",") );
    T3324 = result.substring(result.indexOf(",") + 1);
    Serial.print("T3412:");
    Serial.println(T3412);
    Serial.print("T3324:");
    Serial.println(T3324);



    int T3412_int = 0, T3324_int = 0;
    for (int i = 0; i < 8; ++i) {
        if (T3412[i] == '1') {
            T3412_int |= (1 << (7 - i));
        }
    }
    Serial.print("T3412_int:");
    Serial.println(T3412_int, BIN);

    for (int i = 0; i < 8; ++i) {
        if (T3324[i] == '1') {
            T3324_int |= (1 << (7 - i));
        }
    }
    Serial.print("T3324_int:");
    Serial.println(T3324_int, BIN);

    int T3324_flag = (T3324_int >> 5);
    int T3324_timer = (T3324_int & 0x1F);

    Serial.println("Active-Time:");
    switch (T3324_flag) {
    case 0:
        Serial.println(" 0 0 0 value is incremented in multiples of 10 minutes");
        Serial.printf(" T3324_timer: %u minutes\n", T3324_timer * 10);
        break;
    case 1:
        Serial.println(" 0 0 1 value is incremented in multiples of 1 hour");
        Serial.printf(" T3324_timer: %u hours\n", T3324_timer * 1);
        break;
    case 2:
        Serial.println(" 0 1 0 value is incremented in multiples of 10 hours");
        Serial.printf(" T3324_timer: %u hours\n", T3324_timer * 10);
        break;
    case 3:
        Serial.println(" 0 1 1 value is incremented in multiples of 2 seconds");
        Serial.printf(" T3324_timer: %u seconds\n", T3324_timer * 2);
        break;
    case 4:
        Serial.println(" 1 0 0 value is incremented in multiples of 30 seconds");
        Serial.printf(" T3324_timer: %u seconds\n", T3324_timer * 30);
        break;
    case 5:
        Serial.println(" 1 0 1 value is incremented in multiples of 1 minute");
        Serial.printf(" T3324_timer: %u minutes\n", T3324_timer * 1);
        break;
    default:
        break;
    }

    int T3412_flag = (T3412_int >> 5);
    int T3412_timer = (T3412_int & 0x1F);
    Serial.println("Periodic-TAU:");
    switch (T3412_flag) {
    case 0:
        Serial.println(" 0 0 0 value is incremented in multiples of 10 minutes");
        Serial.printf(" T3412_timer: %u minutes\n", T3412_timer * 10);
        break;
    case 1:
        Serial.println(" 0 0 1 value is incremented in multiples of 1 hour");
        Serial.printf(" T3412_timer: %u hours\n", T3412_timer * 1);
        break;
    case 2:
        Serial.println(" 0 1 0 value is incremented in multiples of 10 hours");
        Serial.printf(" T3412_timer: %u hours\n", T3412_timer * 10);
        break;
    case 3:
        Serial.println(" 0 1 1 value is incremented in multiples of 2 seconds");
        Serial.printf(" T3412_timer: %u seconds\n", T3412_timer * 2);
        break;
    case 4:
        Serial.println(" 1 0 0 value is incremented in multiples of 30 seconds");
        Serial.printf(" T3412_timer: %u seconds\n", T3412_timer * 30);

        break;
    case 5:
        Serial.println(" 1 0 1 value is incremented in multiples of 1 minute");
        Serial.printf(" T3412_timer: %u minutes\n", T3412_timer * 1);
        break;
    default:
        break;
    }

}