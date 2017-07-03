

#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1

/*
Name:		Ventilation.ino
Created:	16/06/2017 20:59:27
Author:	Ruben && paulo
*/
//#include <LiquidCrystal_I2C.h>
//#include <LiquidCrystal.h>
#include <ArduinoJson.h>
//#include <DHT_U.h>
#include <DHT.h>
//#include <arduino_pins.h>
#include <Adafruit_Sensor.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//*********DEFINES*********//

#define DHTTYPE DHT22     // DHT 22 
//#define DHTPIN 5

DHT dht(D1, DHTTYPE);

StaticJsonBuffer<300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();


// Update these with values suitable for your network.

//const char* ssid = "droid_wlan";
//const char* password = "WlanDr01d16";

const char* ssid = "home_anytime";
const char* password = "iot2017!";

const char* mqtt_server = "192.168.1.67";//local router
const char* mqtt_user = "modulo2";//local router
const char* mqtt_pass = "modulo2";//local router

const char* mqtt_temp_config_topic = "homeassistant/sensor/temperature/config";
const char* mqtt_hum_config_topic = "homeassistant/sensor/humidity/config";

const char* mqtt_temp_state_topic = "homeassistant/sensor/temperature/state";
const char* mqtt_hum_state_topic = "homeassistant/sensor/humidity/state";

#define TOPICO_SUBSCRIBE "switch/lamp" 


WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

float old_hum = 0;
float old_temp = 0;

void setup() {
	pinMode(D1, INPUT);
	pinMode(D2, OUTPUT);
	pinMode(D3, OUTPUT);
	digitalWrite(D2, LOW);
	digitalWrite(D3, LOW);

	dht.begin();
	delay(1000);
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

	Serial.begin(9600);
	//Serial.setDebugOutput(true);
	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(mqtt_callback);
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


void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
	String msg;
	for (int i = 0; i < length; i++)
	{
		char c = (char)payload[i];
		msg += c;
	}

	
	if (String(topic) == TOPICO_SUBSCRIBE) {
		if (msg.equals("ON"))
		{
			digitalWrite(D3, HIGH);
			Serial.println(msg);

		}
		if (msg.equals("OFF"))
		{
			Serial.println(msg);

			digitalWrite(D3, LOW);
		}
	}


}

void reconnect() {
	// Loop until we're reconnected
	Serial.print("Attempting MQTT connection...");
	while (!client.connected()) {
		// Attempt to connect

		configure_MQTT_temp_sensor();
		configure_MQTT_hum_sensor();

		client.subscribe(TOPICO_SUBSCRIBE);
	}
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
	return !isnan(newValue) &&
		(newValue <= prevValue - maxDiff || newValue >= prevValue + maxDiff);
}


void loop() {
	float temp = 0.0;	
	float hum = 0.0;


	if (!client.connected()) {
		reconnect();
	}
	client.loop();

	temp = dht.readTemperature();
	hum = dht.readHumidity();


	if (checkBound(temp, old_temp, 0.1)) {
		client.publish(mqtt_temp_state_topic, String(temp).c_str());
		Serial.println(temp);
		old_temp = temp;
	}	

	if (checkBound(hum, old_hum, 0.1)) {
		client.publish(mqtt_hum_state_topic, String(hum).c_str());
		old_hum = hum;
	}
	
	delay(5000);

}

void configure_MQTT_temp_sensor()
{
	if (client.connect("Temperature_&_Humidity_Sensor", mqtt_user, mqtt_pass)) {
		Serial.println("MQTT_Connected");

		//******TEMPERATURE CONFIGURATION*******//
		JSONencoder["name"] = "Living Room Temperature";
		JSONencoder["unit_of_measurement"] = "\u2103";


		char JSONmessageBuffer[300];
		JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
		Serial.println(JSONmessageBuffer);

		if (!client.publish(mqtt_temp_config_topic, JSONmessageBuffer, sizeof(JSONmessageBuffer)))
		{
			Serial.println("Temperature Config Message Not Published");
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
void configure_MQTT_hum_sensor()
{
	if (client.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
		Serial.println("MQTT_Connected");

		//******HUMIDITY CONFIGURATION*******//
		JSONencoder["name"] = "Living Room Humidity";
		JSONencoder["device_class"] = "Humidity";
		JSONencoder["unit_of_measurement"] = "%";

		char JSONmessageBuffer[300];
		JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
		Serial.println(JSONmessageBuffer);


		if (!client.publish(mqtt_hum_config_topic, JSONmessageBuffer, sizeof(JSONmessageBuffer)))
		{
			Serial.println("Humidity Config Message Not Published");
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