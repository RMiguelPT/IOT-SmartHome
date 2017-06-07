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


#define MQTT_SENSOR_TOPIC "hassbian"
#define MQTT_SENSOR_TYPE "sensor"
#define MQTT_SENSOR_ID "room_temp"
#define MQTT_SENSOR_CONFIG "config"

#define MQTT_SENSOR_FULL_CONFIG "hassbian/sensor/roomTemp/config"

/*FUNCTION PROTOTYPES*/
bool is_just_connected();
void send_config();

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(BLE_GND, OUTPUT);
	pinMode(BLE_VCC, OUTPUT);
	digitalWrite(BLE_GND, LOW);
	digitalWrite(BLE_VCC, HIGH);


	Serial.begin(9600);
	ble.begin(9600);

	ble.setTimeout(200);
}

// The loop function is called in an endless loop
void loop()
{
	/*while (ble.available()){
		Serial.write(ble.read());
	}*/
	if (is_just_connected())
	{
		Serial.println("CONNECTED");
		delay(2000);
		send_config();
	}


	if (Serial.available()){
		ble.write(Serial.read());
	}
	//Add your repeated code here
}

bool is_just_connected()
{
	if (ble.available()>0)
	{
		if (ble.readString()=="OK+CONN")
		{
			return true;
		}
	}
	return false;
}

void send_config()
{
	ble.write(MQTT_SENSOR_TOPIC);
	delay(100);
	ble.write(MQTT_SENSOR_TYPE);
	delay(100);
	ble.write(MQTT_SENSOR_ID);
	delay(100);
	ble.write(MQTT_SENSOR_CONFIG);
	delay(100);
}
