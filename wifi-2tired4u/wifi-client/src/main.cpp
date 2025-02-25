#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <esp_wifi.h>


const char* SSID     = "tock_tutorial"; // MUST update this
const char* PASSWORD = ""; // MUST update this
WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP);

uint8_t MacAddr[6];
esp_err_t ret;
void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi network");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
    }
    Serial.println(WiFi.localIP());
    Serial.print("Client device is connected to Channel ");
    Serial.println(WiFi.channel());
    Serial.print("Client device MAC address is ");

    ret = esp_wifi_get_mac(WIFI_IF_STA, MacAddr);
    if(ret == ESP_OK){
      Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  MacAddr[0], MacAddr[1], MacAddr[2],
                  MacAddr[3], MacAddr[4], MacAddr[5]);
    }
    else{
      Serial.println("Failed to read MAC address");
    }
    timeClient.begin();
}

void loop()
{
    // TODO
    timeClient.update();

    Serial.println(timeClient.getFormattedTime());
    Serial.println(timeClient.getFormattedDate());

    delay(1000);
    timeClient.forceUpdate();
}