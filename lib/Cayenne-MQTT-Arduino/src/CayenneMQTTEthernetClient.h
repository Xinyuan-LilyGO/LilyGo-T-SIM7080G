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
*/

#ifndef _CAYENNEMQTTETHERNETCLIENT_h
#define _CAYENNEMQTTETHERNETCLIENT_h

#include "CayenneArduinoMQTTClient.h"

static const uint32_t HASH_PRIME = 0x01000193; //   16777619 - Default prime value recommended for fnv hash
static const uint32_t HASH_SEED = 0x811C9DC5;  // 2166136261 - Default seed value recommended for fnv hash

class CayenneMQTTEthernetClient : public CayenneArduinoMQTTClient
{
public:
	/**
	* Begins Cayenne session
	* @param username Cayenne username
	* @param password Cayenne password
	* @param clientID Cayennne client ID
	* @param mac Mac address for device
	*/
	void begin(const char* username, const char* password, const char* clientID, const byte mac[] = NULL)
	{
		byte* macAddress = (byte*)GetMACAddress(clientID, mac);
		while (!Ethernet.begin(macAddress)) {
			CAYENNE_LOG("DHCP failed, retrying");
			delay(100);
		}
		IPAddress local_ip = Ethernet.localIP();
		CAYENNE_LOG("IP: %d.%d.%d.%d", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
		CayenneArduinoMQTTClient::begin(_ethernetClient, username, password, clientID);
	}

	/**
	* Begins Cayenne session
	* @param username Cayenne username
	* @param password Cayenne password
	* @param clientID Cayennne client ID
	* @param local Static IP address of device
	* @param mac Mac address for device
	*/
	void begin(const char* username, const char* password, const char* clientID, IPAddress local, const byte mac[] = NULL)
	{
		Ethernet.begin((byte*)GetMACAddress(clientID, mac), local);
		IPAddress local_ip = Ethernet.localIP();
		CAYENNE_LOG("IP: %d.%d.%d.%d", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
		CayenneArduinoMQTTClient::begin(_ethernetClient, username, password, clientID);
	}

	/**
	* Begins Cayenne session
	* @param username Cayenne username
	* @param password Cayenne password
	* @param clientID Cayennne client ID
	* @param local Static IP address of device
	* @param dns IP address of DNS server
	* @param gateway IP address of gateway
	* @param subnet Subnet mask
	* @param mac Mac address for device
	*/
	void begin(const char* username, const char* password, const char* clientID, IPAddress local, IPAddress dns, IPAddress gateway, IPAddress subnet, const byte mac[] = NULL)
	{
		Ethernet.begin((byte*)GetMACAddress(clientID, mac), local, dns, gateway, subnet);
		IPAddress local_ip = Ethernet.localIP();
		CAYENNE_LOG("IP: %d.%d.%d.%d", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
		CayenneArduinoMQTTClient::begin(_ethernetClient, username, password, clientID);
	}

private:
	/**
	* Get MAC address fror the device
	* @param token Authentication token from Cayenne site
	* @param mac User defined mac address for device
	* @returns Mac address for device
	*/
	const byte* GetMACAddress(const char* token, const byte mac[])
	{
		if (mac != NULL)
			return mac;

		_mac[0] = 0xFE;
		//Generate MAC from token to prevent collisions on the same network.
		_mac[1] = token[0] ? token[0] : 0xED;
		*((uint32_t*)&_mac[2]) = fnv1a(token);
		CAYENNE_LOG("MAC: %X-%X-%X-%X-%X-%X", _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]);
		return _mac;
	}

	/**
	* Create hash value using FNV-1a algorithm.
	* @param text Null terminated string to generate hash from
	* @returns hash Seed value for hash
	*/
	uint32_t fnv1a(const char* text, uint32_t hash = HASH_SEED)
	{
		while (*text)
			hash = (*text++ ^ hash) * HASH_PRIME;
		return hash;
	}

	EthernetClient _ethernetClient;
	byte _mac[6];
};

CayenneMQTTEthernetClient Cayenne;

#endif