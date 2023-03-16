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

#include <stdlib.h> 
#include <string.h> 
#include "CayenneUtils.h"

#ifdef DIGITAL_AND_ANALOG_SUPPORT
	#ifdef PARSE_INFO_PAYLOADS
		#define PARSE_TOPICS_COUNT 12
	#else
		#define PARSE_TOPICS_COUNT 5
	#endif
#else
	#ifdef PARSE_INFO_PAYLOADS
		#define PARSE_TOPICS_COUNT 6
	#else
		#define PARSE_TOPICS_COUNT 1
	#endif
#endif

#define THINGS_STRING CAYENNE_PSTR("/things/")

typedef struct TopicChannel
{
	CayenneTopic topic;
	unsigned int channel;
} TopicChannel;

/**
* Build a specified topic string.
* @param[out] topic Returned topic string
* @param[in] length CayenneTopic buffer length
* @param[in] username Cayenne username
* @param[in] clientID Cayennne client ID
* @param[in] suffix The topic suffix
* @return CAYENNE_SUCCESS if topic string was created, error code otherwise
*/
int buildTopic(char* topic, size_t length, const char* username, const char* clientID, const char* suffix) {
	size_t topicLength = 0;
	if (!topic || !username || !clientID || !suffix)
		return CAYENNE_FAILURE;
	topicLength = strlen(username) + strlen(clientID) + strlen(suffix) + 11;
	if (topicLength > length)
		return CAYENNE_BUFFER_OVERFLOW;

	topic[0] = '\0';
	strcat(topic, CAYENNE_VERSION);
	strcat(topic, "/");
	strcat(topic, username);
	CAYENNE_STRCAT(topic, THINGS_STRING);
	strcat(topic, clientID);
	strcat(topic, "/");
	strcat(topic, suffix);
	return CAYENNE_SUCCESS;
}

/**
* Build a specified topic suffix string.
* @param[out] suffix Returned suffix string
* @param[in] length Suffix buffer length
* @param[in] topic Cayenne topic
* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
* @return CAYENNE_SUCCESS if suffix string was created, error code otherwise
*/
int buildSuffix(char* suffix, size_t length, const CayenneTopic topic, unsigned int channel) {
	char* topicString = NULL;
	if (!suffix)
		return CAYENNE_FAILURE;
	switch (topic)
	{
	case COMMAND_TOPIC:
		topicString = COMMAND_STRING;
		break;
	case DATA_TOPIC:
		topicString = DATA_STRING;
		break;
	case RESPONSE_TOPIC:
		topicString = RESPONSE_STRING;
		break;
	case SYS_MODEL_TOPIC:
		topicString = SYS_MODEL_STRING;
		break;
	case SYS_VERSION_TOPIC:
		topicString = SYS_VERSION_STRING;
		break;
	case SYS_CPU_MODEL_TOPIC:
		topicString = SYS_CPU_MODEL_STRING;
		break;
	case SYS_CPU_SPEED_TOPIC:
		topicString = SYS_CPU_SPEED_STRING;
		break;
#ifdef DIGITAL_AND_ANALOG_SUPPORT
	case DIGITAL_TOPIC:
		topicString = DIGITAL_STRING;
		break;
	case DIGITAL_COMMAND_TOPIC:
		topicString = DIGITAL_COMMAND_STRING;
		break;
	case DIGITAL_CONFIG_TOPIC:
		topicString = DIGITAL_CONFIG_STRING;
		break;
	case ANALOG_TOPIC:
		topicString = ANALOG_STRING;
		break;
	case ANALOG_COMMAND_TOPIC:
		topicString = ANALOG_COMMAND_STRING;
		break;
	case ANALOG_CONFIG_TOPIC:
		topicString = ANALOG_CONFIG_STRING;
		break;
#endif
	default:
		return CAYENNE_FAILURE;
	}

	if (!topicString)
		return CAYENNE_FAILURE;
	if (CAYENNE_STRLEN(topicString) >= length)
		return CAYENNE_BUFFER_OVERFLOW;

	suffix[0] = '\0';
	CAYENNE_STRCAT(suffix, topicString);
	if (channel != CAYENNE_NO_CHANNEL) {
		strcat(suffix, "/");
		if (channel == CAYENNE_ALL_CHANNELS) {
			strcat(suffix, "+");
		}
		else {
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			itoa(channel, &suffix[strlen(suffix)], 10);
#else
			snprintf(&suffix[strlen(suffix)], length - strlen(suffix), "%u", channel);
#endif
		}
	}
	return CAYENNE_SUCCESS;
}

