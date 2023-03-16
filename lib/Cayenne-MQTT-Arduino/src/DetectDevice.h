/**
 * @file       DetectDevice.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       May 2016
 * @brief
 *
 */

#ifndef DetectDevice_h
#define DetectDevice_h

#ifndef INFO_CPU

    /******************************************
     * ATmega
     */

    #if   defined(__AVR_ATmega168__)
    #define INFO_CPU      "ATmega168"
    #elif defined(__AVR_ATmega328P__)
    #define INFO_CPU      "ATmega328P"
    #elif defined(__AVR_ATmega1280__)
    #define INFO_CPU      "ATmega1280"
    #elif defined(__AVR_ATmega1284__)
    #define INFO_CPU      "ATmega1284"
    #elif defined(__AVR_ATmega2560__)
    #define INFO_CPU      "ATmega2560"
    #elif defined(__AVR_ATmega32U4__)
    #define INFO_CPU      "ATmega32U4"
    #elif defined(__SAM3X8E__)
    #define INFO_CPU      "AT91SAM3X8E"
    #elif defined(__SAMD21G18A__)
    #define INFO_CPU      "ATSAMD21G18"

    /******************************************
     * ATtiny
     */

    #elif defined(__AVR_ATtiny25__)
    #define INFO_CPU      "ATtiny25"
    #elif defined(__AVR_ATtiny45__)
    #define INFO_CPU      "ATtiny45"
    #elif defined(__AVR_ATtiny85__)
    #define INFO_CPU      "ATtiny85"
    #elif defined(__AVR_ATtiny24__)
    #define INFO_CPU      "ATtiny24"
    #elif defined(__AVR_ATtiny44__)
    #define INFO_CPU      "ATtiny44"
    #elif defined(__AVR_ATtiny84__)
    #define INFO_CPU      "ATtiny84"
    #elif defined(__AVR_ATtiny2313__)
    #define INFO_CPU      "ATtiny2313"
    #elif defined(__AVR_ATtiny4313__)
    #define INFO_CPU      "ATtiny4313"

    /******************************************
     * ESP8266
     */

    #elif defined(ESP8266)
    #define INFO_CPU      "Xtensa32"
    #elif defined(ESP32)
    #define INFO_CPU      "XtensaLX6"
    #endif
#endif

