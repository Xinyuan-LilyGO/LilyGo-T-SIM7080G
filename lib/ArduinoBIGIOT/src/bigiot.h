
/////////////////////////////////////////////////////////////////
/*
  bigiot.cpp - Arduino library simplifies the use of connected BIGIOT platforms.
  Created by Lewis he on January 1, 2019.
*/
/////////////////////////////////////////////////////////////////
#pragma once

#include "Client.h"
#include "WiFiClient.h"
#define DEBUG_BIGIOT_PORT Serial
#ifdef DEBUG_BIGIOT_PORT
#define DEBUG_BIGIOTCIENT(fmt, ...)               DEBUG_BIGIOT_PORT.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ )
#define DEBUG_BIGIOT_WRITE(x)                     DEBUG_BIGIOT_PORT.write(x)
#else
#define DEBUG_BIGIOTCIENT(...)
#define DEBUG_BIGIOT_WRITE(x)
#endif

#define BIGIOT_PLATFORM_HOST                      "www.bigiot.net"
#define BIGIOT_PLATFORM_PORT                      8282
#define BIGIOT_PLATFORM_HTTPS_PORT                443
#define BIGIOT_LOGINT_WELCOME                     1
#define BIGIOT_LOGINT_CHECK_IN                    2
#define BIGIOT_LOGINT_TOKEN                       3
#define BIGIOT_PLATFROM_COMMAND_TOTAL             12
#define REC_TIMEOUT                               10000
#define BIGIOT_PLATFORM_ALARM_INTERVAL            600000
#define BIGIOT_PLATFROM_COMMAND_TABLE             {"play", "stop", "offOn", "minus", "up", "plus", "left", "pause", "right", "backward", "down", "forward"} ;
#define BIGIOT_PLATFORM_8282_HATERATE_PACK        ("{\"M\":\"b\"}\n")
#define BIGIOT_PLATFORM_8383_HATERATE_PACK        ("{\"M\":\"ping\"}")
#define BIGIOT_PLATFORM_WEBSOCKET_HATERATE_PACK   ("{\"M\":\"beat\"}\n")

#define SERVERCHAN_LINK_FORMAT                    "http://sc.ftqq.com/%s.send?text=%s"
#define SERVERCHAN_DESP_MAX_LENGTH                65536

#define XEMAIL_RECV_TIMEOUT                       10000

#define PLATFORM_ARRAY_SIZE(x)                    (sizeof(x)/sizeof(x[0]))


enum {
    INVALD = -2,
    DISCONNECT = -1,
    PLAY = 0,
    STOP,
    OFFON,
    MINUS,
    UP,
    PLUS,
    LEFT,
    PAUSE,
    RIGHT,
    BACKWARD,
    DOWN,
    FPRWARD,
    CUSTOM,
};


class xEamil : public WiFiClient
{
public:
    // xEamil(Client &client);
    void setEmailHost(const char *host, uint16_t port);
    bool setSender(const char *user, const char *password);
    void setRecipient(const char *email);
    bool sendEmail(String &subject, String &content);
    bool sendEmail(const char *subject, const char *content);
private:
    bool emailRecv();
    void emailFail();

protected:
    Client *_client;
    uint16_t _emailPort;
    String _emailHost, _emailUser, _emailPasswd, _recipient, _base64User, _base64Pass;
};


class ServerChan
{
public:
    ServerChan(String sckey): _sckey(sckey)
    {
    }
    void setSCKEY(const char *key);
    void setSCKEY(String &key);
    bool sendWechat(const char *text, const char *desp = NULL);
    bool sendWechat(String text, String desp);
private:
    String _sckey;
};

class BIGIOT
{
public:
    BIGIOT(Client &client);
    typedef void (*eventCallbackFunc)(const int id, const int c, const char *command, const char *salve);
    typedef void (*generlCallbackFunc)(BIGIOT &);

    void connectAttack(generlCallbackFunc f),
         disconnectAttack(generlCallbackFunc f),
         eventAttach(eventCallbackFunc f);

    int handle(void);

    bool login(const char *devId, const char *apiKey, const char *userKey = "", bool reconnect = true),

         upload(const char *id, const char *data),
         upload(const char *id[], const char *data[], int len),
         upload(String id, String data),

         loaction(const char *id, const char *longitude, const char *latitude),
         loaction(const char *id, float longitude, float latitude),
         loaction(String id, String longitude, String latitude),

         uploadPhoto( const char *id, const char *type, const char *filename,  uint8_t *image, size_t size),

         sendAlarm(const char *method, const char *message),
         sendAlarm(String method, String message);

    bool operator == (BIGIOT &b);
    bool isOnline();
    const char *deviceName();
    String deviceName() const;
    void setHeartFreq(uint32_t f);
    bool checkOnline();

private:
    bool loginToBigiot();

    int loginParse(String pack),
        packetParse(String pack);

    String getLoginPacket(String apiKey),
           getLogoutPacket();

    const char *platform_command[BIGIOT_PLATFROM_COMMAND_TOTAL] = BIGIOT_PLATFROM_COMMAND_TABLE;

protected:
    uint16_t _port;
    uint32_t _heartFreq = 30000;
    String _host,
           _dev,
           _key,
           _usrKey,
           _token,
           _devName;
    Client *_client;
    bool _reconnect, _isLogin, _isCall;
    eventCallbackFunc _eventCallback = NULL;
    generlCallbackFunc _disconnectCallback = NULL;
    generlCallbackFunc _connectCallback = NULL;
};