/**
* Check if topic matches.
* @param[in] filter Filter to check topic against
* @param[in] topicName CayenneTopic name
* @param[in] topicNameLen CayenneTopic name length
* return true if topic matches, false otherwise
*/
int topicMatches(char* filter, char* topicName, unsigned int topicNameLen)
{
	char* curf = filter;
	char* curn = topicName;
	char* curn_end = topicName + topicNameLen;

	while (*curf && curn < curn_end)
	{
		if (*curn == '/' && *curf != '/')
			break;
		if (*curf != '+' && *curf != '#' && *curf != *curn)
			break;
		if (*curf == '+')
		{   // skip until we meet the next separator, or end of string
			char* nextpos = curn + 1;
			while (nextpos < curn_end && *nextpos != '/')
				nextpos = ++curn + 1;
		}
		else if (*curf == '#')
			curn = curn_end - 1;    // skip until end of string
		curf++;
		curn++;
	};

	return ((curn == curn_end) && (*curf == '\0'));
}

/**
* Get the count of values in a message.
* @param[out] count Returned number of values found in message
* @param[in] payload Payload string, must be null terminated
* @param[in] token Character token for splitting "unit=value" payloads, 0 to just parse first comma delimited value
* @return CAYENNE_SUCCESS if value count succeeded, error code otherwise
*/
int getValueCount(size_t* count, char* payload, char token) {
	char* index = payload;
	size_t unitCount = 0;
	size_t valueCount = 0;
	int countingValues = 0;

	if (token == 0) {
		//Currently there can only be one value in payload if this isn't a "unit=value" payload.
		*count = 1;
		return CAYENNE_SUCCESS;
	}

	*count = 0;
	while (index && *index != '\0') {
		if ((*index == ',') || (*index == token)) {
			if (*index == ',') {
				if (countingValues) {
					valueCount++;
				}
				else {
					unitCount++;
				}
			}
			else if (*index == token) {
				countingValues = 1;
				valueCount++;
			}
		}
		index++;
	}
	
	if (countingValues) {
		if ((valueCount != unitCount) && !(unitCount == 0 && valueCount == 1)) {
			return CAYENNE_FAILURE;
		}
	}
	else {
		valueCount = 1;
	}
	*count = valueCount;
	return CAYENNE_SUCCESS;
}

/**
* Parse a null terminated payload string in place. This may modify the payload string.
* @param[out] values Returned payload data unit & value array
* @param[in,out] valuesSize Size of values array, returns the count of values in the array
* @param[out] type Returned type, NULL if there is none
* @param[in] payload Payload string, must be null terminated
* @param[in] token Character token for splitting "unit=value" payloads, 0 to just parse first comma delimited value
* @return CAYENNE_SUCCESS if value and id were parsed, error code otherwise
*/
int parsePayload(CayenneValuePair* values, size_t* valuesSize, const char** type, char* payload, char token) {
	char* index = payload;
	size_t count = 0;
	int parsingValues = 0;
	size_t valueIndex = 0;
#ifdef PARSE_INFO_PAYLOADS
	int result = getValueCount(&count, payload, token);
	if (result != CAYENNE_SUCCESS) {
		*valuesSize = 0;
		return result;
	}
#else
	count = 1;
#endif

	if(token == 0)
		parsingValues = 1; //Don't need to parse units if there is no unit/value separator

	values[0].value = NULL;
	values[0].unit = NULL;
	*type = NULL;
	while (index && *index != '\0') {
		if ((*index == ',') || (*index == token)) {
			if (*index == ',') {
				*type = payload;
				if (valueIndex < *valuesSize) {
					if (parsingValues) {
						values[valueIndex].value = index + 1;
					}
					else {
						values[valueIndex].unit = index + 1;
					}
				}
				*index = '\0';
				valueIndex++;
				if (token == 0)
					break;
			}
			else if (*index == token && !parsingValues) {
				parsingValues = 1;
				valueIndex = 0;
				*type = payload;
				values[valueIndex].value = index + 1;
				*index = '\0';
				valueIndex++;
				if (count == valueIndex)
					break;
			}
		}
		index++;
	};
	*valuesSize = count;
	return CAYENNE_SUCCESS;
}

