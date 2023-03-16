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

Code adapted from Blynk library BlynkHandlers.h. Copyright info below.
* @file       BlynkHandlers.h
* @author     Volodymyr Shymanskyy
* @license    This project is released under the MIT License (MIT)
* @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
* @date       Jan 2015
* @brief      Handlers for virtual channel operations
*/

#ifndef _CAYENNEHANDLERS_h
#define _CAYENNEHANDLERS_h

#include "CayenneMessage.h"

// Helper macros
#define V0  0
#define V1  1
#define V2  2
#define V3  3
#define V4  4
#define V5  5
#define V6  6
#define V7  7
#define V8  8
#define V9  9
#define V10 10
#define V11 11
#define V12 12
#define V13 13
#define V14 14
#define V15 15
#define V16 16
#define V17 17
#define V18 18
#define V19 19
#define V20 20
#define V21 21
#define V22 22
#define V23 23
#define V24 24
#define V25 25
#define V26 26
#define V27 27
#define V28 28
#define V29 29
#define V30 30
#define V31 31

// Input & Output handlers
// Two IN & OUT handler macros are used to allow processing the channel value correctly if a macro is used
// for the channel when declaring the handler, e.g. CAYENNE_IN(V1).
#define CAYENNE_IN_2(channel) void InputHandler ## channel (Request& request, CayenneMessage& getValue)
#define CAYENNE_OUT_2(channel) void OutputHandler ## channel  (Request& request)
#define CAYENNE_IN(channel) CAYENNE_IN_2(channel)
#define CAYENNE_OUT(channel) CAYENNE_OUT_2(channel)

#define CAYENNE_IN_DEFAULT()   CAYENNE_IN(Default)
#define CAYENNE_OUT_DEFAULT()  void CayenneOutDefault()

// Additional handlers
#define CAYENNE_CONNECTED()    void CayenneConnected()
#define CAYENNE_DISCONNECTED() void CayenneDisconnected()


// Default read/write handlers (you can redefine them in your code)
#ifdef __cplusplus
extern "C" {
#endif

struct Request
{
	unsigned int channel;
};

typedef void (*InputHandlerFunction)(Request& request, CayenneMessage& getValue);
typedef void(*OutputHandlerFunction)(Request& request);

InputHandlerFunction GetInputHandler(uint8_t channel);
OutputHandlerFunction GetOutputHandler(uint8_t channel);

// Declare placeholders
CAYENNE_OUT();
CAYENNE_IN();
void EmptyHandler();

// Declare all channel handlers (you can redefine them in your code)
CAYENNE_CONNECTED();
CAYENNE_DISCONNECTED();

CAYENNE_IN_DEFAULT();
CAYENNE_OUT_DEFAULT();

CAYENNE_IN(0);
CAYENNE_IN(1);
CAYENNE_IN(2);
CAYENNE_IN(3);
CAYENNE_IN(4);
CAYENNE_IN(5);
CAYENNE_IN(6);
CAYENNE_IN(7);
CAYENNE_IN(8);
CAYENNE_IN(9);
CAYENNE_IN(10);
CAYENNE_IN(11);
CAYENNE_IN(12);
CAYENNE_IN(13);
CAYENNE_IN(14);
CAYENNE_IN(15);
CAYENNE_IN(16);
CAYENNE_IN(17);
CAYENNE_IN(18);
CAYENNE_IN(19);
CAYENNE_IN(20);
CAYENNE_IN(21);
CAYENNE_IN(22);
CAYENNE_IN(23);
CAYENNE_IN(24);
CAYENNE_IN(25);
CAYENNE_IN(26);
CAYENNE_IN(27);
CAYENNE_IN(28);
CAYENNE_IN(29);
CAYENNE_IN(30);
CAYENNE_IN(31); 

CAYENNE_OUT(0 );
CAYENNE_OUT(1 );
CAYENNE_OUT(2 );
CAYENNE_OUT(3 );
CAYENNE_OUT(4 );
CAYENNE_OUT(5 );
CAYENNE_OUT(6 );
CAYENNE_OUT(7 );
CAYENNE_OUT(8 );
CAYENNE_OUT(9 );
CAYENNE_OUT(10);
CAYENNE_OUT(11);
CAYENNE_OUT(12);
CAYENNE_OUT(13);
CAYENNE_OUT(14);
CAYENNE_OUT(15);
CAYENNE_OUT(16);
CAYENNE_OUT(17);
CAYENNE_OUT(18);
CAYENNE_OUT(19);
CAYENNE_OUT(20);
CAYENNE_OUT(21);
CAYENNE_OUT(22);
CAYENNE_OUT(23);
CAYENNE_OUT(24);
CAYENNE_OUT(25);
CAYENNE_OUT(26);
CAYENNE_OUT(27);
CAYENNE_OUT(28);
CAYENNE_OUT(29);
CAYENNE_OUT(30);
CAYENNE_OUT(31);

#ifdef __cplusplus
}
#endif

#endif
