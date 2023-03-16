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

#ifndef _CAYENNEDATAARRAY_h
#define _CAYENNEDATAARRAY_h

#include "CayenneUtils.h"

#if defined(__cplusplus)

// C++ version of the data array. This is defined here so it requires no source file.

namespace CayenneMQTT
{
	/**
	* @class DataArray
	* Class for manipulating a data array of unit/value pairs.
	* @param BUFFER_SIZE Maximum buffer size to use for data array, in bytes.
	* @param MAX_VALUES Maximum number of unit/value pairs in the array.
	*/
	template<int BUFFER_SIZE = CAYENNE_MAX_PAYLOAD_SIZE, int MAX_VALUES = CAYENNE_MAX_MESSAGE_VALUES>
	class DataArray
	{
	public:
		/**
		* Construct an empty array. 
		*/
		DataArray() {
			clear();
		}

		/**
		* Clear the array.
		*/
		void clear() {
			for (int i = 0; i < CAYENNE_MAX_MESSAGE_VALUES; ++i) {
				_values[i].unit = NULL;
				_values[i].value = NULL;
			}
			_valueCount = 0;
			_index = 0;
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		* @param[in] unitInFlash If true the unit string is in flash memory, otherwise false.
		* @param[in] valueInFlash If true the value string is in flash memory, otherwise false.
		*/
		void add(const char* unit, const char* value, bool unitInFlash = false, bool valueInFlash = false) {
			if (_valueCount >= CAYENNE_MAX_MESSAGE_VALUES)
				return;

			size_t unitLength = 0;
			if (unit) {
				unitLength = (unitInFlash ? CAYENNE_STRLEN(unit) : strlen(unit)) + 1;
			}
			size_t valueLength = 0;
			if (value) {
				valueLength = (valueInFlash ? CAYENNE_STRLEN(value) : strlen(value)) + 1;
			}
			if (_index + unitLength + valueLength > BUFFER_SIZE)
				return;

			if (unit) {
				unitInFlash ? CAYENNE_MEMCPY(_buffer + _index, unit, unitLength) : memcpy(_buffer + _index, unit, unitLength);
				_values[_valueCount].unit = _buffer + _index;
				_index += unitLength;
			}
			else {
				_values[_valueCount].unit = NULL;
			}

			if (value) {
				valueInFlash ? CAYENNE_MEMCPY(_buffer + _index, value, valueLength) : memcpy(_buffer + _index, value, valueLength);
				_values[_valueCount].value = _buffer + _index;
				_index += valueLength;
			}
			else {
				_values[_valueCount].value = NULL;
			}

			_valueCount++;
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const char* unit, const int value) {
			char str[2 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			itoa(value, str, 10);
#else
			snprintf(str, sizeof(str), "%d", value);
#endif
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const char* unit, const unsigned int value) {
			char str[1 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			utoa(value, str, 10);
#else
			snprintf(str, sizeof(str), "%u", value);
#endif
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const char* unit, const long value) {
			char str[2 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			ltoa(value, str, 10);
#else
			snprintf(str, sizeof(str), "%ld", value);
#endif
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const char* unit, const unsigned long value) {
			char str[1 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			ultoa(value, str, 10);
#else
			snprintf(str, sizeof(str), "%lu", value);
#endif
			add(unit, str);
		}

#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined (ESP8266)
		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const char* unit, const float value) {
			char str[33];
			dtostrf(value, 5, 3, str);
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const char* unit, const double value) {
			char str[33];
			dtostrf(value, 5, 3, str);
			add(unit, str);
		}

#else
		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const char* unit, const float value) {
			char str[33];
			snprintf(str, 33, "%2.3f", value);
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const char* unit, const double value) {
			char str[33];
			snprintf(str, 33, "%2.3f", value);
			add(unit, str);
		}

#endif

#ifdef CAYENNE_USING_PROGMEM
		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		void add(const char* unit, const __FlashStringHelper* value) {
			const char* valueString = reinterpret_cast<const char *>(value);
			add(unit, valueString, false, true);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		void add(const __FlashStringHelper* unit, const char* value) {
			const char* unitString = reinterpret_cast<const char *>(unit);
			add(unitString, value, true, false);
		}
		
		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		void add(const __FlashStringHelper* unit, const __FlashStringHelper* value) {
			const char* unitString = reinterpret_cast<const char *>(unit);
			const char* valueString = reinterpret_cast<const char *>(value);
			add(unitString, valueString, true, true);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const __FlashStringHelper* unit, const int value) {
			char str[2 + 8 * sizeof(value)];
			itoa(value, str, 10);
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const __FlashStringHelper* unit, const unsigned int value) {
			char str[1 + 8 * sizeof(value)];
			utoa(value, str, 10);
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const __FlashStringHelper* unit, const long value) {
			char str[2 + 8 * sizeof(value)];
			ltoa(value, str, 10);
			add(unit, str);
		}
		
		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const __FlashStringHelper* unit, const unsigned long value) {
			char str[1 + 8 * sizeof(value)];
			ultoa(value, str, 10);
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const __FlashStringHelper* unit, const float value) {
			char str[33];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined (ESP8266)
			dtostrf(value, 5, 3, str);
#else
			snprintf(str, 33, "%2.3f", value);
#endif
			add(unit, str);
		}

		/**
		* Add the specified unit/value pair to the array.
		* @param[in] unit The unit to add.
		* @param[in] value The value to add.
		*/
		inline void add(const __FlashStringHelper* unit, const double value) {
			char str[33];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32) || defined (ESP8266)
			dtostrf(value, 5, 3, str);
#else
			snprintf(str, 33, "%2.3f", value);
#endif
			add(unit, str);
		}

#endif
		/**
		* Get the unit/value pair array.
		* @return Pointer to the array.
		*/
		const CayenneValuePair* getArray() const {
			return _values;
		}

		/**
		* Get the number of items in the unit/value pair array.
		* @return Count of items.
		*/
    	const size_t getCount() const {
			return _valueCount;
		}

	private:
		CayenneValuePair _values[MAX_VALUES];
		size_t _valueCount;
		char _buffer[BUFFER_SIZE];
		size_t _index;
	};
}

typedef CayenneMQTT::DataArray<> CayenneDataArray;

#else

