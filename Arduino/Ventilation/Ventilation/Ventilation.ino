/*
Name:		Ventilation.ino
Created:	22/06/2017 14:32:12
Author:	Ruben
*/
#include <ArduinoJson.h>
#include <arduino_pins.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

StaticJsonBuffer<300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();

#define VENTILATION_PIN D4

/********************NETWORK CONFIGURATION******************/
/*--DroidLAN--*/
//const char* ssid = "droid_wlan";
//const char* password = "WlanDr01d16";

const char* ssid = "home_anytime";
const char* password = "iot2017!";

const char* mqtt_server = "192.168.1.67";//local router
const char* mqtt_user = "modulo2";//local router
const char* mqtt_pass = "modulo2";//local router


const char* mqtt_config_topic = "homeassistant/switch/ventilation/config";
const char* mqtt_state_topic = "homeassistant/switch/ventilation/state";
const char* mqtt_command_topic = "homeassistant/switch/ventilation/set";



WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;





void setup() {

	pinMode(VENTILATION_PIN, OUTPUT);
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output



	Serial.begin(9600);
	//Serial.setDebugOutput(true);
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

	if ((char)payload[0] == '1') {
		digitalWrite(VENTILATION_PIN, HIGH);   
		client.publish(mqtt_state_topic, "1");
	}
	else {
		digitalWrite(VENTILATION_PIN, LOW);  // Turn the LED off by making the voltage HIGH
		client.publish(mqtt_state_topic, "0");
	}

}

void reconnect() {
	// Loop until we're reconnected
	Serial.print("Attempting MQTT connection...");
	while (!client.connected()) {
		configure_MQTT_switch();
	}
}
void loop() {

	if (!client.connected()) {
		Serial.println("******MQTT NOT CONNECTED******");
		reconnect();		
	}
	client.loop();
	delay(2500);

}

void configure_MQTT_switch()
{
	if (client.connect("Ventilation1", mqtt_user, mqtt_pass)) {
		Serial.println("MQTT_Connected");

		//******TEMPERATURE CONFIGURATION*******//
		JSONencoder["name"] = "Living Room Ventilation";
		JSONencoder["command_topic"] = mqtt_command_topic;
		JSONencoder["payload_on"] = 1;
		JSONencoder["payload_off"] = 0;


		char JSONmessageBuffer[300];
		JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
		Serial.println(JSONmessageBuffer);

		if (!client.publish(mqtt_config_topic, JSONmessageBuffer, sizeof(JSONmessageBuffer)))
		{
			Serial.println("Ventilation Config Message Not Published");
		}
		client.subscribe(mqtt_command_topic);

	}
	else {
		Serial.print("failed, rc=");
		Serial.print(client.state());
		Serial.println(" try again in 5 seconds");
		// Wait 5 seconds before retrying
		delay(5000);
	}
}
