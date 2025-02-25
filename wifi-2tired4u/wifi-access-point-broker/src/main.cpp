#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "TinyMqtt.h"   // MQTT client/broker
#include "TinyConsole.h" // MQTT
#include "TinyStreaming.h" // MQTT

uint16_t PORT = 1883;
const uint8_t  RETAIN = 10;  // Max retained messages
MqttBroker broker(PORT);

const char* ssid = "tock_tutorial";
const char* password = "";


void setup()
{
    Serial.begin(115200);

    // TODO
	if (strlen(ssid)==0)
		Console << TinyConsole::red << "****** PLEASE MODIFY ssid/password *************" << endl;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial << '.';
    delay(500);
  }
  Console << TinyConsole::green << "Connected to " << ssid << "IP address: " << WiFi.localIP() << endl;

  broker.begin();
  Console << "Broker ready : " << WiFi.localIP() << " on port " << PORT << endl;
}

void loop()
{
    // TODO
    broker.loop();
}

