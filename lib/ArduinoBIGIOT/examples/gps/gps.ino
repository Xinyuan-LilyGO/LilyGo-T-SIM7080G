#if defined ESP32
#include <WiFi.h>
#elif defined ESP8266
#include <ESP8266WiFi.h>
#else
#error "Only support espressif esp32/8266 chip"
#endif

#include "bigiot.h"
#include <TinyGPS++.h>  //Need TinyGPS librarie support https://github.com/mikalhart/TinyGPSPlus

#define WIFI_TIMEOUT 30000

TinyGPSPlus gps;
#define GPS_BANUD_RATE 9600
#define GPS_RX_PIN 12
#define GPS_TX_PIN 15

const char *ssid = "your wifi ssid";
const char *passwd = "your wifi password";

const char *id = "your device id";              //platform device id
const char *apikey = "your device apikey";      //platform device api key
const char *usrkey = "your user key";           //platform user key , if you are not using encrypted login,you can leave it blank

#define BIGIOT_LOACTION_STREAM_ID   "stream data id"     //Positioning data interface id
#define STREAM_UPLOAD_TIMEOUT       10000               //Data update interval

WiFiClient client;
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

void setup()
{
    Serial.begin(115200);

    //Open Serial1 to read gps chip
    Serial1.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    delay(100);

    WiFi.begin(ssid, passwd);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print("Connect ssid fail");
        while (1);
    }

    //Regist platform command event hander
    bigiot.eventAttach(eventCallback);

    //Regist device disconnect hander
    bigiot.disconnectAttack(disconnectCallback);

    //Regist device connect hander
    bigiot.connectAttack(connectCallback);

    // Login to bigiot.net
    if (!bigiot.login(id, apikey, usrkey)) {
        Serial.println("Login fail");
        while (1);
    }
}


void loop()
{
    static uint64_t last_upload_time = 0;
    static uint64_t last_wifi_check_time = 0;

    if (WiFi.status() == WL_CONNECTED) {
        //Wait for platform command release
        bigiot.handle();
    } else {
        uint64_t now = millis();
        // Wifi disconnection reconnection mechanism
        if (now - last_wifi_check_time > WIFI_TIMEOUT) {
            Serial.println("WiFi connection lost. Reconnecting...");
            WiFi.reconnect();
            last_wifi_check_time = now;
        }
    }

    //Upload targeting data every 10 seconds
    if (gps.location.isValid() && millis() - last_upload_time > STREAM_UPLOAD_TIMEOUT) {
        last_upload_time = millis();
        bigiot.loaction(BIGIOT_LOACTION_STREAM_ID, gps.location.lng(), gps.location.lat());
    }

    while (Serial1.available())
        gps.encode(Serial1.read());

}

