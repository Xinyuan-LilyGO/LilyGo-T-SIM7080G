#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"
#include <WiFi.h>

// #define USING_WIFI           //Connect to www.bigiot.net using wifi
XPowersPMU  PMU;

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>
#include "utilities.h"

#include "bigiot.h"


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(Serial1);
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

#ifdef USING_WIFI
const char *ssid = "Your wifi ssid";
const char *passwd = "Your wifi password";
#endif


const char *id = "YOUR DEVICE ID";                //platform device id
const char *apikey = "YOUR APIKEY";        //platform device api key
const char *usrkey = "";                 //platform user key , if you are not using encrypted login,you can leave it blank

#define BIGIOT_LOACTION_STREAM_ID   "Positioning data interface id"             //Positioning data interface id
#define STREAM_UPLOAD_TIMEOUT       10000               //Data update interval

// Set to wake up every 5 minutes to send positioning coordinates to the platform
#define uS_TO_S_FACTOR 1000000ULL                       /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5*60                             /* Time ESP32 will go to sleep (in seconds) */

#ifdef USING_WIFI
WiFiClient client;
#else
TinyGsmClient client(modem, 0);
#endif
BIGIOT bigiot(client);


void eventCallback(const int devid, const int comid, const char *comstr, const char *slave)
{
    // You can handle the commands issued by the platform here.
    Serial.printf(" device id:%d ,command id:%d command string:%s\n", devid, comid, comstr);
}

void disconnectCallback(BIGIOT &obj)
{
    // When the device is disconnected to the platform, you can handle your peripherals here
    Serial.print(obj.deviceName());
    Serial.println("  disconnect");

}

void connectCallback(BIGIOT &obj)
{
    // When the device is connected to the platform, you can preprocess your peripherals here
    Serial.print(obj.deviceName());
    Serial.println("  connect");
}


void deepsleep()
{
    // Turn off modem
    modem.poweroff();

    // Wait until the modem does not respond to the command, and then proceed to the next step
    while (modem.testAT()) {
        delay(500);
    }

    //Turn off modem power
    PMU.disableDC3();

    //Turn off gps power
    PMU.disableBLDO2();      //The antenna power must be turned on to use the GPS function

    //Turn off the power supply for level conversion
    PMU.disableBLDO1();

    //Turn off chg led
    PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);

    Wire.end();

    Serial1.end();

    Serial1.end();

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
    //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

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

void setup()
{
    WiFi.mode(WIFI_OFF);

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

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
    }

    // ESP32S3 power supply cannot be turned off
    // PMU.disableDC1();

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

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();


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
            Serial.println("Retry start modem .");
        }
    }
    Serial.println();
    Serial.print("Modem started!");



    /*********************************
     * step 3 : Check if the SIM card is inserted
    ***********************************/

    if (modem.getSimStatus() != SIM_READY) {
        Serial.println("SIM Card is not insert!!!");
        return ;
    }


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

    // Disconnect the network. GPS and network cannot be enabled at the same time
    modem.gprsDisconnect();

    // GPS function needs to be enabled for the first use
    if (modem.enableGPS() == false) {
        Serial.print("Modem enable gps function failed!!");
    }


    /*********************************
    * step 6 : Obtain location information
    ***********************************/
    PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);
    while (1) {
        //Upload targeting data every 10 seconds
        if (modem.getGPS(&lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2,
                         &year2, &month2, &day2, &hour2, &min2, &sec2)) {
            Serial.println();
            Serial.print("lat:"); Serial.print(String(lat2, 8)); Serial.print("\t");
            Serial.print("lon:"); Serial.print(String(lon2, 8)); Serial.println();
            Serial.print("speed:"); Serial.print(speed2); Serial.print("\t");
            Serial.print("altitude:"); Serial.print(alt2); Serial.println();
            Serial.print("year:"); Serial.print(year2);
            Serial.print(" montoh:"); Serial.print(month2);
            Serial.print(" day:"); Serial.print(day2);
            Serial.print(" hour:"); Serial.print(hour2);
            Serial.print(" minutes:"); Serial.print(min2);
            Serial.print(" second:"); Serial.print(sec2); Serial.println();
            Serial.println();

            break;
        }
        delay(1000);
    }

    // When you need to access the network after positioning, you need to disable GPS
    modem.disableGPS();

    // After successful positioning, the PMU charging indicator flashes quickly
    PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_4HZ);


    /*********************************
    * step 7 : Connect to the network
    ***********************************/
    // Activate network bearer, APN can not be configured by default,
    // if the SIM card is locked, please configure the correct APN and user password, use the gprsConnect() method
    modem.sendAT("+CNACT=0,1");
    if (modem.waitResponse() != 1) {
        Serial.println("Activate network bearer Failed!");
    }


    int res = modem.isGprsConnected();
    Serial.print("GPRS status:");
    Serial.println(res ? "connected" : "not connected");

#ifdef USING_WIFI
    WiFi.begin(ssid, passwd);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print("Connect ssid fail");
        while (1);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
#endif


    /*********************************
    * step 8 : Connect to the BIGIOT
    ***********************************/
    //Regist platform command event hander
    bigiot.eventAttach(eventCallback);

    //Regist device disconnect hander
    bigiot.disconnectAttack(disconnectCallback);

    //Regist device connect hander
    bigiot.connectAttack(connectCallback);

    // Login to bigiot.net
    if (!bigiot.login(id, apikey, usrkey)) {
        Serial.println("Login fail");
        // Enter deep sleep after connection failure and resend in the next cycle
        deepsleep();
    }

}

void loop()
{
    bigiot.handle();
    if (client.connected() ) {
        // Upload the saved longitude and latitude to the bigiot platform
        bigiot.loaction(BIGIOT_LOACTION_STREAM_ID, lon2, lat2);
    }
    // Enter deep sleep and resend in the next cycle
    deepsleep();
}

