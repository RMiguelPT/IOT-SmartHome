/*
 Name:		Monitor_Screen.ino
 Created:	24/06/2017 12:10:38
 Author:	Ruben
*/


#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <arduino_pins.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//*********DEFINES*********//
#define FORWARD_BUTTON_PIN D4

#define TEMP_SCREEN 1
#define HUM_SCREEN 2
#define WATER_SCREEN 3
#define VENTILATION_SCREEN 4
#define LIGHT_SCREEN 5
#define GAS_SCREEN 6
#define FLAMES_SCREEN 7
#define SSID_SCREEN 8
#define IP_SCREEN 9

#define ON 1
#define OFF 0

// Update these with values suitable for your network.

//const char* ssid = "droid_wlan";
//const char* password = "WlanDr01d16";

const char* ssid = "home_anytime";
const char* password = "iot2017!";

const char* mqtt_server = "192.168.1.67";
const char* mqtt_user = "modulo2";
const char* mqtt_pass = "modulo2";

const char* mqtt_temperature_state_topic = "homeassistant/sensor/temperature/state";
const char* mqtt_humidity_state_topic = "homeassistant/sensor/humidity/state";
const char* mqtt_water_state_topic = "homeassistant/sensor/water/state";
const char* mqtt_light_state_topic = "homeassistant/sensor/light/state";
const char* mqtt_ventilation_state_topic = "homeassistant/switch/ventilation/state";
const char* mqtt_gas_state_topic = "homeassistant/sensor/gas/state";
const char* mqtt_flame_state_topic = "homeassistant/sensor/flame/state";


float temperature = 0.0;
float humidity = 0.0;
String water;
String light;
int ventilation = 0;
String gas;
String flame_value;

int screen_number = 1;

StaticJsonBuffer<300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();

WiFiClient espClient;
PubSubClient client(espClient);
// set the LCD address to 0x27 for a 16 chars 2 line display
// A FEW use address 0x3F
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol

LiquidCrystal_I2C lcd(0x3f, 20, 4);  // Set the LCD I2C address





int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button


void setup() {

	pinMode(D1, INPUT);
	pinMode(FORWARD_BUTTON_PIN, INPUT);

	//delay(1000);
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

									 
	lcd.begin();
	lcd.backlight(); // finish with backlight on  
	lcd.clear();


	Serial.begin(9600);
	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
}

void setup_wifi() {

	delay(100);
	// We start by connecting to a WiFi network
	Serial.println();
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Connecting to ");
	Serial.print("Connecting to ");
	lcd.setCursor(0, 1);

	WiFi.mode(WIFI_STA);
	Serial.println(ssid);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(5000);
		Serial.print(".");
		lcd.print(".");

	}

	lcd.setCursor(0, 0);
	lcd.print("WiFi connected");
	lcd.setCursor(0, 1);
	lcd.print("IP address: ");
	lcd.setCursor(0, 2);
	lcd.print(WiFi.localIP());


	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	delay(2000);
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("-- HOME ASSISTANT --");
}

void callback(char* topic, byte* payload, unsigned int length) {
	String msg;
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
		//msg += (char)payload[i];
	}
	//Serial.println();*/

	if (strcmp(topic, mqtt_temperature_state_topic) == 0 ) {
		//char buffer[10];
		for (int i = 0; i < length; i++) {
			msg += (char)payload[i];
		}
		temperature = msg.toFloat();
		if (screen_number == TEMP_SCREEN)
		{
			show_data();
		}
	}
	if (strcmp(topic, mqtt_humidity_state_topic) == 0) {
		//char buffer[10];
		for (int i = 0; i < length; i++) {
			msg += (char)payload[i];
		}
		//humidity = atof(buffer);
		humidity = msg.toFloat();
		if (screen_number == HUM_SCREEN)
		{
			show_data();
		}
	}
	if (strcmp(topic, mqtt_water_state_topic) == 0) {
		for (int i = 0; i < length; i++) {

			//water[i] = (char)payload[i];
			msg += (char)payload[i];
		}
		water = msg;
		//water = (char*)payload;
		if (screen_number == WATER_SCREEN)
		{
			show_data();
			//memset(water, 0, sizeof(water));
		}
	}
	if (strcmp(topic, mqtt_ventilation_state_topic) == 0) {

		/*Serial.print("Message arrived [");
		Serial.print(topic);
		Serial.print("] ");*/
		//char buffer[2];
		//buffer[0] = (char)payload[0];
		msg[0] = (char)payload[0];


		ventilation = msg.toInt();
		Serial.println("VENTILATION:");
		Serial.println(ventilation);
		if (screen_number == VENTILATION_SCREEN)
		{
			show_data();
		}
	}
	if (strcmp(topic, mqtt_light_state_topic) == 0) {
		//char buffer[10];
		for (int i = 0; i < length; i++) {
			msg += (char)payload[i];
		}
		light = msg;
		if (screen_number == LIGHT_SCREEN)
		{
			show_data();
		}
	}

	if (strcmp(topic, mqtt_gas_state_topic) == 0) {
		//char buffer[10];
		for (int i = 0; i < length; i++) {
			msg += (char)payload[i];
		}
		gas = msg;
		if (screen_number == GAS_SCREEN)
		{
			show_data();
		}
	}

	if (strcmp(topic, mqtt_flame_state_topic) == 0) {
		//char buffer[10];
		for (int i = 0; i < length; i++) {
			msg += (char)payload[i];
		}
		flame_value = msg;
		if (screen_number == FLAMES_SCREEN)
		{
			show_data();
		}
	}


	/*Topico flame*/
}

