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

This file contains code modified from the thermistor code at http://playground.arduino.cc/ComponentLib/Thermistor2
*/

#ifndef _CAYENNETEMPERATURE_h
#define _CAYENNETEMPERATURE_h

#include <Arduino.h>
#include <math.h>

class Temperature
{
public:
	virtual float getKelvin() = 0;

	virtual float getCelsius() = 0;

	virtual float getFahrenheit() = 0;

	float celsiusToKelvin(float celsius) {
		return celsius + 273.15;
	}

	float kelvinToCelsius(float kelvin) {
		return kelvin - 273.15;
	}

	float celsiusToFahrenheit(float celsius) {
		return (celsius * 9.0) / 5.0 + 32.0;
	}
};

class Thermistor : public Temperature
{
public:
	Thermistor(int pin, float resistance) :	_pin(pin), _resistance(resistance) {
	}

	float getKelvin() {
		int reading = analogRead(_pin);
		if (reading != 0) {
			long resistance = _resistance * ((1024.0 / reading) - 1);
			float temp = log(resistance);
			return 1 / (0.001129148 + (0.000234125 * temp) + (0.0000000876741 * temp * temp * temp));
		}
		return 0;
	}

	float getCelsius() {
		return kelvinToCelsius(getKelvin());
	}

	float getFahrenheit() {
		return celsiusToFahrenheit(getCelsius());
	}

private:
	int _pin;
	float _resistance;
};

class TMP36 : public Temperature
{
public:
	TMP36(int pin, float voltage) : _pin(pin), _voltage(voltage) {
	}

	float getKelvin() {
		return celsiusToKelvin(getCelsius());
	}

	float getCelsius() {
		int reading = analogRead(_pin);
		float currentVoltage = (reading * _voltage) / 1024.0;
		return (currentVoltage - 0.5) * 100;
	}

	float getFahrenheit() {
		return celsiusToFahrenheit(getCelsius());
	}

private:
	int _pin;
	float _voltage;
};

#endif