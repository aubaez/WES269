#include <Arduino.h> // Required for all code
#include <WiFi.h> // Required for all wifi code

void setup()
{
    Serial.begin(115200);

    // TODO
    WiFi.mode(WIFI_STA); //standard way to connect to existing network
    WiFi.disconnect();
    delay(100);

    Serial.println("Setup done");
}

void loop()
{
    // TODO
    Serial.println("Starting Scan");

    int numNetworks = WiFi.scanNetworks();
    Serial.println("Completed Scan");

    if(numNetworks == 0){
        Serial.println("No Networks Found!");
    }
    else{
        Serial.print(numNetworks); // print the # of networks
        Serial.println(" Networks Found"); //string message
        Serial.println("Nr | SSID                            | RSSI | CH | Encryption"); //table header
        for(int i = 0; i < numNetworks; ++i){ // step through list of Networks
           //Print SSID and RSSI for each network found 
            Serial.printf("%2d", i + 1);
            Serial.print(" | ");
            Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            Serial.print(" | ");
            Serial.printf("%4ld", WiFi.RSSI(i));
            Serial.print(" | ");
            Serial.printf("%2ld", WiFi.channel(i));
            Serial.print(" | ");
            switch (WiFi.encryptionType(i)) {
                case WIFI_AUTH_OPEN:            Serial.print("open"); break;
                case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
                case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
                case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
                case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
                case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
                case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
                case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
                case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
                default:                        Serial.print("unknown");
            }
            Serial.println();
      delay(10);
    }
  }
  Serial.println("");

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();

  // Wait a bit before scanning again.
  delay(5000);
   


}
