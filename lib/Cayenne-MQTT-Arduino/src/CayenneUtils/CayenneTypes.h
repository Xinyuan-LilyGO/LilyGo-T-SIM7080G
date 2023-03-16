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

#ifndef _CAYENNETYPES_h
#define _CAYENNETYPES_h

// Data types
#define TYPE_BAROMETRIC_PRESSURE "bp" // Barometric pressure
#define TYPE_BATTERY "batt" // Battery
#define TYPE_LUMINOSITY "lum" // Luminosity
#define TYPE_PROXIMITY "prox" // Proximity
#define TYPE_RELATIVE_HUMIDITY "rel_hum" // Relative Humidity
#define TYPE_TEMPERATURE "temp" // Temperature
#define TYPE_VOLTAGE "voltage" // Voltage
#define TYPE_DIGITAL_SENSOR "digital_sensor" // Voltage

#define MAX_TYPE_LENGTH 14

// Unit types
#define UNIT_UNDEFINED "null"
#define UNIT_PASCAL "pa" // Pascal
#define UNIT_HECTOPASCAL "hpa" // Hectopascal
#define UNIT_PERCENT "p" // % (0 to 100)
#define UNIT_RATIO "r" // Ratio
#define UNIT_VOLTS "v" // Volts
#define UNIT_LUX "lux" // Lux
#define UNIT_MILLIMETER "mm" // Millimeter
#define UNIT_CENTIMETER "cm" // Centimeter
#define UNIT_METER "m" // Meter
#define UNIT_DIGITAL "d" // Digital (0/1)
#define UNIT_FAHRENHEIT "f" // Fahrenheit
#define UNIT_CELSIUS "c" // Celsius
#define UNIT_KELVIN "k" // Kelvin
#define UNIT_MILLIVOLTS "mv" // Millivolts

#define MAX_UNIT_LENGTH 4

#endif
