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

#ifndef _CAYENNETOPICS_h
#define _CAYENNETOPICS_h

typedef enum CayenneTopic
{
	UNDEFINED_TOPIC,
	DATA_TOPIC,
	COMMAND_TOPIC,
	RESPONSE_TOPIC,
	SYS_MODEL_TOPIC,
	SYS_VERSION_TOPIC,
	SYS_CPU_MODEL_TOPIC,
	SYS_CPU_SPEED_TOPIC,
	SYS_CONNECTION_TOPIC,
#ifdef DIGITAL_AND_ANALOG_SUPPORT
	DIGITAL_TOPIC,
	DIGITAL_COMMAND_TOPIC,
	DIGITAL_CONFIG_TOPIC,
	ANALOG_TOPIC, 
	ANALOG_COMMAND_TOPIC,
	ANALOG_CONFIG_TOPIC,
#endif
} CayenneTopic;

#define DATA_STRING CAYENNE_PSTR("data")
#define COMMAND_STRING CAYENNE_PSTR("cmd")
#define RESPONSE_STRING CAYENNE_PSTR("response")
#define SYS_MODEL_STRING CAYENNE_PSTR("sys/model")
#define SYS_VERSION_STRING CAYENNE_PSTR("sys/version")
#define SYS_CPU_MODEL_STRING CAYENNE_PSTR("sys/cpu/model")
#define SYS_CPU_SPEED_STRING CAYENNE_PSTR("sys/cpu/speed")
#define SYS_CONNECTION_STRING CAYENNE_PSTR("sys/conn")
#ifdef DIGITAL_AND_ANALOG_SUPPORT
	#define DIGITAL_STRING CAYENNE_PSTR("digital")
	#define DIGITAL_COMMAND_STRING CAYENNE_PSTR("digital-cmd")
	#define DIGITAL_CONFIG_STRING CAYENNE_PSTR("digital-conf")
	#define ANALOG_STRING CAYENNE_PSTR("analog")
	#define ANALOG_COMMAND_STRING CAYENNE_PSTR("analog-cmd")
	#define ANALOG_CONFIG_STRING CAYENNE_PSTR("analog-conf")
#endif 

#define CAYENNE_ALL_CHANNELS 0xFFFF
#define CAYENNE_NO_CHANNEL 0xFFFE

#endif