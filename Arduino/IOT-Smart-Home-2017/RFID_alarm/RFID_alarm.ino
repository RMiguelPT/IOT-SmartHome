#include <ArduinoJson.h>
#include <require_cpp11.h>
#include <deprecated.h>
#include <SoftwareSerial.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>

#include <SPI.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define TOPICO_SUBSCRIBE "sensor/RFID"     
#define TOPICO_PUBLISH   "sensor/RFID"   

#define ID_MQTT  "RFID"
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

#define BUZZER_SENSOR_PIN D3

const char* SSID = "home_anytime";
const char* PASSWORD = "iot2017!";

const char* BROKER_MQTT = "192.168.1.67";
int BROKER_PORT = 1883;

int stateAlarm = 1;
MFRC522 mfrc522(D1, D2);
MFRC522::MIFARE_Key key;

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
	initSerial();
	initWiFi();
	initMQTT();
}

void initSerial()
{
	Serial.begin(9600);
	SPI.begin();
	mfrc522.PCD_Init();
	pinMode(BUZZER_SENSOR_PIN, OUTPUT);
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
		if (msg.equals("ARMED"))
		{
			stateAlarm = 1;
		}
		if (msg.equals("DISARMED"))
		{
			stateAlarm = 0;
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



void loop()
{

	VerificaConexoesWiFIEMQTT();
	MQTT.loop();

	//Verificando existencia do card no leitor
	if (!mfrc522.PICC_IsNewCardPresent()) {
		delay(1000);
		return;
	}

	//Verificando Leitura do card
	if (!mfrc522.PICC_ReadCardSerial()) {
		delay(1000);
		return;
	}
	//Mostra UID na serial


	Serial.print("UID da tag : ");
	String conteudo = "";
	byte letra;
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		// Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
		// Serial.print(mfrc522.uid.uidByte[i], HEX);
		conteudo.concat(String(mfrc522.uid.uidByte[i]<0x10 ? " 0" : " "));
		conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
	}
	Serial.println(conteudo);
	if (conteudo.equals(" 01 e2 b1 65")) {
		tone(BUZZER_SENSOR_PIN, 1500, 250);
		delay(1000);
	  noTone(BUZZER_SENSOR_PIN);   
    
		Serial.println("auth");
		if (stateAlarm != 1) {
			MQTT.publish(TOPICO_PUBLISH, "ARMED");
		}
		else { MQTT.publish(TOPICO_PUBLISH, "DISARMED"); }
	}
}
