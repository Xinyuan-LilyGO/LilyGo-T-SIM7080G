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


#define EMAIL_SMPT_SERVER_HOST  "smtp.163.com"      //Only supports smtp mailbox server
#define EMAIL_SMPT_SERVER_PORT  25                  //Server port
#define EMAIL_SENDER_ACCOUNT    "xxx@163.com"       //Sender account
#define EMAIL_SENDER_PASSWORD   "password"          //Sender password
#define EMAIL_RECIPIENT         "xxx@xx.com"        //Recipient email


#define SERVER_WECHAT_KEY       "xxxxxx"            //Third-party WeChat interface apikey，See http://sc.ftqq.com/3.version

#define BIGIOT_TEMP_STREAM_ID       "stream data id"    //Stream data id
#define STREAM_UPLOAD_TIMEOUT       2000                //Data update interval
#define DS18B20_PIN                 15                  //18b20 data pin

// Simple ds18b20 class
class DS18B20
{
public:
    DS18B20(int gpio)
    {
        pin = gpio;
    }

    float temp()
    {
        uint8_t arr[2] = {0};
        if (reset()) {
            wByte(0xCC);
            wByte(0x44);
            delay(750);
            reset();
            wByte(0xCC);
            wByte(0xBE);
            arr[0] = rByte();
            arr[1] = rByte();
            reset();
            return (float)(arr[0] + (arr[1] * 256)) / 16;
        }
        return 0;
    }
private:
    int pin;

    void write(uint8_t bit)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delayMicroseconds(5);
        if (bit)digitalWrite(pin, HIGH);
        delayMicroseconds(80);
        digitalWrite(pin, HIGH);
    }

    uint8_t read()
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delayMicroseconds(2);
        digitalWrite(pin, HIGH);
        delayMicroseconds(15);
        pinMode(pin, INPUT);
        return digitalRead(pin);
    }

    void wByte(uint8_t bytes)
    {
        for (int i = 0; i < 8; ++i) {
            write((bytes >> i) & 1);
        }
        delayMicroseconds(100);
    }

    uint8_t rByte()
    {
        uint8_t r = 0;
        for (int i = 0; i < 8; ++i) {
            if (read()) r |= 1 << i;
            delayMicroseconds(15);
        }
        return r;
    }

    bool reset()
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delayMicroseconds(500);
        digitalWrite(pin, HIGH);
        pinMode(pin, INPUT);
        delayMicroseconds(500);
        return digitalRead(pin);
    }
};


WiFiClient client;
BIGIOT bigiot(client);
DS18B20 temp18B20(DS18B20_PIN);

void eventCallback(const int devid, const int comid, const char *comstr, const char *slave)
{
    // You can handle the commands issued by the platform here.
    Serial.printf(" device id:%d ,command id:%d command string:%s\n", devid, comid, comstr);
}

void disconnectCallback(BIGIOT &obj)
{
    // When the device is disconnected to the platform, you can handle your peripherals here
    bool ret;
    String content = "[" + String(millis()) + "]" + obj.deviceName() + " 设备断开连接,请检查您的设备...";
    String title = "来自 " + String(obj.deviceName()) + " 发来的消息";

    //When the device is disconnected, you can choose to send the message to the specified mailbox.
    xEamil mail;
    mail.setEmailHost(EMAIL_SMPT_SERVER_HOST, EMAIL_SMPT_SERVER_PORT);
    mail.setSender(EMAIL_SENDER_ACCOUNT, EMAIL_SENDER_PASSWORD);
    mail.setRecipient(EMAIL_RECIPIENT);
    if (!mail.sendEmail(title, content)) {
        Serial.println("Send fail");
    }

    // Or you can send wechat message
    ServerChan cat(SERVER_WECHAT_KEY);

    if (! cat.sendWechat(title, content)) {
        Serial.println("Send fail");
    }
}

void connectCallback(BIGIOT &obj)
{
    // When the device is connected to the platform, you can preprocess your peripherals here
    Serial.print(obj.deviceName());
    Serial.println("  connect");
}


void tempRead()
{
    //Single data stream upload
    float temp = temp18B20.temp();
    bigiot.upload(BIGIOT_TEMP_STREAM_ID, String(temp));

    //When the temperature rises to 25 degrees Celsius, an alarm will be sent to the mailbox.
    if (temp > 25) {
        bool ret = bigiot.sendAlarm("email", "DS18B20 temperature is outside the preset range, current temperature" + String(temp) + "*C");
        // Or send to QQ
        // bool ret = bigiot.sendAlarm("qq", "DS18B20 temperature is outside the preset range, current temperature" + String(temp) + "*C");
        // Or send to Sina weibo
        // bool ret = bigiot.sendAlarm("weibo", "DS18B20 temperature is outside the preset range, current temperature" + String(temp) + "*C");
        if (!ret) {
            Serial.println("The transmission failed, and the maximum sending interval of the platform is 10 minutes.");
        }
    }
}

void setup()
{
    Serial.begin(115200);

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

    if (millis() - last_upload_time > STREAM_UPLOAD_TIMEOUT) {
        last_upload_time = millis();
        tempRead();
    }
}