/*
 Name:		Gas_n_Smoke.ino
 Created:	23/06/2017 23:45:57
 Author:	Ruben
*/

#include <ArduinoJson.h>
#include <arduino_pins.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//*********DEFINES*********//
#define GAS_SENSOR_PIN A0

StaticJsonBuffer<300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();


// Update these with values suitable for your network.

//const char* ssid = "droid_wlan";
//const char* password = "WlanDr01d16";

const char* ssid = "home_anytime"; //SSID
const char* password = "iot2017!"; //WiFi Password

const char* mqtt_server = "192.168.1.67"; //mqtt broker
const char* mqtt_user = "modulo2"; // mqtt broker username
const char* mqtt_pass = "modulo2"; // mqtt broker password

const char* mqtt_config_topic = "homeassistant/sensor/gas/config";
const char* mqtt_state_topic = "homeassistant/sensor/gas/state";
// Topic to receive gas value threshold from the platform
const char* mqtt_set_topic = "homeassistant/sensor/gas/set";


WiFiClient espClient;
PubSubClient client(espClient);


int gas_value = 0;
int mqtt_gas_level = 200;
bool gas_detected = false;

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
		delay(5000);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
	String msg;
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
		msg += (char)payload[i];
	}
	
	Serial.println();
	
	if (String(topic) == mqtt_set_topic)
	{
		mqtt_gas_level =  msg.toInt();
		Serial.println(mqtt_gas_level);
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

	gas_value = analogRead(GAS_SENSOR_PIN); //Read data from analog pin and store it to value variable
	Serial.println(gas_value);

	if (gas_value > mqtt_gas_level && !gas_detected)
	{
		client.publish(mqtt_state_topic, "GAS DETECTED");
		gas_detected = true;
	}
	else if(gas_value < mqtt_gas_level && gas_detected)
	{
		client.publish(mqtt_state_topic, "Air Safe");
		gas_detected = false;

	}
	
	delay(3000);
}

void configure_MQTT_sensor()
{
	if (client.connect("GasSensor", mqtt_user, mqtt_pass)) {
		Serial.println("MQTT_Connected");

		//******TEMPERATURE CONFIGURATION*******//
		JSONencoder["name"] = "Kitchen Gas Presence";
		JSONencoder["unit_of_measurement"] = "";


		char JSONmessageBuffer[300];
		JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
		Serial.println(JSONmessageBuffer);

		if (!client.publish(mqtt_config_topic, JSONmessageBuffer, sizeof(JSONmessageBuffer)))
		{
			Serial.println("Gas Sensor Config Message Not Published");
		}
		if (!client.publish(mqtt_set_topic, String(mqtt_gas_level).c_str()))
		{
			Serial.println("Gas Sensor Set Message Not Published");
		}
		client.subscribe(mqtt_set_topic);


	}
	else {
		Serial.print("failed, rc=");
		Serial.print(client.state());
		Serial.println(" try again in 5 seconds");
		// Wait 5 seconds before retrying
		delay(5000);
	}
}