/**
* Build a specified topic string.
* @param[out] topicName Returned topic string
* @param[in] length CayenneTopic buffer length
* @param[in] username Cayenne username
* @param[in] clientID Cayennne client ID
* @param[in] topic Cayenne topic
* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
* @return CAYENNE_SUCCESS if topic string was created, error code otherwise
*/
int CayenneBuildTopic(char* topicName, size_t length, const char* username, const char* clientID, CayenneTopic topic, unsigned int channel) {
	char channelSuffix[20] = {0};
	int result = buildSuffix(channelSuffix, sizeof(channelSuffix), topic, channel);
	if (result != CAYENNE_SUCCESS)
		return result;
	return buildTopic(topicName, length, username, clientID, channelSuffix);
}

/**
* Build a specified data payload.
* @param[out] payload Returned payload
* @param[in,out] length Payload buffer length
* @param[in] type Optional type to use for type,unit=value payload, can be NULL
* @param[in] values Unit/value array
* @param[in] valueCount Number of values
* @return CAYENNE_SUCCESS if topic string was created, error code otherwise
*/
int CayenneBuildDataPayload(char* payload, size_t* length, const char* type, const CayenneValuePair* values, size_t valueCount) {
	int i;
	size_t payloadLength = 0;
	for (i = 0; i < valueCount; ++i) {
		if (values[i].unit) {
			payloadLength += strlen(values[i].unit) + 1;
		}
		else if (type) {
			// If type exists but unit does not, use UNIT_UNDEFINED for the unit.
			payloadLength += strlen(UNIT_UNDEFINED) + 1;
		}
		payloadLength += values[i].value ? strlen(values[i].value) + 1 : 0;
	}
	payloadLength += type ? strlen(type) + 1 : 0;
	//If payload can't fit the payload plus a terminating null byte return.
	if (payloadLength > *length) {
		return CAYENNE_BUFFER_OVERFLOW;
	}

	payload[0] = '\0';
	if (type) {
		strcat(payload, type);
	}
	for (i = 0; i < valueCount; ++i) {
		if (payload[0] != '\0')
			strcat(payload, ",");
		if (values[i].unit)
			strcat(payload, values[i].unit);
		else if (type)
			strcat(payload, UNIT_UNDEFINED);
	}
	if (payload[0] != '\0' && valueCount > 0 && values[0].value)
		strcat(payload, "=");
	for (i = 0; i < valueCount && values[i].value; ++i) {
		strcat(payload, values[i].value);
		if (i + 1 < valueCount)
			strcat(payload, ",");
	}
	*length = --payloadLength; //Subtract terminating null 
	return CAYENNE_SUCCESS;
}

