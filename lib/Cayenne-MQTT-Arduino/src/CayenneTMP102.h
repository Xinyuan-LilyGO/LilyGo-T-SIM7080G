/*
The MIT License(MIT)

Cayenne Arduino Client Library
Copyright © 2016 myDevices

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

#ifndef _CAYENNETMP102_h
#define _CAYENNETMP102_h

#include <Wire.h>
#include "CayenneTemperature.h"

class TMP102 : public Temperature
{
public:
	TMP102(int address) : _address(address) {
	}

	float getKelvin() {
		return celsiusToKelvin(getCelsius());
	}

	float getCelsius() {
		Wire.requestFrom(_address, 2);
		byte msb = Wire.read();
		byte lsb = Wire.read();
		int reading = ((msb << 8) | lsb) >> 4;
		return reading * 0.0625;
	}

	float getFahrenheit() {
		return celsiusToFahrenheit(getCelsius());
	}

private:
	int _address;
};

#endif