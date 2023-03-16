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

#ifndef _CAYENNEMQTTCLIENT_h
#define _CAYENNEMQTTCLIENT_h

#include "MQTTClient.h"
#include "../CayenneUtils/CayenneDefines.h"
#include "../CayenneUtils/CayenneUtils.h"
#include "../CayenneUtils/CayenneDataArray.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(WIN32_DLL) || defined(WIN64_DLL)
#define DLLImport __declspec(dllimport)
#define DLLExport __declspec(dllexport)
#elif defined(LINUX_SO)
#define DLLImport extern
#define DLLExport  __attribute__ ((visibility ("default")))
#else
#define DLLImport
#define DLLExport
#endif

	/**
	* Cayenne message data passed to message handler functions.
	*/
	typedef struct CayenneMessageData
	{
		const char* clientID; /**< The client ID of the message. */
		CayenneTopic topic; /**< The topic the message was received on. */
		unsigned int channel; /**< The channel the message was received on. */
		const char* id; /**< The message ID, if it is a command message, otherwise NULL. */
		const char* type; /**< The type of data in the message, if it exists, otherwise NULL. */
		CayenneValuePair values[CAYENNE_MAX_MESSAGE_VALUES]; /**< The unit/value data pairs in the message. The units and values can be NULL. */
		size_t valueCount; /**< The count of items in the values array. */
	} CayenneMessageData;

	typedef void(*CayenneMessageHandler)(CayenneMessageData*);

	/**
	* Cayenne MQTT client data.
	*/
	typedef struct CayenneMQTTClient
	{
		MQTTClient mqttClient; /**< MQTT client struct. */
		const char* username; /**< Cayenne MQTT username. */
		const char* password; /**< Cayenne MQTT pasword. */
		const char* clientID; /**< Cayenne MQTT client ID. */
		unsigned char sendbuf[CAYENNE_MAX_MESSAGE_SIZE + 1]; /**< Buffer used for sending data. */
		unsigned char readbuf[CAYENNE_MAX_MESSAGE_SIZE + 1]; /**< Buffer used for receiving data. */

		/**
		* Cayenne custom message handler data.
		*/
		struct CayenneMessageHandlers
		{
			const char* clientID; /**< Client ID of the message to handle. */
			CayenneTopic topic; /**< Topic of the message to handle. */
			unsigned int channel; /**< Channel of the message to handle. */
			void(*fp) (CayenneMessageData*); /**< Custom message handler function. */
		} messageHandlers[CAYENNE_MAX_MESSAGE_HANDLERS];  /**< Custom message handler array. */

		void(*defaultMessageHandler) (CayenneMessageData*); /**< Default message handler used if no custom handlers match the received message topic. */
	} CayenneMQTTClient;

	/**
	* Create a Cayenne MQTT client object
	* @param[out] client The initialized client object
	* @param[out] network The network connection
	* @param[in] username Cayenne username
	* @param[in] password Cayenne password
	* @param[in] clientID Cayennne client ID
	* @param[in] defaultHandler Default MQTT message handler, can be NULL
	*/
	DLLExport void CayenneMQTTClientInit(CayenneMQTTClient* client, Network* network, const char* username, const char* password, const char* clientID, CayenneMessageHandler defaultHandler);

	/**
	* Connect to the Cayenne server.
	* @param[in] client The client object
	* @return success code
	*/
	DLLExport int CayenneMQTTConnect(CayenneMQTTClient* client);

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
	DLLExport int CayenneMQTTPublishData(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, const char* value);

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
	DLLExport int CayenneMQTTPublishDataInt(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, int value);

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
	DLLExport int CayenneMQTTPublishDataUInt(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, unsigned int value);

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
	DLLExport int CayenneMQTTPublishDataLong(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, long value);

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
	DLLExport int CayenneMQTTPublishDataULong(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, unsigned long value);

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
	DLLExport int CayenneMQTTPublishDataDouble(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, double value);

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
	DLLExport int CayenneMQTTPublishDataFloat(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const char* unit, float value);

	/**
	* Send multiple value data array to Cayenne.
	* @param[in] client The client object
	* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
	* @param[in] topic Cayenne topic
	* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
	* @param[in] type Optional type to use for a type=value pair, can be NULL
	* @param[in] values Unit / value array
	* @param[in] valueCount Number of values
	* @return success code
	*/
	DLLExport int CayenneMQTTPublishDataArray(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, const char* type, const CayenneValuePair* values, size_t valueCount);

	/**
	* Send a response to a channel.
	* @param[in] client The client object
	* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
	* @param[in] id ID of message the response is for
	* @param[in] error Optional error message, NULL for success
	* @return success code
	*/
	DLLExport int CayenneMQTTPublishResponse(CayenneMQTTClient* client, const char* clientID, const char* id, const char* error);

	/**
	* Subscribe to a topic.
	* @param[in] client The client object
	* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with. This string is not copied, so it must remain available for the life of the subscription.
	* @param[in] topic Cayenne topic
	* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
	* @param[in] handler The message handler, NULL to use default handler
	* @return success code
	*/
	DLLExport int CayenneMQTTSubscribe(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel, CayenneMessageHandler handler);

	/**
	* Unsubscribe from a topic.
	* @param[in] client The client object
	* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
	* @param[in] topic Cayenne topic
	* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
	* @return success code
	*/
	DLLExport int CayenneMQTTUnsubscribe(CayenneMQTTClient* client, const char* clientID, CayenneTopic topic, unsigned int channel);

	/**
	* Disconnect from the Cayenne server.
	* @param[in] client The client object
	* @return success code
	*/
	DLLExport int CayenneMQTTDisconnect(CayenneMQTTClient* client);

	/**
	* Check if the client is connected to the Cayenne server.
	* @param[in] client The client object
	* @return 1 if connected, 0 if not connected
	*/
	DLLExport int CayenneMQTTConnected(CayenneMQTTClient* client);

	/**
	* Yield to allow MQTT message processing.
	* @param[in] client The client object
	* @param[in] time The time in milliseconds to yield for
	* @return success code
	*/
	DLLExport int CayenneMQTTYield(CayenneMQTTClient* client, int time);


#if defined(__cplusplus)
	 }
#endif

#endif