/**
* Build a specified response payload.
* @param[out] payload Returned payload
* @param[in,out] length Payload buffer length
* @param[in] id ID of message the response is for
* @param[in] error Optional error message, NULL for success
* @return CAYENNE_SUCCESS if payload string was created, error code otherwise
*/
int CayenneBuildResponsePayload(char* payload, size_t* length, const char* id, const char* error) {
	CayenneValuePair values[1];
	values[0].unit = id;
	values[0].value = error;
	if (error) {
		return CayenneBuildDataPayload(payload, length, "error", values, 1);
	}
	return CayenneBuildDataPayload(payload, length, "ok", values, 1);
}

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
int CayenneParseTopic(CayenneTopic* topic, unsigned int* channel, const char** clientID, const char* username, char* topicName, unsigned int length) {
	char* index = NULL;
	int i = 0;
	TopicChannel parseTopics[PARSE_TOPICS_COUNT] = { { COMMAND_TOPIC, CAYENNE_ALL_CHANNELS },
#ifdef DIGITAL_AND_ANALOG_SUPPORT
		{ ANALOG_COMMAND_TOPIC, CAYENNE_ALL_CHANNELS },{ ANALOG_CONFIG_TOPIC, CAYENNE_ALL_CHANNELS },{ DIGITAL_COMMAND_TOPIC, CAYENNE_ALL_CHANNELS },{ DIGITAL_CONFIG_TOPIC, CAYENNE_ALL_CHANNELS },
#ifdef PARSE_INFO_PAYLOADS
		{ DIGITAL_TOPIC, CAYENNE_ALL_CHANNELS },{ ANALOG_TOPIC, CAYENNE_ALL_CHANNELS },
#endif
#endif
#ifdef PARSE_INFO_PAYLOADS
		{ DATA_TOPIC, CAYENNE_ALL_CHANNELS },{ SYS_MODEL_TOPIC, CAYENNE_NO_CHANNEL },{ SYS_VERSION_TOPIC, CAYENNE_NO_CHANNEL },{ SYS_CPU_MODEL_TOPIC, CAYENNE_NO_CHANNEL },{ SYS_CPU_SPEED_TOPIC, CAYENNE_NO_CHANNEL }
#endif
	};

	if (!topic || !channel || !topicName)
	{
		return CAYENNE_FAILURE;
	}
	if (length > CAYENNE_MAX_MESSAGE_SIZE)
	{
		return CAYENNE_BUFFER_OVERFLOW;
	}
	if (strncmp(CAYENNE_VERSION, topicName, strlen(CAYENNE_VERSION)) != 0)
		return CAYENNE_FAILURE;
	index = topicName + strlen(CAYENNE_VERSION) + 1;
	if (strncmp(username, index, strlen(username)) != 0)
		return CAYENNE_FAILURE;
	index += strlen(username);
	if (CAYENNE_STRNCMP(index, THINGS_STRING, CAYENNE_STRLEN(THINGS_STRING)) != 0)
		return CAYENNE_FAILURE;
	index += CAYENNE_STRLEN(THINGS_STRING);
	char* deviceIDEnd = strchr(index, '/');
	if (!deviceIDEnd)
		return CAYENNE_FAILURE;
	*clientID = index;
	*deviceIDEnd = '\0';

	index = deviceIDEnd + 1;
	*topic = UNDEFINED_TOPIC;
	*channel = CAYENNE_NO_CHANNEL;
	length -= (index - topicName);
	for (i = 0; i < PARSE_TOPICS_COUNT; ++i)
	{
		char channelSuffix[32] = { 0 };
		if (buildSuffix(channelSuffix, sizeof(channelSuffix), parseTopics[i].topic, parseTopics[i].channel) == CAYENNE_SUCCESS && topicMatches(channelSuffix, index, length)) {
			*topic = parseTopics[i].topic;
			break;
		}
	}

	if (*topic == UNDEFINED_TOPIC)
		return CAYENNE_FAILURE;

	if (parseTopics[i].channel != CAYENNE_NO_CHANNEL) {
		if (length == 0 || length > 31)
			return CAYENNE_FAILURE;
		char* channelIndex = NULL;
		char buffer[32] = { 0 };
		memcpy(buffer, index, length);
		buffer[length] = '\0';
		channelIndex = strrchr(buffer, '/');
		if (channelIndex && ++channelIndex) {
			char* indexEnd = NULL;
			unsigned int channelNumber = strtoul(channelIndex, &indexEnd, 10);
			if (indexEnd && *indexEnd == '\0') {
				if (((channelNumber != 0) && (*channelIndex != '0')) || ((channelNumber == 0) && (*channelIndex == '0') && (channelIndex + 1 == indexEnd))) {
					*channel = channelNumber;
				}
			}
		}
	}

	return CAYENNE_SUCCESS;
}

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
int CayenneParsePayload(CayenneValuePair* values, size_t* valuesSize, const char** type, const char** id, CayenneTopic topic, char* payload) {
	int i;
	if (!payload || !valuesSize || *valuesSize == 0)
		return CAYENNE_FAILURE;

	*type = NULL;
	*id = NULL;
	for(i = 0; i < *valuesSize; i++) {
		values[i].unit = NULL;
		values[i].value = NULL;
	}
	switch (topic)
	{
#ifdef PARSE_INFO_PAYLOADS
	case DATA_TOPIC:
		parsePayload(values, valuesSize, type, payload, '=');
		if (!values[0].value)
			return CAYENNE_FAILURE;
		break;
#endif
#ifdef DIGITAL_AND_ANALOG_SUPPORT
#ifdef PARSE_INFO_PAYLOADS
	case ANALOG_TOPIC:
		parsePayload(values, valuesSize, type, payload, 0);
		values[0].unit = values[0].value; //Use unit to store resolution
		values[0].value = *type;
		*type = NULL; 
		if (!values[0].value)
			return CAYENNE_FAILURE;
		break;
#endif
	case DIGITAL_COMMAND_TOPIC:
	case ANALOG_COMMAND_TOPIC:
#endif
	case COMMAND_TOPIC:
		parsePayload(values, valuesSize, type, payload, 0);
		*id = *type;
		*type = NULL;
		if (!values[0].value)
			return CAYENNE_FAILURE;
		break;
	default:
		break;
	}

	if (!values[0].value) {
		values[0].value = payload;
		values[0].unit = NULL;
		*type = NULL;
		*id = NULL;
		*valuesSize = 1;
	}

	return CAYENNE_SUCCESS;
}



