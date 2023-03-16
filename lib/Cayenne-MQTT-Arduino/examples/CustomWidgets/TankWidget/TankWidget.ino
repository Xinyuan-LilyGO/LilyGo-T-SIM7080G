/*
Cayenne Tank Widget Example.

The CayenneMQTT Library is required to run this sketch. If you have not already done so you can install it from the Arduino IDE Library Manager.

Steps:
1. Attach an analog input device (e.g. a temperature sensor) to your Arduino.
2. Set the VIRTUAL_CHANNEL value below to the virtual channel of a Tank Custom Widget you have added in the Dashboard.
3. Update the CAYENNE_OUT function below to send the data from your sensor.
4. Set the Cayenne authentication info to match the authentication info from the Dashboard.
5. Compile and upload this sketch.
*/

//#define CAYENNE_DEBUG       // Uncomment to show debug messages
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#include <CayenneMQTTEthernet.h>

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "MQTT_USERNAME";
char password[] = "MQTT_PASSWORD";
char clientID[] = "CLIENT_ID";

void setup() {
	Serial.begin(9600);
	Cayenne.begin(username, password, clientID);
}

void loop() {
	Cayenne.loop();
}

// Default function for sending sensor data at intervals to Cayenne.
CAYENNE_OUT(1)
{
	// Write data to Cayenne here. 
	int val = analogRead(0);
	val = map(val, 0, 1023, 0, 100);  
	Cayenne.virtualWrite(1, val, "tl", "null");
}
