#if defined ESP32
#include <WiFi.h>
#elif defined ESP8266
#include <ESP8266WiFi.h>
#else
#error "Only support espressif esp32/8266 chip"
#endif

#include "bigiot.h"

#include "DHT.h" // Need DHT-sensor-library support https://github.com/adafruit/DHT-sensor-library


#define WIFI_TIMEOUT 30000

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT21     // DHT 21 (AM2301)Î
#define DHTTYPE DHT11       // DHT 22  (AM2302), AM2321
#define DHT_SENSOR_PIN 22

WiFiClient client;
BIGIOT bigiot(client);
DHT dht(DHT_SENSOR_PIN, DHTTYPE);

const char *ssid = "your wifi ssid";
const char *passwd = "your wifi password";

const char *id = "your device id";              //platform device id
const char *apikey = "your device apikey";      //platform device api key
const char *usrkey = "your user key";           //platform user key , if you are not using encrypted login,you can leave it blank

#define BIGIOT_TEMP_STREAM_ID       "stream data id"      //Temp stream data id
#define BIGIOT_HUMIDITY_STREAM_ID   "stream data id"      //Humidity stream data id
#define STREAM_UPLOAD_TIMEOUT       10000                 //Data update interval


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

    delay(100);

    WiFi.begin(ssid, passwd);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print("Connect ssid fail");
        while (1);
    }

    //Initialize the DHT sensor
    dht.begin();

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


void sensorRead()
{
    static char celsiusTemp[7];
    static char humidityTemp[7];

    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();

    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    } else {
        // Computes temperature values in Celsius + Fahrenheit and Humidity
        float hic = dht.computeHeatIndex(t, h, false);
        dtostrf(hic, 6, 2, celsiusTemp);

        dtostrf(h, 6, 2, humidityTemp);

        /*
            Upload sensor data stream to bigiot platform
        */

        //Multiple data uploads
        const char *id[] = {BIGIOT_TEMP_STREAM_ID, BIGIOT_HUMIDITY_STREAM_ID};
        const char *data[] = {celsiusTemp, humidityTemp};
        bigiot.upload(id, data, 2);

        /*
        // Or you can single data stream upload

        bigiot.upload(BIGIOT_HUMIDITY_STREAM_ID,humidityTemp);

        // Single data stream upload need delay some time
        delay(1000);

        bigiot.upload(BIGIOT_TEMP_STREAM_ID, celsiusTemp);

        */

        //When the temperature rises to 25 degrees Celsius, an alarm will be sent to the mailbox.
        if (h > 25) {
            bool ret = bigiot.sendAlarm("email", "DTH11 humidity is outside the preset range, current humidity" + String(humidityTemp) + "%");
            // Or send to QQ
            // bool ret = bigiot.sendAlarm("qq", "DTH11 humidity is outside the preset range, current humidity" + String(humidityTemp) + "%");
            // Or send to Sina weibo
            // bool ret = bigiot.sendAlarm("weibo", "DTH11 humidity is outside the preset range, current humidity" + String(humidityTemp) + "%");
            if (!ret) {
                Serial.println("The transmission failed, and the maximum sending interval of the platform is 10 minutes.");
            }
        }
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
    //Upload sensor data every 10 seconds
    if (millis() - last_upload_time > STREAM_UPLOAD_TIMEOUT) {
        last_upload_time = millis();
        sensorRead();
    }
}

