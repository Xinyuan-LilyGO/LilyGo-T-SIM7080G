/*
  This example is to be used when using Cayenne triggers. 
  The CayenneMQTT Library is required to run this sketch. If you have not already done so you can install it from the Arduino IDE Library Manager.
  Steps:
  1. Set the trigger and threshold values in the sketch below.
  2. Set the Cayenne authentication info to match the authentication info from the Dashboard.
  3. Compile and upload the sketch.
  4. Two temporary widget (DATA_CHANNEL and TRIGGER_CHANNEL) will be automatically generated in the Cayenne Dashboard. To make the widget permanent click the plus sign on the widget.
  5. Add trigger on your cayenne dashboard for the TRIGGER_CHANNEL widget when it becomes 1.
*/

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTEthernet.h> // Change this to use a different communication device. See Communications examples.

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "MQTT_USERNAME";
char password[] = "MQTT_PASSWORD";
char clientID[] = "CLIENT_ID";

#define DATA_CHANNEL 0 //Virtual channel for publishing sensor data.
#define TRIGGER_CHANNEL 1 //Virtual channel for publishing the trigger value.
#define THRESHOLD 200 //Threshold for the trigger.
bool sendBelowThreshold = true; //Set to true if the trigger should happen when the data value is below the threshold, 
                                //false if it should happen when the data value is above or equal to the threshold.
bool crossedThreshold = false;

void setup() {
  Serial.begin(9600);
  Cayenne.begin(username, password, clientID);
}

void sendTriggerValue(int channel, int value, int threshold, bool sendBelowThreshold) {
  if (((value >= threshold) && !sendBelowThreshold) || ((value < threshold) && sendBelowThreshold)) {
    if (!crossedThreshold) {
      Cayenne.virtualWrite(channel, 1, "digital_sensor", "d"); //set trigger two-state widget to 1 
      crossedThreshold = true;
    }
  }
  else
  {
    Cayenne.virtualWrite(channel, 0, "digital_sensor", "d"); //set trigger two-state widget to 0
    crossedThreshold = false;
  }
}

void loop() {
  Cayenne.loop();
}

CAYENNE_OUT_DEFAULT()
{
  int sensor_value = analogRead(A0); 
  sendTriggerValue(TRIGGER_CHANNEL, sensor_value, THRESHOLD, sendBelowThreshold);
  Cayenne.virtualWrite(DATA_CHANNEL, sensor_value , "analog_sensor", "null"); //widget to display sensor data.
}
