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

Code adapted from Blynk library BlynkSerial.h. Copyright info below.
* @file       BlynkSerial.h
* @author     Volodymyr Shymanskyy
* @license    This project is released under the MIT License (MIT)
* @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
* @date       Jan 2015
*/

#ifndef _CAYENNEMQTTSERIALCLIENT_h
#define _CAYENNEMQTTSERIALCLIENT_h

#include "CayenneArduinoMQTTClient.h"

#define SERIAL_CLIENT_BUFFER_SIZE CAYENNE_MAX_MESSAGE_SIZE

template <class T>
class SerialClient : public Client
{
public:
	SerialClient(T& stream) : _stream(stream), _conn(0), _head(0), _tail(0)	{}

	void begin(uint32_t baud) {
		_stream.begin(baud);
	}

	int connect(IPAddress ip, uint16_t port) {
		CAYENNE_LOG("Connecting...");
		return _conn = true;
	}

	int connect(const char *host, uint16_t port) {
		CAYENNE_LOG("Connecting...");
		return _conn = true;
	}

	size_t write(uint8_t) {}

	size_t write(const uint8_t *buf, size_t size) {
		return _stream.write(buf, size);
	}

	int available() {
		return _stream.available() + (((unsigned int)(SERIAL_CLIENT_BUFFER_SIZE + _head - _tail)) % SERIAL_CLIENT_BUFFER_SIZE);
	}

	int read() {
		bool reading = true;
		while (reading)
		{
			while (_stream.available())
			{
				unsigned char c = _stream.read();
				uint8_t i = (unsigned int)(_head + 1) % SERIAL_CLIENT_BUFFER_SIZE;

				// if we should be storing the received character into the location
				// just before the tail (meaning that the head would advance to the
				// current location of the tail), we're about to overflow the buffer
				// and so we don't write the character or advance the head.
				if (i != _tail) {
					_buffer[_head] = c;
					_head = i;
				}
			}
			delay(2);
			if (!_stream.available())
				reading = false;
		}
		// if the head isn't ahead of the tail, we don't have any characters
		if (_head == _tail) {
			return -1;
		}
		else {
			unsigned char c = _buffer[_tail];
			_tail = (uint8_t)(_tail + 1) % SERIAL_CLIENT_BUFFER_SIZE;
			return c;
		}
	}

	int read(uint8_t *buf, size_t size) {
		return readBytes(buf, size);
	}

	int peek() {}

	void flush() {}

	void stop() { _conn = false; }

	uint8_t connected() { return _conn; }

	operator bool() {}

protected:
	T& _stream;
	uint8_t _conn;
	unsigned char _buffer[SERIAL_CLIENT_BUFFER_SIZE];
	uint8_t _head;
	uint8_t _tail;
};

template <class T>
class SerialClientChecked : public SerialClient<T>
{
public:
	SerialClientChecked(T& stream) : SerialClient<T>(stream) {}

	void begin(uint32_t baud) {
		this->_stream.begin(baud);
		while (!this->_stream) {
			// Wait for serial port to connect.
		}
	}

	uint8_t connected() { return this->_conn && this->_stream; }
};

class CayenneMQTTSerialClient : public CayenneArduinoMQTTClient
{
public:
	CayenneMQTTSerialClient() : _serial(Serial) {}

	/**
	* Begins Cayenne session
	* @param username Cayenne username
	* @param password Cayenne password
	* @param clientID Cayennne client ID
	* @param baud Baud rate for connection
	*/
	void begin(const char* username, const char* password, const char* clientID, uint32_t baud = 9600)
	{
		_serial.begin(baud);
		CayenneArduinoMQTTClient::begin(_serial, username, password, clientID);
	}

private:
#if defined(__SAMD21G18A__)
	// For Arduino Zero (M0)
	SerialClientChecked<Uart> _serial;
#elif defined(__SAM3X8E__)
	// For Arduino Due
	SerialClientChecked<UARTClass> _serial;
#elif defined(USBCON)
	// For versions with hw USB, like Micro
	SerialClientChecked<Serial_> _serial;
#else
	// For versions with UART
	SerialClient<HardwareSerial> _serial;
#endif
};

CayenneMQTTSerialClient Cayenne;

#endif