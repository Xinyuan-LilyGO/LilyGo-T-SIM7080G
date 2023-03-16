/*
The MIT License(MIT)

Cayenne MQTT Client Library
Copyright (c) 2016 myDevices

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

#include "CayenneMQTTClient.h"
#include <string.h>

void MQTTMessageArrived(MessageData* md, void* userData)
{
	CayenneMQTTClient* client = (CayenneMQTTClient*)userData;
	if (client)
	{
		int i;
		int result = MQTT_FAILURE;
		CayenneMessageData message;

		result = CayenneParseTopic(&message.topic, &message.channel, &message.clientID, client->username, md->topicName->lenstring.data, md->topicName->lenstring.len);
		if (result != CAYENNE_SUCCESS)
			return;
		//Null terminate the string since that is required by CayenneParsePayload. The readbuf is set to CAYENNE_MAX_MESSAGE_SIZE+1 to allow for appending a null.
		((char*)md->message->payload)[md->message->payloadlen] = '\0';
		message.valueCount = CAYENNE_MAX_MESSAGE_VALUES;
		result = CayenneParsePayload(message.values, &message.valueCount, &message.type, &message.id, message.topic, (char*)md->message->payload);
		if (result != CAYENNE_SUCCESS)
			return;

		result = MQTT_FAILURE;
		for (i = 0; i < CAYENNE_MAX_MESSAGE_HANDLERS; ++i) {
			if (client->messageHandlers[i].fp != NULL && client->messageHandlers[i].topic == message.topic &&
				(client->messageHandlers[i].channel == message.channel || client->messageHandlers[i].channel == CAYENNE_ALL_CHANNELS) && 
				(strcmp(client->messageHandlers[i].clientID, message.clientID) == 0))
			{
				client->messageHandlers[i].fp(&message);
				result = MQTT_SUCCESS;
			}
		}

		if (result == MQTT_FAILURE && client->defaultMessageHandler != NULL)	{
			client->defaultMessageHandler(&message);
		}
	}
}

/**
* Create a Cayenne MQTT client object
* @param[out] client The initialized client object
* @param[out] network The network connection
* @param[in] username Cayenne username
* @param[in] password Cayenne password
* @param[in] clientID Cayennne client ID
* @param[in] defaultHandler Default MQTT message handler, can be NULL
*/
void CayenneMQTTClientInit(CayenneMQTTClient* client, Network* network, const char* username, const char* password, const char* clientID, CayenneMessageHandler defaultHandler)
{
	int i;
	MQTTClientInit(&client->mqttClient, network, 30000, client->sendbuf, CAYENNE_MAX_MESSAGE_SIZE, client->readbuf, CAYENNE_MAX_MESSAGE_SIZE);
	for (i = 0; i < CAYENNE_MAX_MESSAGE_HANDLERS; ++i)
	{
		client->messageHandlers[i].clientID = NULL;
		client->messageHandlers[i].topic = UNDEFINED_TOPIC;
		client->messageHandlers[i].channel = CAYENNE_NO_CHANNEL;
		client->messageHandlers[i].fp = NULL;
	}
	client->defaultMessageHandler = defaultHandler;
	client->mqttClient.defaultMessageHandler = MQTTMessageArrived;
	client->mqttClient.userData = client;
	client->username = username;
	client->password = password;
	client->clientID = clientID;
}

/**
* Connect to the Cayenne server
* @param[in] client The client object
* @return success code
*/
int CayenneMQTTConnect(CayenneMQTTClient* client)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 3;
	data.clientID.cstring = (char*)client->clientID;
	data.username.cstring = (char*)client->username;
	data.password.cstring = (char*)client->password;
	return MQTTConnect(&client->mqttClient, &data);
}

/**
* Send data to Cayenne.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] type Optional type to use for a type=value pair, can be NULL
* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
* @param[in] value Data value
* @return success code
*/
int CayenneMQTTPublishData(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, const char* value)
{
	CayenneValuePair valuePair[1];
	valuePair[0].value = value;
	valuePair[0].unit = unit;
	return CayenneMQTTPublishDataArray(client, clientID, topic, channel, type, valuePair, 1);
}


