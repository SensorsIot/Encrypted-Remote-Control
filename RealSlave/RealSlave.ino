/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Slave >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <AES128.h>

#define CHANNEL 0
#define PAIRPIN 12

// int addr = 0;
#define EEPROM_SIZE 64
#define MAGIC 55

typedef struct counterStruc {
  byte magic;
  unsigned long counter;
} ;
unsigned long receivedCounter;
counterStruc  rollingCounter;

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESPNow/Basic/Slave Example");
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);


  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  // read EEPROM
  EEPROM.get(0, rollingCounter);
  Serial.println(rollingCounter.magic);
  if (rollingCounter.magic != MAGIC) {
    rollingCounter.magic = MAGIC;
    EEPROM.put(0, rollingCounter);
    Serial.println ("...................Please pair..................");
    EEPROM.commit();
  }

  pinMode(PAIRPIN, INPUT_PULLUP);
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println();
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);

  // Decrypt
  for (int i = 0; i < 50; i++) ciphertext[i] = 0;  // clear input file
  for (int i = 0; i < 16; i++) dec_iv[i] = 0;  // clear internal variable
  int ii = 0;
  for (int i = 0; i < 50; i++) {  // fill data into ciphertext (maybe not needed)
    if (data[i] == 0) break;
    ciphertext[i] = data[i];
    Serial.print(ciphertext[i]);
    ii++;
  } Serial.println();
  String decrypted = decrypt(ciphertext, ii, dec_iv); // Decryption
  receivedCounter = atol(decrypted.c_str());  // create unsigned long variable

  Serial.print("ReceivedCounter: ");
  Serial.print(receivedCounter);

  // Compare received with stored counter
  Serial.print(" RollingCounter ");
  Serial.print(rollingCounter.counter);
  if (receivedCounter > rollingCounter.counter) {
    Serial.println(" Code valid");
    rollingCounter.counter = receivedCounter;
    EEPROM.put(0, rollingCounter);
    EEPROM.commit();
  }
  else Serial.println(" Code not valid");

}

void loop() {
  if (digitalRead(PAIRPIN) == 0) {
    Serial.println("Pairing Key");
    rollingCounter.counter = receivedCounter;
    EEPROM.put(0, rollingCounter);
    EEPROM.commit();
    Serial.println(rollingCounter.counter);
  }
}
