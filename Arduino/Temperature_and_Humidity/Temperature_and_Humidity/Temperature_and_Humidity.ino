/*
Basic ESP8266 MQTT example

This sketch demonstrates the capabilities of the pubsub library in combination
with the ESP8266 board/library.

It connects to an MQTT server then:
- publishes "hello world" to the topic "outTopic" every two seconds
- subscribes to the topic "inTopic", printing out any messages
it receives. NB - it assumes the received payloads are strings not binary
- If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
else switch it off

It will reconnect to the server if the connection is lost using a blocking
reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
achieve the same result without blocking the main loop.

To install the ESP8266 board, (using Arduino 1.6.4+):
- Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
http://arduino.esp8266.com/stable/package_esp8266com_index.json
- Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
- Select your ESP8266 in "Tools -> Board"

*/

#include <ArduinoJson.h>
//#include <DHT_U.h>
#include <DHT.h>
#include <arduino_pins.h>
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

const char* ssid = "BitNet-Informatica";
const char* password = "bitnet-infor-2014*";

//const char* mqtt_server = "10.20.228.211";
const char* mqtt_server = "192.168.1.14";
const char* mqtt_user = "pi";
const char* mqtt_pass = "raspberry";

const char* mqtt_temp_config_topic = "homeassistant/sensor/temperature/config";
const char* mqtt_hum_config_topic = "homeassistant/sensor/humidity/config";

const char* mqtt_temp_state_topic = "homeassistant/sensor/temperature/state";
const char* mqtt_hum_state_topic = "homeassistant/sensor/humidity/state";



WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;



void setup() {

	pinMode(D1, INPUT);
	dht.begin();
	delay(1000);
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
	Serial.println(ssid);

	WiFi.mode(WIFI_STA);
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
	while (!client.connected()) {
		Serial.println("Attempting MQTT connection...");
		// Attempt to connect
	
		if (client.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
			Serial.println("MQTT_Connected");

			//******TEMPERATURE CONFIGURATION*******//
			JSONencoder["name"] = "Living Room Temperature";
			JSONencoder["device_class"] = "Temperature";
			JSONencoder["unit_of_measurement"] = "ºC";
			
			char JSONmessageBuffer[301];
			JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
			JSONencoder.printTo(Serial);
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
}
void loop() {
	float temp = 0;
	int hum = 0;

	if (!client.connected()) {
		reconnect();
	}
	client.loop();

	temp = dht.readTemperature();
	

	
	if (isnan(temp))
	{
		Serial.print(F("Failed to read from DHT sensor!\r\n"));
	}
	else
	{
		Serial.println(temp);
		client.publish(mqtt_temp_state_topic, String(temp).c_str());
	}
	delay(2500);


	//client.publish(mqtt_temp_state_topic, String(temp).c_str(), true);
	//delay(1000);
	//hum = dht.readHumidity();
	


	
	//client.publish(mqtt_temp_state_topic, "0.0");
	//client.publish(mqtt_hum_state_topic, String(hum).c_str(), true);

	//delay(5000);

}

