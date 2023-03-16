/*
  Cayenne HC-SR04 Example
  This sketch shows how to send HC-SR04 Sensor data to the Cayenne Dashboard.
  The CayenneMQTT Library is required to run this sketch. If you have not already done so you can install it from the Arduino IDE Library Manager.
  Steps:
  1. Attach a HC-SR04 to your Arduino:
   Schematic:
   HC-SR04    Arduino
   [VCC] --- [5V]
   [GND] --- [GND]
   [TRIGGER_PIN] --- [Digital Pin 2]
   [ECHO_PIN] --- [Digital Pin 3]
  2. Set the DISTANCE_VIRTUAL_CHANNEL value below to a free virtual channel.
  3. Set the Cayenne authentication info to match the authentication info from the Dashboard.
  4. Compile and upload this sketch.
  5. Once the Arduino connects to the Dashboard it should automatically create temporary display widget with data.
   To make a temporary widget permanent click the plus sign on the widget.
*/

#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial // Comment this out to disable prints and save space
#include <CayenneMQTTEthernet.h>

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "MQTT_USERNAME";
char password[] = "MQTT_PASSWORD";
char clientID[] = "CLIENT_ID";

#define DISTANCE_VIRTUAL_CHANNEL 1

// Defines pins numbers for the HC-SR04 connections.
#define TRIGGER_PIN  2
#define ECHO_PIN 3

// Defines variables for storing the calculated values.
long duration;
int distance;

void setup() {
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(9600);
  Cayenne.begin(username, password, clientID);
}

void loop() {
  Cayenne.loop();
}

// This function is called at intervals to send HC-SR04 sensor data to Cayenne.
CAYENNE_OUT(DISTANCE_VIRTUAL_CHANNEL)
{
  digitalWrite(TRIGGER_PIN, LOW); // Clears the trigger pin
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH); // Sets the trigger pin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH); // Reads the echo pin, returns the sound wave travel time in microseconds

  distance = duration * 0.034 / 2; // Calculating the distance.

  // Send the distance value to Cayenne on proximity widget in centimeter.
  Cayenne.virtualWrite(DISTANCE_VIRTUAL_CHANNEL, distance, "prox", "cm");
}
