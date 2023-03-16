/*
The MIT License(MIT)

Cayenne Arduino Client Library
Copyright © 2016 myDevices

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files(the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Code adapted from Blynk library BlynkSimpleShieldEsp8266.h. Copyright info below.
* @file       BlynkSerial.h
* @author     Volodymyr Shymanskyy
* @license    This project is released under the MIT License (MIT)
* @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
* @date       Jan 2015
*/

#ifndef _CAYENNEMQTTESP8266SHIELD_h
#define _CAYENNEMQTTESP8266SHIELD_h

#define INFO_CONNECTION "ESP8266Shield"

#include "Client.h"
#include "ESP8266SerialLibrary.h"
#include "CayenneUtils/CayenneDefines.h"
#include "CayenneArduinoMQTTClient.h"

#ifdef ESP8266
#error This code is not intended to run on the ESP8266 platform! Please check your Tools->Board setting.
#endif

#ifndef CAYENNE_ESP8266_MUX
#define CAYENNE_ESP8266_MUX  1
#endif

#ifndef ESP8266_CLIENT_BUFFER_SIZE
#define ESP8266_CLIENT_BUFFER_SIZE CAYENNE_MAX_MESSAGE_SIZE
#endif

#define WRITE_CHUNK_SIZE 40 // The chunk size to use when sending data to prevent sending too much data at once.

class ESP8266Client : public Client
{
public:
	ESP8266Client() : _esp8266(NULL), _connected(false), _domain(NULL), _port(0), _head(0), _tail(0) {}

	void initialize(ESP8266* esp8266) {
		_esp8266 = esp8266;
		_esp8266->setOnData(onData, this);
	}

	bool connectWiFi(const char* ssid, const char* pass)
	{
		::delay(500);
		if (!_esp8266->kick()) {
			CAYENNE_LOG("ESP is not responding");
			return false;
		}
		if (!_esp8266->setEcho(0)) {
			CAYENNE_LOG("Failed to disable Echo");
			return false;
		}
		if (!_esp8266->enableMUX()) {
			CAYENNE_LOG("Failed to enable MUX");
		}
		if (!_esp8266->setOprToStation()) {
			CAYENNE_LOG("Failed to set STA mode");
			return false;
		}
		if (!_esp8266->joinAP(ssid, pass)) {
			CAYENNE_LOG("Failed to connect WiFi: %s", ssid);
			return false;
		}
		CAYENNE_LOG("Connected to WiFi");
		return true;
	}

	int connect(IPAddress ip, uint16_t port) {
		// Not currently implemented
		_connected = false;
		return _connected;
	}

	int connect(const char *host, uint16_t port) {
		_connected = _esp8266->createTCP(CAYENNE_ESP8266_MUX, host, port);
		return _connected;
	}

	size_t write(uint8_t) {}

	size_t write(const uint8_t *buf, size_t size) {
		if (_esp8266->send(CAYENNE_ESP8266_MUX, buf, size)) {
			return size;
		}
		return 0;
	}

	int available() {
		_esp8266->run();
		return ((unsigned int)(ESP8266_CLIENT_BUFFER_SIZE + _head - _tail)) % ESP8266_CLIENT_BUFFER_SIZE;
	}

	int read() {
		// if the head isn't ahead of the tail, we don't have any characters
		if (_head == _tail) {
			return -1;
		}
		else {
			unsigned char c = _buffer[_tail];
			_tail = (uint8_t)(_tail + 1) % ESP8266_CLIENT_BUFFER_SIZE;
			return c;
		}
	}

	int read(uint8_t *buf, size_t size) {
		return readBytes(buf, size);
	}

	int peek() {}

	void flush() {}

	void stop() {
		_connected = false;
		_head = _tail = 0;
		_esp8266->releaseTCP(CAYENNE_ESP8266_MUX);
	}

	uint8_t connected() { return _connected; }

	operator bool() {}

private:
	static void onData(uint8_t mux_id, uint32_t len, void* ptr) {
		((ESP8266Client*)ptr)->onData(mux_id, len);
	}

	void onData(uint8_t mux_id, uint32_t len) {
		if (mux_id != CAYENNE_ESP8266_MUX) {
			return;
		}
		Stream* puart = _esp8266->getUart();
		unsigned long start = millis();
		while (len) {
			if (millis() - start > 5000) {
				len = 0;
				_head = _tail = 0;
				CAYENNE_LOG_DEBUG("Timeout reading data");
			}
			if (puart->available()) {
				uint8_t c = puart->read();
				uint8_t i = (unsigned int)(_head + 1) % ESP8266_CLIENT_BUFFER_SIZE;

				if (i != _tail) {
					_buffer[_head] = c;
					_head = i;
				}
				else {
					CAYENNE_LOG_DEBUG("Buffer overflow");
				}
				len--;
			}
		}
	}

	ESP8266* _esp8266;
	bool _connected;
	const char* _domain;
	uint16_t _port;
	unsigned char _buffer[ESP8266_CLIENT_BUFFER_SIZE];
	uint8_t _head;
	uint8_t _tail;
};

class CayenneMQTTESP8266Client : public CayenneArduinoMQTTClient
{
public:
	/**
	* Begins Cayenne session
	* @param username Cayenne username
	* @param password Cayenne password
	* @param clientID Cayennne client ID
	* @param esp8266 ESP8266 to use for wifi connection
	* @param ssid WiFi network id
	* @param pass WiFi network password
	*/
	void begin(const char* username, const char* password, const char* clientID, ESP8266& esp8266, const char* ssid, const char* pass)
	{
		_client.initialize(&esp8266);
		while (!_client.connectWiFi(ssid, pass)) {
			delay(1000);
		}
		CayenneArduinoMQTTClient::begin(_client, username, password, clientID, WRITE_CHUNK_SIZE);
	}

	/**
	* Main Cayenne loop
	*
	* @param yieldTime  Time in milliseconds to yield to allow processing of incoming MQTT messages and keep alive packets.
	* NOTE: Decreasing the yieldTime while calling write functions (e.g. virtualWrite) in your main loop could cause a
	* large number of messages to be sent to the Cayenne server. Use caution when adjusting this because sending too many
	* messages could cause your IP to be rate limited or even blocked. If you would like to reduce the yieldTime to cause your
	* main loop to run faster, make sure you use a timer for your write functions to prevent them from running too often.
	*/
	void loop(int yieldTime = 1000) {
		unsigned long start = millis();
		while (millis() - start < yieldTime) {
			// Loop repeatedly to ensure ESP serial buffer is read quickly enough to prevent buffer overflows.
			CayenneArduinoMQTTClient::loop(0);
		}
	}

private:
	ESP8266Client _client;
};

CayenneMQTTESP8266Client Cayenne;

#endif