/*
 Name:		Panic_Button.ino
 Created:	27/06/2017 20:38:25
 Author:	Ruben
*/

#include <arduino_pins.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//*********DEFINES*********//

#define BUTTON_PIN D1
#define SINGLEPRESSED 1
#define HELDED 2
#define DOUBLEPRESS 3

StaticJsonBuffer<300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();


// Update these with values suitable for your network.
//const char* ssid = "droid_wlan";
//char* password = "WlanDr01d16";

const char* ssid = "home_anytime"; //local crouter
const char* password = "iot2017!"; //local router

const char* mqtt_server = "192.168.1.67"; //local router
const char* mqtt_user = "modulo2";
const char* mqtt_pass = "modulo2";

const char* mqtt_config_topic = "homeassistant/binary_sensor/panic/config";
const char* mqtt_state_topic = "homeassistant/binary_sensor/panic/state";



WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];

float panic_button_held_time = 0.0;


int bounceTime = 50;
float holdTime = 2000.0;
int doubleTime = 500;

int lastReading = LOW;
int hold = 0;
int single = 0;
int LEDstate = 0;

long onTime = 0;
long lastSwitchTime = 0;


void setup() {

	pinMode(BUTTON_PIN, INPUT_PULLUP);

	delay(1000);
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output


	Serial.begin(9600);
	setup_wifi();
	client.setServer(mqtt_server, 1883);
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

	if (pressedTime(holdTime) == HELDED)
	{
		client.publish(mqtt_state_topic, String(1).c_str());
	}
	
}

void configure_MQTT_sensor()
{
	if (client.connect("PanicButton", mqtt_user, mqtt_pass)) {
		Serial.println("MQTT_Connected");

		//****** CONFIGURATION*******//
		JSONencoder["name"] = "Panic Button";
		JSONencoder["state_topic"] = mqtt_state_topic;
		JSONencoder["payload_on"] = 1;
		JSONencoder["payload_off"] = 0;

		char JSONmessageBuffer[300];
		JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
		Serial.println(JSONmessageBuffer);

		if (!client.publish(mqtt_config_topic, JSONmessageBuffer, sizeof(JSONmessageBuffer)))
		{
			Serial.println("Panic Button Config Message Not Published");
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


// User JamesCarruthers url: http://forum.arduino.cc/index.php?topic=42489.0
int pressedTime(float hold_time)
{
	int reading = digitalRead(BUTTON_PIN);
	int state = 0;
	

	//first pressed
	if (reading == LOW && lastReading == HIGH) {
		onTime = millis();
	}

	//held
	if (reading == LOW && lastReading == LOW) {
		if ((millis() - onTime) > hold_time) {
			hold = 1;
		}
	}

	//released
	if (reading == HIGH && lastReading == LOW) {
		if (((millis() - onTime) > bounceTime) && hold != 1) {
			state = onRelease();
		}
		if (hold == 1) {
			Serial.println("held");
			hold = 0;
			state = HELDED;
		}
	}
	lastReading = reading;

	if (single == 1 && (millis() - lastSwitchTime) > doubleTime) {
		Serial.println("single press");
		single = 0;
		state = SINGLEPRESSED;
	}

	return state;
}

int onRelease() {

	if ((millis() - lastSwitchTime) >= doubleTime) {
		single = 1;
		lastSwitchTime = millis();
		return 0;
	}

	if ((millis() - lastSwitchTime) < doubleTime) {
		Serial.println("double press");
		single = 0;
		lastSwitchTime = millis();
		return DOUBLEPRESS;
	}

}