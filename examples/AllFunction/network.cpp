/**
 * @file      network.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

String hostName = "LilyGo-Cam-";
String ipAddress = "";
bool isAP = false;
bool networkReady = false;

void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi clients stopped");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("Disconnected from WiFi access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("Obtained IP address: ");
        Serial.println(WiFi.localIP());
        networkReady = true;
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
        Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
        Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
        Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_PIN:
        Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case ARDUINO_EVENT_WIFI_AP_START:
        Serial.println("WiFi access point started");
        networkReady = true;
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        Serial.println("WiFi access point  stopped");
        networkReady = false;
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        Serial.println("Client connected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        Serial.println("Client disconnected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        Serial.println("Assigned IP address to client");
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        Serial.println("Received probe request");
        break;
    default:
        break;
    }
}

void setupNetwork(bool setup_AP_Mode)
{
    isAP = setup_AP_Mode;

    WiFi.onEvent(WiFiEvent);

    if (setup_AP_Mode)
    {
        WiFi.mode(WIFI_AP);
        hostName += WiFi.macAddress().substring(0, 5);
        WiFi.softAP(hostName.c_str());
        ipAddress = WiFi.softAPIP().toString();
        Serial.print("Started AP mode ,ip address is :");
        Serial.println(WiFi.softAPIP().toString());
    }
    else
    {
        wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
        // wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
        // wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");
    }
}

void loopNetwork()
{
    if (!isAP)
    {
        wifiMulti.run();
    }
}

String getIpAddress()
{
    if (!networkReady)
    {
        return "";
    }
    if (isAP)
    {
        return WiFi.softAPIP().toString();
    }
    else
    {
        return WiFi.localIP().toString();
    }
}