/*
  Cayenne DHT Example
  This sketch shows how to send DHT Sensor data to the Cayenne Dashboard.
  The CayenneMQTT Library is required to run this sketch. If you have not already done so you can install it from the Arduino IDE Library Manager.

  Steps:
  1. Install DHT sensor library: https://github.com/adafruit/DHT-sensor-library.
  2. Install Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor.
  3. Attach a DHT to your Arduino:
   Schematic:
   DHT    Arduino
   [VDD] --- [5V]
   [GND] --- [GND]
   [OUTPUT] --- [Digital Pin 2]
  4. Set the TEMPERATURE_VIRTUAL_CHANNEL value below to a free virtual channel.
  5. Set the HUMIDITY_VIRTUAL_CHANNEL value below to a free virtual channel.
  6. Set the correct DHT type you are using and the pin attached to the Arduino.
  7. Set the Cayenne authentication info to match the authentication info from the Dashboard.
  8. Compile and upload this sketch.
  9. Once the Arduino connects to the Dashboard it should automatically create temporary display widget with data.
   To make a temporary widget permanent click the plus sign on the widget.
*/

#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial // Comment this out to disable prints and save space
#include <CayenneMQTTEthernet.h>
#include <DHT.h>

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "MQTT_USERNAME";
char password[] = "MQTT_PASSWORD";
char clientID[] = "CLIENT_ID";

#define DHTPIN 2     // Digital pin connected to the DHT sensor

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define TEMPERATURE_VIRTUAL_CHANNEL 1
#define HUMIDITY_VIRTUAL_CHANNEL 2

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor.

void setup() {
  Serial.begin(9600);
  Cayenne.begin(username, password, clientID);
  dht.begin();
}

void loop() {
  Cayenne.loop();
}

// This function is called at intervals to send temperature sensor data to Cayenne in Celsius.
CAYENNE_OUT(TEMPERATURE_VIRTUAL_CHANNEL)
{
  Cayenne.virtualWrite(TEMPERATURE_VIRTUAL_CHANNEL, dht.readTemperature(), "temp", "c");
}

// This function is called at intervals to send humidity sensor data to Cayenne.
CAYENNE_OUT(HUMIDITY_VIRTUAL_CHANNEL)
{
  Cayenne.virtualWrite(HUMIDITY_VIRTUAL_CHANNEL, dht.readHumidity(), "rel_hum", "p");
}