/**
* Send data to Cayenne.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] type Optional type to use for a type=value pair, can be NULL
* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
* @param[in] value Data value
* @return success code
*/
int CayenneMQTTPublishDataInt(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, int value)
{
	char str[2 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
	itoa(value, str, 10);
#else
	snprintf(str, sizeof(str), "%d", value);
#endif
	return CayenneMQTTPublishData(client, clientID, topic, channel, type, unit, str);
}


/**
* Send data to Cayenne.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] type Optional type to use for a type=value pair, can be NULL
* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
* @param[in] value Data value
* @return success code
*/
int CayenneMQTTPublishDataUInt(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, unsigned int value)
{
	char str[1 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
	utoa(value, str, 10);
#else
	snprintf(str, sizeof(str), "%u", value);
#endif
	return CayenneMQTTPublishData(client, clientID, topic, channel, type, unit, str);
}


/**
* Send data to Cayenne.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] type Optional type to use for a type=value pair, can be NULL
* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
* @param[in] value Data value
* @return success code
*/
int CayenneMQTTPublishDataLong(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, long value)
{
	char str[2 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined(ENERGIA)
	ltoa(value, str, 10);
#else
	snprintf(str, sizeof(str), "%ld", value);
#endif
	return CayenneMQTTPublishData(client, clientID, topic, channel, type, unit, str);
}


/**
* Send data to Cayenne.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] type Optional type to use for a type=value pair, can be NULL
* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
* @param[in] value Data value
* @return success code
*/
int CayenneMQTTPublishDataULong(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, unsigned long value)
{
	char str[1 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined(ENERGIA)
	ultoa(value, str, 10);
#else
	snprintf(str, sizeof(str), "%lu", value);
#endif
	return CayenneMQTTPublishData(client, clientID, topic, channel, type, unit, str);
}


/**
* Send data to Cayenne.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] type Optional type to use for a type=value pair, can be NULL
* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
* @param[in] value Data value
* @return success code
*/
int CayenneMQTTPublishDataDouble(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, double value)
{
	char str[33];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined(ENERGIA) || defined (ESP8266)
	dtostrf(value, 5, 3, str);
#else
	snprintf(str, 33, "%2.3f", value);
#endif
	return CayenneMQTTPublishData(client, clientID, topic, channel, type, unit, str);
}


/**
* Send data to Cayenne.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] type Optional type to use for a type=value pair, can be NULL
* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
* @param[in] value Data value
* @return success code
*/
int CayenneMQTTPublishDataFloat(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, float value)
{
	char str[33];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined(ENERGIA) || defined (ESP8266)
	dtostrf(value, 5, 3, str);
#else
	snprintf(str, 33, "%2.3f", value);
#endif
	return CayenneMQTTPublishData(client, clientID, topic, channel, type, unit, str);
}

/**
* Send multiple value data array to Cayenne.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] type Optional type to use for a type=value pair, can be NULL
* @param[in] values Unit/value array
* @param[in] valueCount Number of values
* @return success code
*/
int CayenneMQTTPublishDataArray(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const CayenneValuePair* values, size_t valueCount)
{
	char buffer[CAYENNE_MAX_MESSAGE_SIZE + 1] = { 0 };
	int result = CayenneBuildTopic(buffer, sizeof(buffer), client->username, clientID ? clientID : client->clientID, topic, channel);
	if (result == CAYENNE_SUCCESS) {
		size_t size = strlen(buffer);
		char* payload = &buffer[size + 1];
		size = sizeof(buffer) - (size + 1);
		result = CayenneBuildDataPayload(payload, &size, type, values, valueCount);
		if (result == CAYENNE_SUCCESS) {
			MQTTMessage message;
			message.qos = QOS0;
			message.retained = 1;
			message.dup = 0;
			message.payload = (void*)payload;
			message.payloadlen = size;
			result = MQTTPublish(&client->mqttClient, buffer, &message);
		}
	}
	return result;
}

/**
* Send a response to a channel.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] id ID of message the response is for
* @param[in] error Optional error message, NULL for success
* @return success code
*/
int CayenneMQTTPublishResponse(CayenneMQTTClient* client, const char* clientID, const char* id, const char* error)
{
	char buffer[CAYENNE_MAX_MESSAGE_SIZE + 1] = { 0 };
	int result = CayenneBuildTopic(buffer, sizeof(buffer), client->username, clientID ? clientID : client->clientID, RESPONSE_TOPIC, CAYENNE_NO_CHANNEL);
	if (result == CAYENNE_SUCCESS) {
		size_t size = strlen(buffer);
		char* payload = &buffer[size + 1];
		size = sizeof(buffer) - (size + 1);
		result = CayenneBuildResponsePayload(payload, &size, id, error);
		if (result == CAYENNE_SUCCESS) {
			MQTTMessage message;
			message.qos = QOS1;
			message.retained = 1;
			message.dup = 0;
			message.payload = (void*)payload;
			message.payloadlen = size;
			result = MQTTPublish(&client->mqttClient, buffer, &message);
		}
	}
	return result;
}

/**
* Subscribe to a topic.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with. This string is not copied, so it must remain available for the life of the subscription.
* @param[in] topic Cayenne topic
* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
* @param[in] handler The message handler, NULL to use default handler
* @return success code
*/
int CayenneMQTTSubscribe(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, CayenneMessageHandler handler)
{
	char topicName[CAYENNE_MAX_MESSAGE_SIZE] = { 0 };
	int result = CayenneBuildTopic(topicName, sizeof(topicName), client->username, clientID ? clientID : client->clientID, topic, channel);
	if (result == CAYENNE_SUCCESS) {
		int i;
		int qos = QOS0;
		result = MQTTSubscribe(&client->mqttClient, topicName, qos, NULL);
		if (handler && result == qos)
		{
			for (i = 0; i < CAYENNE_MAX_MESSAGE_HANDLERS; ++i)
			{
				if (client->messageHandlers[i].fp == NULL)
				{
					client->messageHandlers[i].clientID = clientID;
					client->messageHandlers[i].topic = topic;
					client->messageHandlers[i].channel = channel;
					client->messageHandlers[i].fp = handler;
					break;
				}
			}
		}
	}
	return result;
}

/**
* Unsubscribe from a topic.
* @param[in] client The client object
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
* @param[in] topic Cayenne topic
* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
* @return success code
*/
int CayenneMQTTUnsubscribe(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel)
{
	char topicName[CAYENNE_MAX_MESSAGE_SIZE] = { 0 };
	int result = CayenneBuildTopic(topicName, sizeof(topicName), client->username, clientID ? clientID : client->clientID, topic, channel);
	if (result == CAYENNE_SUCCESS) {
		result = MQTTUnsubscribe(&client->mqttClient, topicName);
		if (result == MQTT_SUCCESS)
		{
			int i;
			for (i = 0; i < CAYENNE_MAX_MESSAGE_HANDLERS; ++i)
			{
				if ((client->messageHandlers[i].topic == topic && client->messageHandlers[i].channel == channel) &&
					(strcmp(clientID ? clientID : client->clientID, client->messageHandlers[i].clientID) == 0))
				{
					client->messageHandlers[i].clientID = NULL;
					client->messageHandlers[i].topic = UNDEFINED_TOPIC;
					client->messageHandlers[i].channel = CAYENNE_NO_CHANNEL;
					client->messageHandlers[i].fp = NULL;
				}
			}
		}
	}
	return result;
}

/**
* Disconnect from the Cayenne server.
* @param[in] client The client object
* @return success code
*/
int CayenneMQTTDisconnect(CayenneMQTTClient* client)
{
	return MQTTDisconnect(&client->mqttClient);
}

/**
* Check if the client is connected to the Cayenne server.
* @param[in] client The client object
* @return 1 if connected, 0 if not connected
*/
int CayenneMQTTConnected(CayenneMQTTClient* client)
{
	return client->mqttClient.isconnected;
}


/**
* Yield to allow MQTT message processing.
* @param[in] client The client object
* @param[in] time The time in milliseconds to yield for
* @return success code
*/
int CayenneMQTTYield(CayenneMQTTClient* client, int time)
{
	return MQTTYield(&client->mqttClient, time);
}
