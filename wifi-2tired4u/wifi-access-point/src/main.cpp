#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Set these to your desired credentials.
const char *ssid = "LilasAP";
const char *password = "";

void setup()
{
    Serial.begin(115200);
    //TODO
     Serial.println("Configuring access point...");
    if (!WiFi.softAP(ssid, password)) {
        log_e("Soft AP creation failed.");
        while (1);
    }
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

  Serial.println("Server started");
}

void printMacAddr(wifi_sta_info_t client){
    Serial.printf("MAC= %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", 
        client.mac[0],
        client.mac[1],
        client.mac[2],
        client.mac[3],
        client.mac[4],
        client.mac[5]);
}
void get_client_details() {
    wifi_sta_list_t clients;
   
    esp_wifi_ap_get_sta_list(&clients);
    Serial.printf("Connected clients: %d\n", clients.num);

    // loop through list of connected clients and print the details

    for(int i= 0; i < clients.num; i++){
        //gather info per client
        wifi_sta_info_t clientInfo = clients.sta[i];
        printMacAddr(clientInfo);
        Serial.printf("RSSI=%d\n", clientInfo.rssi);
        Serial.printf("11b: %d\n",clientInfo.phy_11b);
        Serial.printf("11g: %d\n",clientInfo.phy_11g);
        Serial.printf("11n: %u\n",clientInfo.phy_11n);
    }
}

void loop()
{
    get_client_details();
    delay(1000);
}

