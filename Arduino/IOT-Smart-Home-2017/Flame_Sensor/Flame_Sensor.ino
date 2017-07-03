/*
 Name:		Flame_Sensor.ino
 Created:	23/06/2017 23:08:16
 Author:	Ruben
*/

#include <ArduinoJson.h>
//#include <arduino_pins.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//*********DEFINES*********//
#define FLAME_SENSOR_PIN A0

StaticJsonBuffer<300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();


// Update these with values suitable for your network.

//const char* ssid = "droid_wlan";
//const char* password = "WlanDr01d16";

const char* ssid = "home_anytime";
const char* password = "iot2017!";

const char* mqtt_server = "192.168.1.67"; //local router

const char* mqtt_user = "modulo2"; //local router
const char* mqtt_pass = "modulo2";//local router

const char* mqtt_config_topic = "homeassistant/sensor/flame/config";
const char* mqtt_state_topic = "homeassistant/sensor/flame/state";
// Topic to receive light value threshold from the platform
const char* mqtt_set_topic = "homeassistant/sensor/flame/set";



WiFiClient espClient;
PubSubClient client(espClient);

int flame_value;
int mqtt_flame_level = 850;
bool isFlame = false;

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
		mqtt_flame_level = msg.toInt();
		Serial.println(mqtt_flame_level);
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

	flame_value = analogRead(FLAME_SENSOR_PIN); //Read data from analog pin and store it to value variable
	flame_value = map(flame_value, 0, 1024, 1024, 0);

	Serial.println(flame_value);

	if (flame_value > mqtt_flame_level && !isFlame)
	{
		Serial.println("FIRE");
		client.publish(mqtt_state_topic, "FIRE");
		isFlame = true;
	}
	else if (flame_value < mqtt_flame_level && isFlame)
	{
		Serial.println("No Fire");
		client.publish(mqtt_state_topic, "Safe");
		isFlame = false;
	}
	delay(1000);

}

void configure_MQTT_sensor()
{
	if (client.connect("FlameSensor", mqtt_user, mqtt_pass)) {
		Serial.println("MQTT_Connected");

		//******TEMPERATURE CONFIGURATION*******//
		JSONencoder["name"] = "Kitchen Flame Level";
		JSONencoder["unit_of_measurement"] = "";


		char JSONmessageBuffer[300];
		JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
		Serial.println(JSONmessageBuffer);

		if (!client.publish(mqtt_config_topic, JSONmessageBuffer, sizeof(JSONmessageBuffer)))
		{
			Serial.println("Flame Sensor Config Message Not Published");
		}
		if (!client.publish(mqtt_set_topic, String(mqtt_flame_level).c_str()))
		{
			Serial.println("Flame Sensor Set Message Not Published");
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