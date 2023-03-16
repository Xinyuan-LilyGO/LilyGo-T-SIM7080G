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

#ifndef _CAYENNEUTILS_h
#define _CAYENNEUTILS_h

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

#include <stdio.h>
#include "CayenneDefines.h"

enum CayenneReturnCode { CAYENNE_BUFFER_OVERFLOW = -2, CAYENNE_FAILURE = -1, CAYENNE_SUCCESS = 0 };

/**
* A unit/value pair used in Cayenne payloads.
*/
typedef struct CayenneValuePair
{
	const char* unit; /**< The data unit. */
	const char* value; /**< The data value. */
} CayenneValuePair;

/**
* Build a specified topic string.
* @param[out] topicName Returned topic string
* @param[in] length CayenneTopic buffer length
* @param[in] username Cayenne username
* @param[in] clientID Cayennne client ID
* @param[in] topic Cayenne topic
* @param[in] channel The topic channel, use CAYENNE_NO_CHANNEL if none is required, CAYENNE_ALL_CHANNELS if a wildcard is required
* @return CAYENNE_SUCCESS if topic string was created, error code otherwise
*/
DLLExport int CayenneBuildTopic(char* topicName, size_t length, const char* username, const char* clientID, CayenneTopic topic, unsigned int channel);

/**
* Build a specified data payload.
* @param[out] payload Returned payload
* @param[in,out] length Payload buffer length
* @param[in] type Optional type to use for type,unit=value payload, can be NULL
* @param[in] values Unit/value array
* @param[in] valueCount Number of values
* @return CAYENNE_SUCCESS if topic string was created, error code otherwise
*/
DLLExport int CayenneBuildDataPayload(char* payload, size_t* length, const char* type, const CayenneValuePair* values, size_t valueCount);

/**
* Build a specified response payload.
* @param[out] payload Returned payload
* @param[in,out] length Payload buffer length
* @param[in] id ID of message the response is for
* @param[in] error Optional error message, NULL for success
* @return CAYENNE_SUCCESS if topic string was created, error code otherwise
*/
DLLExport int CayenneBuildResponsePayload(char* payload, size_t* length, const char* id, const char* error);

/**
* Parse a topic string in place. This may modify the topic string.
* @param[out] topic Returned Cayenne topic
* @param[out] channel Returned channel, CAYENNE_NO_CHANNEL if there is none
* @param[out] clientID Returned client ID
* @param[in] username Cayenne username
* @param[in] topicName Topic name string
* @param[in] length Topic name string length
* @return CAYENNE_SUCCESS if topic was parsed, error code otherwise
*/
DLLExport int CayenneParseTopic(CayenneTopic* topic, unsigned int* channel, const char** clientID, const char* username, char* topicName, unsigned int length);

/**
* Parse a null terminated payload in place. This may modify the payload string. 
* @param[out] values Returned payload data unit & value array
* @param[in,out] valuesSize Size of values array, returns the count of values in the array
* @param[out] type Returned type, NULL if there is none
* @param[out] id Returned message id, empty string if there is none
* @param[in] topic Cayenne topic
* @param[in] payload Payload string, must be null terminated.
* @return CAYENNE_SUCCESS if topic string was created, error code otherwise
*/
DLLExport int CayenneParsePayload(CayenneValuePair* values, size_t* valuesSize, const char** type, const char** id, CayenneTopic topic, char* payload);

#if defined(__cplusplus)
}
#endif

#endif