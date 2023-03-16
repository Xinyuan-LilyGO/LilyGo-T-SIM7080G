/*
The MIT License(MIT)

Cayenne Arduino Client Library
Copyright (c) 2018 myDevices

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files(the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _CAYENNEMQTTGSMCLIENT_h
#define _CAYENNEMQTTGSMCLIENT_h

#include "CayenneArduinoMQTTClient.h"


class CayenneMQTTGSMClient : public CayenneArduinoMQTTClient
{
public:
	/**
	* Begins Cayenne session and in the process establishes a GSM connection with the supplied ssid and WIFI password
	* @param username Cayenne username
	* @param password Cayenne password
	* @param clientID Cayennne client ID
	* @param serial Serial connection to GSM device
	* @param apn Access Point Name
	* @param gprsLogin GPRS login
	* @param gprsPassword GPRS password
	* @param pin SIM pin number
	*/
	void begin(const char* username, const char* password, const char* clientID, Stream& serial, const char* apn, const char* gprsLogin, const char* gprsPassword, const char* pin = "")
	{
		_modem = new TinyGsm(serial);
		_gsmClient.init(_modem);
 		CAYENNE_LOG("Initializing modem");
		if (!_modem->begin()) {
			CAYENNE_LOG("Cannot init");
		}
		if (pin && pin[0] != 0) {
			CAYENNE_LOG("Unlocking SIM");
			_modem->simUnlock(pin);
		}
		CAYENNE_LOG("Waiting for network");
		while (!_modem->waitForNetwork()) {
			CAYENNE_LOG("Register failed");
			delay(500);
		}
		CAYENNE_LOG("Connecting to GPRS");
		while (!_modem->gprsConnect(apn, gprsLogin, gprsPassword)) {
			CAYENNE_LOG("Not connected");
			delay(500);
		}
		CAYENNE_LOG("Connected to GPRS");

		// int csq = _modem->getSignalQuality();
  		// CAYENNE_LOG("Signal quality: %d", csq);
		IPAddress local_ip = _modem->localIP();
		CAYENNE_LOG("IP: %d.%d.%d.%d", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
		CayenneArduinoMQTTClient::begin(_gsmClient, username, password, clientID);
	}

	/**
	* Begins Cayenne session, assumes that the GSM is already up and running. 
	* @param username Cayenne username
	* @param password Cayenne password
	* @param clientID Cayennne client ID
	*/
	void begin(const char* username, const char* password, const char* clientID)
	{
		IPAddress local_ip = _modem->localIP();
		CAYENNE_LOG("IP: %d.%d.%d.%d", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
		CayenneArduinoMQTTClient::begin(_gsmClient, username, password, clientID);
	}


private:
	TinyGsmClient _gsmClient;
	TinyGsm* _modem;
};

CayenneMQTTGSMClient Cayenne;

#endif
