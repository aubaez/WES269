#include <Arduino.h>
#include <WiFi.h>
#include "TinyMqtt.h"   // MQTT client/broker
#include "TinyConsole.h" // MQTT
#include "TinyStreaming.h" // MQTT

const char* BROKER = "broker.emqx.io";
const uint16_t BROKER_PORT = 1883;

const char* ssid = "tock_tutorial";
const char* password = "";


char *petStatuses[] = {
        "Your pet is currently playing outside",
        "Your pet is currently drinking water",
        "Your pet is inside sleeping"
    };
int numStatuses = sizeof(petStatuses) / sizeof(petStatuses[0]);
char* message;
int idx = 0;

//static float temp=19;
static MqttClient client;

void setup()
{
    Serial.begin(115200);

    // TODO
    	delay(500);

	Serial << "Simple clients with wifi " << endl;
	if (strlen(ssid)==0)
		Serial << "****** PLEASE MODIFY ssid/password *************" << endl;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
	{ delay(500); Serial << '.'; }

  Serial << "Connected to " << ssid << "IP address: " << WiFi.localIP() << endl;

	client.connect(BROKER, BROKER_PORT);
}

void loop()
{
    // TODO
    client.loop();	// Don't forget to call loop() for each broker and client

	// delay(1000);		please avoid usage of delay (see below how this done using next_send and millis())
	static auto next_send = millis();
	
	if (millis() > next_send)
	{
		next_send += 1000;

		if (not client.connected())
		{
			Serial << millis() << ": Not connected to broker" << endl;
			return;
		}

		//auto rnd=random(100);

		//if (rnd > 66) temp += 0.1;
		//else if (rnd < 33) temp -= 0.1;
		int temp = idx%numStatuses;
		message = petStatuses[temp];
		idx = idx+ 1;

		//Serial << "--> Publishing a new sensor/temperature value: " << temp << endl;
		Serial << "--> PetStatus: " << message << endl;

		client.publish("PetStatus", message);
	}
}