#ifndef INFO_DEVICE

    #if   defined(ENERGIA)
        #define INFO_DEVICE  "Energia"

        #if   defined(__MSP430F5529__)
        #define INFO_CPU  "MSP430F5529"
        #define NO_FLOAT
        #endif

    #elif defined(LINUX)

        #define INFO_DEVICE  "Linux"

    #elif defined(SPARK) || defined(PARTICLE)

        #if PLATFORM_ID==0
        #define INFO_DEVICE  "Particle Core"
        #elif PLATFORM_ID==6
        #define INFO_DEVICE  "Particle Photon"
        #elif PLATFORM_ID==8
        #define INFO_DEVICE  "Particle P1"
        #elif PLATFORM_ID==9
        #define INFO_DEVICE  "Particle Ethernet"
        #elif PLATFORM_ID==10
        #define INFO_DEVICE  "Particle Electron"
        #elif PLATFORM_ID==82
        #define INFO_DEVICE  "Digistump Oak"
        #elif PLATFORM_ID==88
        #define INFO_DEVICE  "RedBear Duo"
        #elif PLATFORM_ID==103
        #define INFO_DEVICE  "Bluz"
        #else
        #warning "Cannot detect board type"
        #define INFO_DEVICE  "Particle"
        #endif

    #elif defined(MBED_LIBRARY_VERSION)

        #define INFO_DEVICE  "MBED"

    #elif defined(ARDUINO) && defined(MPIDE)
        #define NO_YIELD

        #if   defined(_BOARD_UNO_)
        #define INFO_DEVICE  "chipKIT Uno32"
        #else
        #define INFO_DEVICE  "chipKIT"
        #endif

    #elif defined(ARDUINO)

        /* Arduino AVR */
        #if   defined(ARDUINO_AVR_NANO)
        #define INFO_DEVICE  "Arduino Nano"
        #elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_DUEMILANOVE)
        #define INFO_DEVICE  "Arduino Uno"
        #elif defined(ARDUINO_AVR_YUN)
        #define INFO_DEVICE  "Arduino Yun"
        #elif defined(ARDUINO_AVR_MINI)
        #define INFO_DEVICE  "Arduino Mini"
        #elif defined(ARDUINO_AVR_ETHERNET)
        #define INFO_DEVICE  "Arduino Ethernet"
        #elif defined(ARDUINO_AVR_FIO)
        #define INFO_DEVICE  "Arduino Fio"
        #elif defined(ARDUINO_AVR_BT)
        #define INFO_DEVICE  "Arduino BT"
        #elif defined(ARDUINO_AVR_PRO)
        #define INFO_DEVICE  "Arduino Pro"
        #elif defined(ARDUINO_AVR_NG)
        #define INFO_DEVICE  "Arduino NG"
        #elif defined(ARDUINO_AVR_GEMMA)
        #define INFO_DEVICE  "Arduino Gemma"
        #elif defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
        #define INFO_DEVICE  "Arduino Mega"
        #elif defined(ARDUINO_AVR_ADK)
        #define INFO_DEVICE  "Arduino Mega ADK"
        #elif defined(ARDUINO_AVR_LEONARDO)
        #define INFO_DEVICE  "Arduino Leonardo"
        #elif defined(ARDUINO_AVR_MICRO)
        #define INFO_DEVICE  "Arduino Micro"
        #elif defined(ARDUINO_AVR_ESPLORA)
        #define INFO_DEVICE  "Arduino Esplora"
        #elif defined(ARDUINO_AVR_LILYPAD)
        #define INFO_DEVICE  "Lilypad"
        #elif defined(ARDUINO_AVR_LILYPAD_USB)
        #define INFO_DEVICE  "Lilypad USB"
        #elif defined(ARDUINO_AVR_ROBOT_MOTOR)
        #define INFO_DEVICE  "Robot Motor"
        #elif defined(ARDUINO_AVR_ROBOT_CONTROL)
        #define INFO_DEVICE  "Robot Control"

        /* Arduino SAM/SAMD */
        #elif defined(ARDUINO_SAM_DUE)
        #define INFO_DEVICE  "Arduino Due"
        #elif defined(ARDUINO_SAMD_ZERO)
        #define INFO_DEVICE  "Arduino Zero"
        #elif defined(ARDUINO_SAMD_MKR1000)
        #define INFO_DEVICE  "Arduino MKR1000"

        /* Intel */
        #elif defined(ARDUINO_GALILEO)
        #define INFO_DEVICE  "Galileo"
        #elif defined(ARDUINO_GALILEOGEN2)
        #define INFO_DEVICE  "Galileo Gen2"
        #elif defined(ARDUINO_EDISON)
        #define INFO_DEVICE  "Edison"
        #elif defined(ARDUINO_ARCH_ARC32) // TODO
        #define INFO_DEVICE  "Arduino 101"

        /* Konekt */
        #elif defined(ARDUINO_DASH)
        #define INFO_DEVICE  "Dash"
        #elif defined(ARDUINO_DASHPRO)
        #define INFO_DEVICE  "Dash Pro"

        /* Red Bear Lab */
        #elif defined(ARDUINO_RedBear_Duo)
        #define INFO_DEVICE  "RedBear Duo"
        #elif defined(ARDUINO_BLEND)
        #define INFO_DEVICE  "Blend"
        #elif defined(ARDUINO_BLEND_MICRO_8MHZ) || defined(ARDUINO_BLEND_MICRO_16MHZ)
        #define INFO_DEVICE  "Blend Micro"
        #elif defined(ARDUINO_RBL_nRF51822)
        #define INFO_DEVICE  "BLE Nano"

        /* ESP8266 */
        #elif defined(ARDUINO_ESP8266_ESP01)
        #define INFO_DEVICE  "ESP8266"
        #elif defined(ARDUINO_ESP8266_ESP12)
        #define INFO_DEVICE  "ESP-12"
        #elif defined(ARDUINO_ESP8266_NODEMCU)
        #define INFO_DEVICE  "NodeMCU"
        #elif defined(ARDUINO_ESP8266_THING)
        #define INFO_DEVICE  "Esp Thing"
        #elif defined(ARDUINO_ESP8266_THING_DEV)
        #define INFO_DEVICE  "Esp Thing Dev"

        /* ESP32 */
        #elif defined(ARDUINO_ARCH_ESP32)
        #define INFO_DEVICE  "ESP32"

        /* Digistump */
        #elif defined(ARDUINO_ESP8266_OAK)
        #define INFO_DEVICE  "Oak"
        #elif defined(ARDUINO_AVR_DIGISPARK)
        #define INFO_DEVICE  "Digispark"
        #elif defined(ARDUINO_AVR_DIGISPARKPRO)
        #define INFO_DEVICE  "Digispark Pro"

        /* Microduino */
        #elif defined(ARDUINO_AVR_USB)
        #define INFO_DEVICE  "CoreUSB"
        #elif defined(ARDUINO_AVR_PLUS)
        #define INFO_DEVICE  "Core+"
        #elif defined(ARDUINO_AVR_RF)
        #define INFO_DEVICE  "CoreRF"

        /* Wildfire */
        #elif defined(ARDUINO_WILDFIRE_V2)
        #define INFO_DEVICE  "Wildfire V2"
        #elif defined(ARDUINO_WILDFIRE_V3)
        #define INFO_DEVICE  "Wildfire V3"
        #elif defined(ARDUINO_WILDFIRE_V4)
        #define INFO_DEVICE  "Wildfire V4"

        /* Simblee */
        #elif defined(__Simblee__) // TODO: ARDUINO_SIMBLEE bug?
        #define INFO_DEVICE  "Simblee"

        #else
        #warning "Cannot detect board type"
        #define INFO_DEVICE  "Arduino"
        #endif

	#endif
#endif

#endif
