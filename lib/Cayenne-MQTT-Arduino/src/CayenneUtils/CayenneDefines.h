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

#ifndef _CAYENNEDEFINES_h
#define _CAYENNEDEFINES_h

#define CAYENNE_LIBRARY_VERSION "1.3.0"
#define CAYENNE_VERSION "v1"
#define CAYENNE_DOMAIN "mqtt.mydevices.com"
#define CAYENNE_PORT 1883
#define CAYENNE_TLS_PORT 8883

#ifndef CAYENNE_MAX_MESSAGE_SIZE
#define CAYENNE_MAX_MESSAGE_SIZE 134 // Redefine this for different message size
#endif

#ifndef CAYENNE_MAX_PAYLOAD_SIZE
#define CAYENNE_MAX_PAYLOAD_SIZE 64 // Redefine this for different payload size
#endif

#ifndef CAYENNE_MAX_MESSAGE_HANDLERS
#define CAYENNE_MAX_MESSAGE_HANDLERS 5 /* Redefine to change number of handlers */
#endif

#ifndef CAYENNE_MAX_MESSAGE_VALUES
#define CAYENNE_MAX_MESSAGE_VALUES 4 /* Redefine to change max number of values in a message, must be at least 1 */
#endif

//Comment this out to prevent digital and analog specific code from being compiled. If you only need to send
//and receive standard channel data you can comment this out to decrease the program size.
//#define DIGITAL_AND_ANALOG_SUPPORT

//Comment this out if you don't need to subscribe to data or system info payloads.
//#define PARSE_INFO_PAYLOADS

//Some defines for AVR microcontrollers to allow easier usage of memory in program space.
#if defined(__AVR__) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
#include <avr/pgmspace.h>
#define CAYENNE_USING_PROGMEM
#define CAYENNE_PROGMEM PROGMEM
#define CAYENNE_PSTR(s) PSTR(s)
#define CAYENNE_STRLEN(s) strlen_P(s)
#define CAYENNE_STRCAT(s1, s2) strcat_P(s1, s2)
#define CAYENNE_STRNCMP(s1, s2, n) strncmp_P(s1, s2, n)
#define CAYENNE_MEMCPY(s1, s2, n) memcpy_P(s1, s2, n)
#define CAYENNE_READ_BYTE(b) pgm_read_byte(b);
#define CAYENNE_FLASH(s) F(s)
#else
#define CAYENNE_PROGMEM
#define CAYENNE_PSTR(s) s
#define CAYENNE_STRLEN(s) strlen(s)
#define CAYENNE_STRCAT(s1, s2) strcat(s1, s2)
#define CAYENNE_STRNCMP(s1, s2, n) strncmp(s1, s2, n)
#define CAYENNE_MEMCPY(s1, s2, n) memcpy(s1, s2, n)
#define CAYENNE_READ_BYTE(b) *b;
#define CAYENNE_FLASH(s) s
#endif

#include "CayenneTypes.h"
#include "CayenneTopics.h"

#endif
