#include <Arduino.h> // Required for all code
#include <WiFi.h> // Required for all wifi code
#include <esp_wifi.h>
#include <esp_wifi_types.h>

typedef struct {
  unsigned frame_ctrl:16;
  unsigned duration_id:16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl:16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

const char* wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type)
{
  switch(type) {
    case WIFI_PKT_MGMT: return "MGMT";
    case WIFI_PKT_DATA: return "DATA";
    default:  
      case WIFI_PKT_MISC: return "MISC";
  }
}

void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
    // data!
    const wifi_promiscuous_pkt_t* raw = (wifi_promiscuous_pkt_t*)buf;
    const wifi_ieee80211_packet_t* pkt = (wifi_ieee80211_packet_t*)raw->payload;
    const wifi_ieee80211_mac_hdr_t* hdr = &(pkt->hdr);

    // TODO
    // my MAC address is 34:cd:b0:3b:c0:f8
    if(hdr->addr1[0] == 0x34 &&
      hdr->addr1[1] == 0xcd &&
      hdr->addr1[2] == 0xb0 &&
      hdr->addr1[3] == 0x3b &&
      hdr->addr1[4] == 0xc0 &&
      hdr->addr1[5] == 0xf8){

      printf("PACKET TYPE=%s, CHAN=%02d, RSSI=%02d, BANDWIDTH=%02d, PHY=%02d,"
      " ADDR1=%02x:%02x:%02x:%02x:%02x:%02x:%02x,:%02x,"
      " ADDR2=%02x:%02x:%02x:%02x:%02x:%02x:%02x,:%02x,"
      " ADDR3=%02x:%02x:%02x:%02x:%02x:%02x:%02x,:%02x\n",
      wifi_sniffer_packet_type2str(type),
      raw->rx_ctrl.channel,
      raw->rx_ctrl.rssi,
      raw->rx_ctrl.cwb, 
      raw->rx_ctrl.sig_mode,
      /* ADDR1 */
      hdr->addr1[0],hdr->addr1[1],hdr->addr1[2],
      hdr->addr1[3],hdr->addr1[4],hdr->addr1[5],
      /* ADDR2 */
      hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
      hdr->addr2[3],hdr->addr2[4],hdr->addr2[5],
      /* ADDR3 */
      hdr->addr3[0],hdr->addr3[1],hdr->addr3[2],
      hdr->addr3[3],hdr->addr3[4],hdr->addr3[5]
    );
    }
    else{
      return; // return if there is no match
    }
}


void setup()
{
    Serial.begin(115200);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Pick the channel to scan on
    // Set to channel 1 by default, you should change this though
    esp_wifi_set_channel(5, WIFI_SECOND_CHAN_NONE); //updated to channel 5 

    // Start promiscuous scanning. Callback function will be called for each packet received
    esp_wifi_set_promiscuous_rx_cb(&sniffer_callback); 
    esp_wifi_set_promiscuous(true); 

    Serial.println("Setup done");
}
void wifiScanTest(){
  Serial.println("Starting Scan");

    int numNetworks = WiFi.scanNetworks();
    Serial.println("Completed Scan");

    if(numNetworks == 0){
        Serial.println("No Networks Found!");
    }
     else{
        Serial.print(numNetworks); // print the # of networks
     }      
}
void loop(){
  //doing nothing here
}