	// C version of the data array. Requires source file to be compiled and linked.

	typedef struct CayenneDataArray
	{
		CayenneValuePair values[CAYENNE_MAX_MESSAGE_VALUES];
		unsigned int valueCount;
		char* buffer;
		unsigned int bufferSize;
		unsigned int bufferIndex;
	} CayenneDataArray;

	/**
	* Initialize a data array of unit/value pairs.
	* @param[out] dataArray The initialized data array
	* @param[in] buffer Buffer for storing unit/value pairs. This buffer should be available for as long as the data array is used.
	* @param[in] bufferSize Size of the buffer
	*/
	DLLExport void CayenneDataArrayInit(CayenneDataArray* dataArray, char* buffer, unsigned int bufferSize);

	/**
	* Add the specified unit/value pair to the array.
	* @param[in] dataArray The data array to add values to
	* @param[in] unit The unit to add
	* @param[in] value The value to add
	* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
	*/
	DLLExport int CayenneDataArrayAdd(CayenneDataArray* dataArray, const char* unit, const char* value);

	/**
	* Add the specified unit/value pair to the array.
	* @param[in] dataArray The data array to add values to
	* @param[in] unit The unit to add
	* @param[in] value The value to add
	* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
	*/
	DLLExport int CayenneDataArrayAddInt(CayenneDataArray* dataArray, const char* unit, int value);

	/**
	* Add the specified unit/value pair to the array.
	* @param[in] dataArray The data array to add values to
	* @param[in] unit The unit to add
	* @param[in] value The value to add
	* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
	*/
	DLLExport int CayenneDataArrayAddUInt(CayenneDataArray* dataArray, const char* unit, unsigned int value);

	/**
	* Add the specified unit/value pair to the array.
	* @param[in] dataArray The data array to add values to
	* @param[in] unit The unit to add
	* @param[in] value The value to add
	* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
	*/
	DLLExport int CayenneDataArrayAddLong(CayenneDataArray* dataArray, const char* unit, long value);

	/**
	* Add the specified unit/value pair to the array.
	* @param[in] dataArray The data array to add values to
	* @param[in] unit The unit to add
	* @param[in] value The value to add
	* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
	*/
	DLLExport int CayenneDataArrayAddULong(CayenneDataArray* dataArray, const char* unit, unsigned long value);

	/**
	* Add the specified unit/value pair to the array.
	* @param[in] dataArray The data array to add values to
	* @param[in] unit The unit to add
	* @param[in] value The value to add
	* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
	*/
	DLLExport int CayenneDataArrayAddDouble(CayenneDataArray* dataArray, const char* unit, double value);

	/**
	* Add the specified unit/value pair to the array.
	* @param[in] dataArray The data array to add values to
	* @param[in] unit The unit to add
	* @param[in] value The value to add
	* @return CAYENNE_SUCCESS if unit/value pair was add, CAYENNE_FAILURE otherwise
	*/
	DLLExport int CayenneDataArrayAddFloat(CayenneDataArray* dataArray, const char* unit, float value);

	/**
	* Clear the data array.
	* @param[in] dataArray The data array to clear
	*/
	DLLExport void CayenneDataArrayClear(CayenneDataArray* dataArray);

#endif

#endif