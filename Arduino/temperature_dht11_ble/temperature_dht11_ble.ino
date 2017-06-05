#include "Arduino.h"
#include <SoftwareSerial.h>

#define BLE_VCC 3
#define BLE_GND 4

#define BLE_RX 5
#define BLE_TX 6

#define SENS_VCC 11
#define SENS_GND 10

#define SENS_SIGNAL 12
SoftwareSerial ble(5,6);

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(BLE_GND, OUTPUT);
	pinMode(BLE_VCC, OUTPUT);
	digitalWrite(BLE_GND, LOW);
	digitalWrite(BLE_VCC, HIGH);


	Serial.begin(9600);
	ble.begin(9600);
}

// The loop function is called in an endless loop
void loop()
{
	while (ble.available()){
		Serial.write(ble.read());
	}
	if (Serial.available()){
		ble.write(Serial.read());
	}
	//Add your repeated code here
}
