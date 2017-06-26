/*
  Name:		Gas_n_Smoke.ino
  Created:	23/06/2017 23:45:57
  Author:	Ruben & paulo
  */

#include <ArduinoJson.h>
//#include <arduino_pins.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

  //*********DEFINES*********//
#define LIGHT_SENSOR_PIN A0

	StaticJsonBuffer<300> JSONbuffer;
	JsonObject& JSONencoder = JSONbuffer.createObject();


// Update these with values suitable for your network.

//const char* ssid = "droid_wlan";
//const char* password = "WlanDr01d16";

const char* ssid = "home_anytime";
const char* password = "iot2017!";

//const char* ssid = "BitNet-Informatica";
//const char* password = "bitnet-infor-2014*";

//const char* mqtt_server = "10.20.228.238";
//const char* mqtt_user = "pi";
//const char* mqtt_pass = "raspberry";
const char* mqtt_server = "192.168.1.67";
const char* mqtt_user = "modulo2";
const char* mqtt_pass = "modulo2";


const char* mqtt_config_topic = "homeassistant/sensor/light/config";
const char* mqtt_state_topic = "homeassistant/sensor/light/state";



WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int light_value = 0;
int old_light_vlue = 0;

void setup() {
	pinMode(D1, INPUT);

	delay(1000);
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

	Serial.begin(9600);
	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
}

void setup_wifi() {

	delay(100);
	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");

	WiFi.mode(WIFI_STA);
	Serial.println(ssid);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();

	// Switch on the LED if an 1 was received as first character
	if ((char)payload[0] == '1') {
		digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
										  // but actually the LED is on; this is because
										  // it is acive low on the ESP-01)
	}
	else {
		digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
	}

}

void reconnect() {
	// Loop until we're reconnected
	Serial.print("Attempting MQTT connection...");
	while (!client.connected()) {
		// Attempt to connect
		configure_MQTT_sensor();
	}
}
void loop() {
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
	light_value = analogRead(LIGHT_SENSOR_PIN); //Read data from analog pin and store it to value variable

	if (light_value != old_light_vlue) {
		Serial.print("Light Value: ");
		Serial.print(light_value);
		//light_value = map(light_value, 0, 1024, 1024, 0);
		client.publish(mqtt_state_topic, String(light_value).c_str());
		old_light_vlue = light_value;
	}
	else {
		old_light_vlue = light_value;
	}
	
	delay(500);
}

void configure_MQTT_sensor()
{
	if (client.connect("LightSensor", mqtt_user, mqtt_pass)) {
		Serial.println("MQTT_Connected");

		//******LIGHT CONFIGURATION*******//
		JSONencoder["name"] = "Living Room Light Level";
		JSONencoder["unit_of_measurement"] = "lums";

		char JSONmessageBuffer[300];
		JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
		Serial.println(JSONmessageBuffer);

		if (!client.publish(mqtt_config_topic, JSONmessageBuffer, sizeof(JSONmessageBuffer)))
		{
			Serial.println("Light Sensor Config Message Not Published");
		}
	}
	else {
		Serial.print("failed, rc=");
		Serial.print(client.state());
		Serial.println(" try again in 5 seconds");
		// Wait 5 seconds before retrying
		delay(5000);
	}
}

