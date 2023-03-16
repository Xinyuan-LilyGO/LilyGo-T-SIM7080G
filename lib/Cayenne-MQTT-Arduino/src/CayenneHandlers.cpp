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

Code adapted from Blynk library BlynkHandlers.cpp. Copyright info below.
 * @file       BlynkHandlers.cpp
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Virtual channel utilities
 */

#include "CayenneArduinoDefines.h"

void InputHandler(Request& request, CayenneMessage& getValue)
{
	CAYENNE_LOG("No handler for writing to channel %d", request.channel);
}

void OutputHandler(Request& request)
{
	CAYENNE_LOG("No handler for reading from channel %d", request.channel);
}

void EmptyHandler()
{}

#define CAYENNE_IN_IMPL(channel) void InputHandler ## channel (Request& req, CayenneMessage& getValue) \
          __attribute__((weak, alias("InputHandler")))

#define CAYENNE_OUT_IMPL(channel)  void OutputHandler ## channel (Request& req) \
          __attribute__((weak, alias("OutputHandler")))


CAYENNE_CONNECTED() __attribute__((weak, alias("EmptyHandler")));
CAYENNE_DISCONNECTED() __attribute__((weak, alias("EmptyHandler")));

CAYENNE_IN_IMPL(Default);
CAYENNE_OUT_DEFAULT() __attribute__((weak, alias("EmptyHandler")));;

CAYENNE_IN_IMPL(0);
CAYENNE_IN_IMPL(1);
CAYENNE_IN_IMPL(2);
CAYENNE_IN_IMPL(3);
CAYENNE_IN_IMPL(4);
CAYENNE_IN_IMPL(5);
CAYENNE_IN_IMPL(6);
CAYENNE_IN_IMPL(7);
CAYENNE_IN_IMPL(8);
CAYENNE_IN_IMPL(9);
CAYENNE_IN_IMPL(10);
CAYENNE_IN_IMPL(11);
CAYENNE_IN_IMPL(12);
CAYENNE_IN_IMPL(13);
CAYENNE_IN_IMPL(14);
CAYENNE_IN_IMPL(15);
CAYENNE_IN_IMPL(16);
CAYENNE_IN_IMPL(17);
CAYENNE_IN_IMPL(18);
CAYENNE_IN_IMPL(19);
CAYENNE_IN_IMPL(20);
CAYENNE_IN_IMPL(21);
CAYENNE_IN_IMPL(22);
CAYENNE_IN_IMPL(23);
CAYENNE_IN_IMPL(24);
CAYENNE_IN_IMPL(25);
CAYENNE_IN_IMPL(26);
CAYENNE_IN_IMPL(27);
CAYENNE_IN_IMPL(28);
CAYENNE_IN_IMPL(29);
CAYENNE_IN_IMPL(30);
CAYENNE_IN_IMPL(31);

CAYENNE_OUT_IMPL(0);
CAYENNE_OUT_IMPL(1);
CAYENNE_OUT_IMPL(2);
CAYENNE_OUT_IMPL(3);
CAYENNE_OUT_IMPL(4);
CAYENNE_OUT_IMPL(5);
CAYENNE_OUT_IMPL(6);
CAYENNE_OUT_IMPL(7);
CAYENNE_OUT_IMPL(8);
CAYENNE_OUT_IMPL(9);
CAYENNE_OUT_IMPL(10);
CAYENNE_OUT_IMPL(11);
CAYENNE_OUT_IMPL(12);
CAYENNE_OUT_IMPL(13);
CAYENNE_OUT_IMPL(14);
CAYENNE_OUT_IMPL(15);
CAYENNE_OUT_IMPL(16);
CAYENNE_OUT_IMPL(17);
CAYENNE_OUT_IMPL(18);
CAYENNE_OUT_IMPL(19);
CAYENNE_OUT_IMPL(20);
CAYENNE_OUT_IMPL(21);
CAYENNE_OUT_IMPL(22);
CAYENNE_OUT_IMPL(23);
CAYENNE_OUT_IMPL(24);
CAYENNE_OUT_IMPL(25);
CAYENNE_OUT_IMPL(26);
CAYENNE_OUT_IMPL(27);
CAYENNE_OUT_IMPL(28);
CAYENNE_OUT_IMPL(29);
CAYENNE_OUT_IMPL(30);
CAYENNE_OUT_IMPL(31);

static const InputHandlerFunction InputHandlerVector[MAX_NUM_OF_CHANNELS] CAYENNE_PROGMEM = {
	InputHandler0,  InputHandler1,  InputHandler2,  InputHandler3,
	InputHandler4,  InputHandler5,  InputHandler6,  InputHandler7,
	InputHandler8,  InputHandler9,  InputHandler10, InputHandler11,
	InputHandler12, InputHandler13, InputHandler14, InputHandler15,
	InputHandler16, InputHandler17, InputHandler18, InputHandler19,
	InputHandler20, InputHandler21, InputHandler22, InputHandler23,
	InputHandler24, InputHandler25, InputHandler26, InputHandler27,
	InputHandler28, InputHandler29, InputHandler30, InputHandler31,
};

static const OutputHandlerFunction OutputHandlerVector[MAX_NUM_OF_CHANNELS] CAYENNE_PROGMEM = {
	OutputHandler0,   OutputHandler1,   OutputHandler2,   OutputHandler3,
	OutputHandler4,   OutputHandler5,   OutputHandler6,   OutputHandler7,
	OutputHandler8,   OutputHandler9,   OutputHandler10,  OutputHandler11,
	OutputHandler12,  OutputHandler13,  OutputHandler14,  OutputHandler15,
	OutputHandler16,  OutputHandler17,  OutputHandler18,  OutputHandler19,
	OutputHandler20,  OutputHandler21,  OutputHandler22,  OutputHandler23,
	OutputHandler24,  OutputHandler25,  OutputHandler26,  OutputHandler27,
	OutputHandler28,  OutputHandler29,  OutputHandler30,  OutputHandler31,
};


InputHandlerFunction GetInputHandler(uint8_t channel)
{
	if (channel >= COUNT_OF(InputHandlerVector))
		return NULL;
#ifdef CAYENNE_USING_PROGMEM
	return (InputHandlerFunction)pgm_read_word(&InputHandlerVector[channel]);
#else
	return InputHandlerVector[channel];
#endif
}

OutputHandlerFunction GetOutputHandler(uint8_t channel)
{
	if (channel >= COUNT_OF(OutputHandlerVector))
		return NULL;
#ifdef CAYENNE_USING_PROGMEM
	return (OutputHandlerFunction)pgm_read_word(&OutputHandlerVector[channel]);
#else
	return OutputHandlerVector[channel];
#endif
}