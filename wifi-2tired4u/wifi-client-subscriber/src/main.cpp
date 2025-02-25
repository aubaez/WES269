#include <Arduino.h>
#include <WiFi.h>
#include "TinyMqtt.h"   // MQTT client/broker
#include "TinyConsole.h" // MQTT
#include "TinyStreaming.h" // MQTT

//std::string topic="sensor/temperature";
std::string topic="PetStatus";

MqttBroker broker(1883);
MqttClient mqtt_a(&broker);
MqttClient mqtt_b(&broker);

void onPublishA(const MqttClient* /* srce */, const Topic& topic, const char* payload, size_t /* length */)
{ Serial << "--> Client A received msg on topic " << topic.c_str() << ", " << payload << endl; }

void onPublishB(const MqttClient* /* srce */, const Topic& topic, const char* payload, size_t /* length */)
{ Serial << "--> Client B received msg on topic " << topic.c_str() << ", " << payload << endl; }

void setup()
{
    Serial.begin(115200);

    // TODO
      Serial.begin(115200);
  delay(500);
  Serial << "init" << endl;

  mqtt_a.setCallback(onPublishA);
  mqtt_a.subscribe(topic);

  mqtt_b.setCallback(onPublishB);
  mqtt_b.subscribe(topic);
}

void loop()
{
    // TODO
    broker.loop();	// Don't forget to call loop() for all brokers and clients
  mqtt_a.loop();
  mqtt_b.loop();

  // ============= client A publish ================
	static const int intervalA = 5000;
	static uint32_t timerA = millis() + intervalA;
	
	if (millis() > timerA)
	{
		Serial << "A is publishing " << topic.c_str() << endl;
		timerA += intervalA;
		mqtt_a.publish(topic, "sent by A");
	}

  // ============= client B publish ================
	static const int intervalB = 3000;	// will send topic each 5000 ms
	static uint32_t timerB = millis() + intervalB;
	
	if (millis() > timerB)
	{
		Serial << "B is publishing " << topic.c_str() << endl;
		timerB += intervalB;
		mqtt_b.publish(topic, "sent by B");
	}
}

