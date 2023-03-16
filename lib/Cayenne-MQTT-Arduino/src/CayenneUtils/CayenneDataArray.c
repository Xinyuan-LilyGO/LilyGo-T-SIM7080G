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

#include "CayenneDataArray.h"
#include <string.h>
#include <stdio.h>


/**
* Initialize a data array of unit/value pairs.
* @param[out] dataArray The initialized data array
* @param[in] buffer Buffer for storing unit/value pairs. This buffer should be available for as long as the data array is used.
* @param[in] bufferSize Size of the buffer
*/
void CayenneDataArrayInit(CayenneDataArray* dataArray, char* buffer, unsigned int bufferSize)
{
	dataArray->valueCount = 0;
	dataArray->buffer = buffer;
	dataArray->bufferSize = bufferSize;
	dataArray->bufferIndex = 0;
}

/**
* Add the specified unit/value pair to the array.
* @param[in] dataArray The data array to add values to
* @param[in] unit The unit to add
* @param[in] value The value to add
* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
*/
int CayenneDataArrayAdd(CayenneDataArray* dataArray, const char* unit, const char* value)
{
	if (dataArray->valueCount >= CAYENNE_MAX_MESSAGE_VALUES)
		return CAYENNE_FAILURE;

	size_t unitLength = unit ? strlen(unit) + 1 : 0;
	size_t valueLength = value ? strlen(value) + 1 : 0;
	if (dataArray->bufferIndex + unitLength + valueLength > dataArray->bufferSize)
		return CAYENNE_FAILURE;

	if (unit) {
		memcpy(dataArray->buffer + dataArray->bufferIndex, unit, unitLength);
		dataArray->values[dataArray->valueCount].unit = dataArray->buffer + dataArray->bufferIndex;
		dataArray->bufferIndex += unitLength;
	}
	else {
		dataArray->values[dataArray->valueCount].unit = NULL;
	}

	if (value) {
		memcpy(dataArray->buffer + dataArray->bufferIndex, value, valueLength);
		dataArray->values[dataArray->valueCount].value = dataArray->buffer + dataArray->bufferIndex;
		dataArray->bufferIndex += valueLength;
	}
	else {
		dataArray->values[dataArray->valueCount].value = NULL;
	}

	dataArray->valueCount++;
	return CAYENNE_SUCCESS;
}

/**
* Add the specified unit/value pair to the array.
* @param[in] dataArray The data array to add values to
* @param[in] unit The unit to add
* @param[in] value The value to add
* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
*/
int CayenneDataArrayAddInt(CayenneDataArray* dataArray, const char* unit, int value)
{
	char str[2 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
	itoa(value, str, 10);
#else
	snprintf(str, sizeof(str), "%d", value);
#endif
	return CayenneDataArrayAdd(dataArray, unit, str);
}

/**
* Add the specified unit/value pair to the array.
* @param[in] dataArray The data array to add values to
* @param[in] unit The unit to add
* @param[in] value The value to add
* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
*/
int CayenneDataArrayAddUInt(CayenneDataArray* dataArray, const char* unit, unsigned int value)
{
	char str[1 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
	utoa(value, str, 10);
#else
	snprintf(str, sizeof(str), "%u", value);
#endif
	return CayenneDataArrayAdd(dataArray, unit, str);
}

/**
* Add the specified unit/value pair to the array.
* @param[in] dataArray The data array to add values to
* @param[in] unit The unit to add
* @param[in] value The value to add
* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
*/
int CayenneDataArrayAddLong(CayenneDataArray* dataArray, const char* unit, long value)
{
	char str[2 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
	ltoa(value, str, 10);
#else
	snprintf(str, sizeof(str), "%ld", value);
#endif
	return CayenneDataArrayAdd(dataArray, unit, str);
}

/**
* Add the specified unit/value pair to the array.
* @param[in] dataArray The data array to add values to
* @param[in] unit The unit to add
* @param[in] value The value to add
* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
*/
int CayenneDataArrayAddULong(CayenneDataArray* dataArray, const char* unit, unsigned long value)
{
	char str[1 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
	ultoa(value, str, 10);
#else
	snprintf(str, sizeof(str), "%lu", value);
#endif
	return CayenneDataArrayAdd(dataArray, unit, str);
}

/**
* Add the specified unit/value pair to the array.
* @param[in] dataArray The data array to add values to
* @param[in] unit The unit to add
* @param[in] value The value to add
* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
*/
int CayenneDataArrayAddDouble(CayenneDataArray* dataArray, const char* unit, double value)
{
	char str[33];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined (ESP8266)
	dtostrf(value, 5, 3, str);
#else
	snprintf(str, 33, "%2.3f", value);
#endif
	return CayenneDataArrayAdd(dataArray, unit, str);
}

/**
* Add the specified unit/value pair to the array.
* @param[in] dataArray The data array to add values to
* @param[in] unit The unit to add
* @param[in] value The value to add
* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
*/
int CayenneDataArrayAddFloat(CayenneDataArray* dataArray, const char* unit, float value)
{
	char str[33];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined (ESP8266)
	dtostrf(value, 5, 3, str);
#else
	snprintf(str, 33, "%2.3f", value);
#endif
	return CayenneDataArrayAdd(dataArray, unit, str);
}

/**
* Clear the data array.
* @param[in] dataArray The data array to clear
*/
void CayenneDataArrayClear(CayenneDataArray* dataArray)
{
	dataArray->valueCount = 0;
	dataArray->bufferIndex = 0;
	memset(dataArray->buffer, 0, dataArray->bufferSize);
}
