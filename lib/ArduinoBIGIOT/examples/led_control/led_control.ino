#if defined ESP32
#include <WiFi.h>
#elif defined ESP8266
#include <ESP8266WiFi.h>
#else
#error "Only support espressif esp32/8266 chip"
#endif
#include "bigiot.h"

#define WIFI_TIMEOUT 30000

const char *ssid = "your wifi ssid";
const char *passwd = "your wifi password";

const char *id = "your device id";              //platform device id
const char *apikey = "your device apikey";      //platform device api key
const char *usrkey = "your user key";           //platform user key , if you are not using encrypted login,you can leave it blank

#define LED_PIN 16                              // led pin

WiFiClient client;
BIGIOT bigiot(client);

//中文 开灯 Uincode码
#define TURN_ON     "u5f00u706f"
//中文 关灯 Uincode码
#define TURN_OFF    "u5173u706f"

void eventCallback(const int devid, const int comid, const char *comstr, const char *slave)
{
    // You can handle the commands issued by the platform here.
    Serial.printf("Received[%d] - [%d]:%s \n", devid, comid, comstr);
    switch (comid) {
    case STOP:
        digitalWrite(LED_PIN, HIGH);
        break;
    case PLAY:
        digitalWrite(LED_PIN, LOW);
        break;
    case OFFON:
        break;
    case MINUS:
        break;
    case UP:
        break;
    case PLUS:
        break;
    case LEFT:
        break;
    case PAUSE:
        break;
    case RIGHT:
        break;
    case BACKWARD:
        break;
    case DOWN:
        break;
    case FPRWARD:
        break;
    case CUSTOM:
        // 当收到中文开灯时候将led输出低电平
        if (!strcmp(comstr, TURN_ON)) {
            digitalWrite(LED_PIN, LOW);
        // 当收到中文关灯时候将led输出高电平
        } else if (!strcmp(comstr, TURN_OFF)) {
            digitalWrite(LED_PIN, HIGH);
        }
        break;
    default:
        break;
    }
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

    delay(100);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

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
}