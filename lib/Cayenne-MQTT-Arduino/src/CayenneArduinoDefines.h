/*
The MIT License(MIT)

Cayenne Arduino Client Library
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

Logging code adapted from Blynk library BlynkDebug.h. Copyright info below.
* @file       BlynkDebug.h
* @author     Volodymyr Shymanskyy
* @license    This project is released under the MIT License (MIT)
* @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
* @date       Jan 2015
* @brief      Debug utilities
*/

#ifndef _CAYENNEARDUINODEFINES_h
#define _CAYENNEARDUINODEFINES_h

#include "CayenneUtils/CayenneDefines.h"

#define ERROR_INCORRECT_PARAM "Bad param"

#define MAX_NUM_OF_CHANNELS 32
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#if defined(ARDUINO)
	#if ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
	#endif
#endif

#include <stdio.h>
#include <stdarg.h>

#ifdef CAYENNE_PRINT
	static void log(const char* CAYENNE_PROGMEM message, ...)
	{
		va_list args;
		va_start(args, message);
		char buffer[12];
		CAYENNE_PRINT.print('[');
		CAYENNE_PRINT.print(millis());
		CAYENNE_PRINT.print(CAYENNE_FLASH("] "));
		const char* p = message;
		size_t n = 0;
		while (1) {
			char c = CAYENNE_READ_BYTE(p++);
			if (c == '%') {
				buffer[n] = 0;
				CAYENNE_PRINT.print(buffer);
				n = 0;
				char next = CAYENNE_READ_BYTE(p++);
				switch (next) {
				case 's':
					CAYENNE_PRINT.print(va_arg(args, char*));
					break;
				case 'd':
					CAYENNE_PRINT.print(va_arg(args, int));
					break;
				case 'u':
					CAYENNE_PRINT.print(va_arg(args, unsigned int));
					break;
				case 'f':
					CAYENNE_PRINT.print(va_arg(args, double));
					break;
				case 'l':
					next = CAYENNE_READ_BYTE(p++);
					switch (next) {
					case 'x':
					case 'X':
						CAYENNE_PRINT.print(va_arg(args, long), 16);
						break;
					default:
						CAYENNE_PRINT.print(va_arg(args, long));
						break;
					}
					break;
				case 'x':
				case 'X':
					CAYENNE_PRINT.print(va_arg(args, int), 16);
					break;
				default:
					CAYENNE_PRINT.print(next);
					break;
				}
			}
			else {
				buffer[n++] = c;
				if (n >= sizeof(buffer) - 1)
				{
					buffer[n] = 0;
					CAYENNE_PRINT.print(buffer);
					n = 0;
					buffer[0] = 0;
				}
			}
			if (c == 0) {
				break;
			}
		}
		CAYENNE_PRINT.println(buffer);
		va_end(args);
	}
	#define CAYENNE_LOG(message, ...) log(CAYENNE_PSTR(message), ##__VA_ARGS__)
#else
	#define CAYENNE_LOG(message, ...)
#endif

#ifdef CAYENNE_DEBUG
	#define CAYENNE_LOG_DEBUG(message, ...) CAYENNE_LOG((message), ##__VA_ARGS__)
#else
	#define CAYENNE_LOG_DEBUG(message, ...)
#endif

#include "DetectDevice.h"
#include "CayenneHandlers.h"

#endif