void reconnect() {
	// Loop until we're reconnected
	int i = 0;
	Serial.print("Attempting MQTT connection...");
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Attempting MQTT connection...");
	while (!client.connected()) {

		if (client.connect("Monitor", mqtt_user, mqtt_pass)) {
			Serial.println("MQTT Connected");
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("MQTT Connected");

			client.subscribe(mqtt_temperature_state_topic);
			client.subscribe(mqtt_humidity_state_topic);
			client.subscribe(mqtt_water_state_topic);
			client.subscribe(mqtt_ventilation_state_topic);
			client.subscribe(mqtt_light_state_topic);
			client.subscribe(mqtt_gas_state_topic);
			client.subscribe(mqtt_flame_state_topic);


		}
		else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}

}
void loop() {

	if (!client.connected()) {
		reconnect();
	}
	client.loop();

	
	// read the pushbutton input pin:
	buttonState = digitalRead(FORWARD_BUTTON_PIN);

	// compare the buttonState to its previous state
	if (buttonState != lastButtonState) {
		// if the state has changed, increment the counter
		if (buttonState == LOW) {
			// if the current state is HIGH then the button
			// wend from off to on:
			buttonPushCounter++;
			Serial.println("on");
			Serial.print("number of button pushes:  ");
			Serial.println(buttonPushCounter);

			screen_number++;
			if (screen_number > 9)
			{
				screen_number = 1;
			}
			show_data();

		}
		else {
			// if the current state is LOW then the button
			// wend from on to off:
			Serial.println("off");
		}
		// Delay a little bit to avoid bouncing
		delay(50);
	}
	// save the current state as the last state,
	//for next time through the loop
	lastButtonState = buttonState;
}

void show_data() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("-- HOME ASSISTANT --");

	switch (screen_number)
	{
		case TEMP_SCREEN: //TEMPERATURE
			lcd.setCursor(0, 1);
			lcd.print("    Living  Room    ");
			lcd.setCursor(0, 2);
			lcd.print("    Temperature    ");
			lcd.setCursor(0, 3);
			lcd.print(temperature);
			lcd.print((char)223);
			lcd.print("C");
				break;
		case HUM_SCREEN: //HUMIDITY
			lcd.setCursor(0, 1);
			lcd.print("    Living  Room    ");
			lcd.setCursor(0, 2);
			lcd.print("    Humidity    ");
			lcd.setCursor(0, 3);
			lcd.print(humidity);
			lcd.print("%");
				break;
		case WATER_SCREEN: //WATER
			lcd.setCursor(0, 1);
			lcd.print("    Kitchen    ");
			lcd.setCursor(0, 2);
			lcd.print(water);
				break;
		case VENTILATION_SCREEN: //Ventilation
			lcd.setCursor(0, 1);
			lcd.print("    Living Room    ");
			lcd.setCursor(0, 2);
			lcd.print("    Ventilation    ");
			lcd.setCursor(0, 3);
			if (ventilation == OFF) {
				lcd.print("OFF");
			}
			if(ventilation == ON) {
				lcd.print("ON");
			}			
				break;
		case LIGHT_SCREEN: //LIGHT
			lcd.setCursor(0, 1);
			lcd.print("    Living  Room    ");
			lcd.setCursor(0, 2);
			lcd.print("    Light Level    ");
			lcd.setCursor(0, 3);
			lcd.print(light);
				break;
		case GAS_SCREEN: //GAS
			lcd.setCursor(0, 1);
			lcd.print("    Kitchen    ");
			lcd.setCursor(0, 2);
			lcd.print("  Gas/Smoke Level   ");
			lcd.setCursor(0, 3);
			lcd.print(gas);
				break;
		case FLAMES_SCREEN: //FLAMES
			lcd.setCursor(0, 1);
			lcd.print("    Kitchen    ");
			lcd.setCursor(0, 2);
			lcd.print("  Flames Detector   ");
			lcd.setCursor(0, 3);
			lcd.print(flame_value);
			break;
		case SSID_SCREEN: //SSID
			lcd.setCursor(0, 1);
			lcd.print("    SSID   ");
			lcd.setCursor(0, 2);
			lcd.print(ssid);
				break;
		case IP_SCREEN: //IP ADDR
			lcd.setCursor(0, 1);
			lcd.print("    IP Address    ");
			lcd.setCursor(0, 2);
			lcd.print(WiFi.localIP());
			break;
		default:
			break;
	}
}
