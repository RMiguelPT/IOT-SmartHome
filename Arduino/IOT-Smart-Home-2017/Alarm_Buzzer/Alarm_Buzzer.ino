
#include <WiFi.h>
#include <ArduinoJson.h>
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
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define TOPICO_SUBSCRIBE "ha/buzzer1"     
#define TOPICO_PUBLISH   "ha/buzz1/in"   


#define ID_MQTT  "modulo2" 
#define USER_MQTT "modulo2"
#define PASS_MQTT "modulo2"                             

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

const char* SSID = "home_anytime";
const char* PASSWORD = "iot2017!";

const char* BROKER_MQTT = "192.168.1.67";
int BROKER_PORT = 1883;

WiFiClient espClient;
PubSubClient MQTT(espClient);


void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);


void setup()
{
	InitOutput();
	initSerial();
	initWiFi();
	initMQTT();
}


void initSerial()
{
	Serial.begin(115200);
}


void initWiFi()
{
	delay(10);
	Serial.println("------ WI-FI------");
	Serial.print("Connecting AP: ");
	Serial.println(SSID);
	Serial.println(" Connecting.....");

	reconectWiFi();
}


void initMQTT()
{
	MQTT.setServer(BROKER_MQTT, BROKER_PORT);
	MQTT.setCallback(mqtt_callback);
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
			tone(D1, 1000);
			Serial.println("ON");
		}
		if (msg.equals("OFF"))
		{
			noTone(D1);
			Serial.println("OFF");
		}
	}
}

void reconnectMQTT()
{
	while (!MQTT.connected())
	{
		Serial.print("*Connecting on Broker MQTT: ");
		Serial.println(BROKER_MQTT);
		if (MQTT.connect(ID_MQTT, USER_MQTT, PASS_MQTT))
		{
			Serial.println("Broker MQTT CONNECTED!");
			MQTT.subscribe(TOPICO_SUBSCRIBE);
		}
		else
		{
			Serial.println("Failed to connect to BROKER");
			Serial.println("Retry in 2s");
			delay(2000);
		}
	}
}

void reconectWiFi()
{
	if (WiFi.status() == WL_CONNECTED)
		return;

	WiFi.begin(SSID, PASSWORD);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(100);
		Serial.print(".");
	}

	Serial.println();
	Serial.print("Success connection AP: ");
	Serial.println(SSID);
	Serial.print("IP: ");
	Serial.println(WiFi.localIP());
}


void VerificaConexoesWiFIEMQTT(void)
{
	if (!MQTT.connected())
		reconnectMQTT();

	reconectWiFi();
}


void InitOutput(void)
{
	pinMode(D1, OUTPUT);
}


void loop()
{
	VerificaConexoesWiFIEMQTT();
	EnviaEstadoOutputMQTT();
	MQTT.loop();
}
