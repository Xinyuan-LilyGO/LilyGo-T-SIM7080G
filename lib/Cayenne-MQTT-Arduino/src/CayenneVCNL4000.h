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

This class adapted from Adafruit VCNL400 example sketch https://github.com/adafruit/VCNL4000/blob/master/vcnl4000.pde
*/

#ifndef _CAYENNEVCNL4000_h
#define _CAYENNEVCNL4000_h

#include <Wire.h>
#include <Arduino.h>

// commands and constants
#define VCNL4000_ADDRESS 0x13
#define VCNL4000_COMMAND 0x80
#define VCNL4000_PRODUCTID 0x81
#define VCNL4000_IRLED 0x83
#define VCNL4000_AMBIENTPARAMETER 0x84
#define VCNL4000_AMBIENTDATA 0x85
#define VCNL4000_PROXIMITYDATA 0x87
#define VCNL4000_SIGNALFREQ 0x89
#define VCNL4000_PROXINITYADJUST 0x8A

#define VCNL4000_3M125 0
#define VCNL4000_1M5625 1
#define VCNL4000_781K25 2
#define VCNL4000_390K625 3

#define VCNL4000_MEASUREAMBIENT 0x10
#define VCNL4000_MEASUREPROXIMITY 0x08
#define VCNL4000_AMBIENTREADY 0x40
#define VCNL4000_PROXIMITYREADY 0x20

#define CALIBRATION_CYCLES 5
#define MEASUREMENT_CYCLES 10
#define THRESHOLD 25
#define NO_PROXIMITY -1
#define TIMEOUT 2000

class VCNL4000
{
public:
	bool begin() {
		Wire.begin();
		uint8_t rev = read8(VCNL4000_PRODUCTID);
		if ((rev & 0xF0) != 0x10) {
			return false;
		}
		write8(VCNL4000_IRLED, 20); // Set IR current
		write8(VCNL4000_SIGNALFREQ, VCNL4000_781K25); // Set proximity signal frequency
		write8(VCNL4000_PROXINITYADJUST, 0x81); // Set proximity timing
		calibrate();
		return true;
	}

	float getLux() {
		return (readAmbient() + 3) * 0.25;
	}

	int getMillimeters() {
		unsigned int success = 0;
		unsigned int fail = 0;
		unsigned int proximitySum = 0;
		while ((fail < MEASUREMENT_CYCLES) && (success < MEASUREMENT_CYCLES)) {
			uint16_t realCount = readProximity() - _offset;
			if (realCount > _threshold) {
				success++;
				proximitySum += realCount;
			}
			else {
				fail++;
			}
		}
		if (fail == MEASUREMENT_CYCLES) {
			return NO_PROXIMITY;
		}
		else {
			return calculateMillimeters(proximitySum / MEASUREMENT_CYCLES);
		}
	}

	uint16_t readProximity() {
		write8(VCNL4000_COMMAND, VCNL4000_MEASUREPROXIMITY);
		while (1) {
			uint8_t result = read8(VCNL4000_COMMAND);
			if (result & VCNL4000_PROXIMITYREADY) {
				return read16(VCNL4000_PROXIMITYDATA);
			}
			delay(1);
		}
	}

	uint16_t readAmbient() {
		write8(VCNL4000_COMMAND, VCNL4000_MEASUREAMBIENT);
		while (1) {
			uint8_t result = read8(VCNL4000_COMMAND);
			if (result & VCNL4000_AMBIENTREADY) {
				return read16(VCNL4000_AMBIENTDATA);
			}
			delay(1);
		}
	}

private:
	void calibrate() {
		for (int i = 0; i < CALIBRATION_CYCLES; ++i) {
			_offset += readProximity();
		}
		_offset = _offset / CALIBRATION_CYCLES;
	}

	uint16_t calculateMillimeters(uint16_t proximityCounts) {
		//According to chip spec the proximity counts are strong non-linear with distance and cannot be calculated
		//with a direct formula. From experience found on web this chip is generally not suited for really exact
		//distance calculations. This is a rough distance estimation lookup table for now. Maybe someone can
		//provide a more exact approximation in the future.
		unsigned int estimatedDistance = 100;
		if (proximityCounts >= 10000) {
			estimatedDistance = 0;
		}
		else if (proximityCounts >= 3000) {
			estimatedDistance = 5;
		}
		else if (proximityCounts >= 900) {
			estimatedDistance = 10;
		}
		else if (proximityCounts >= 300) {
			estimatedDistance = 20;
		}
		else if (proximityCounts >= 150) {
			estimatedDistance = 30;
		}
		else if (proximityCounts >= 75) {
			estimatedDistance = 40;
		}
		else if (proximityCounts >= 50) {
			estimatedDistance = 50;
		}
		else if (proximityCounts >= 25) {
			estimatedDistance = 70;
		}
		return estimatedDistance;
	}

	// Read 1 byte from the VCNL4000 at 'address'
	uint8_t read8(uint8_t address)
	{
		uint8_t data;

		Wire.beginTransmission(VCNL4000_ADDRESS);
		Wire.write(address);
		Wire.endTransmission();

		delayMicroseconds(170);  // delay required

		Wire.requestFrom(VCNL4000_ADDRESS, 1);
		return Wire.read();
	}


	// Read 2 byte from the VCNL4000 at 'address'
	uint16_t read16(uint8_t address)
	{
		uint16_t data;

		Wire.beginTransmission(VCNL4000_ADDRESS);
		Wire.write(address);
		Wire.endTransmission();

		Wire.requestFrom(VCNL4000_ADDRESS, 2);
		data = Wire.read();
		data <<= 8;
		data |= Wire.read();

		return data;
	}

	// write 1 byte
	void write8(uint8_t address, uint8_t data)
	{
		Wire.beginTransmission(VCNL4000_ADDRESS);
		Wire.write(address);
		Wire.write(data);
		Wire.endTransmission();
	}

	uint16_t _offset;
	uint16_t _threshold;
};

#endif
