/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <EEPROM.h>
#include <AES128.h>

struct {
  byte magic;
  unsigned long counter;
} savedCounter;

#define EEPROM_SIZE 64

#define MAGIC 55

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0x4E, 0xD8, 0xD1};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  String d;
  bool e;
} struct_message;

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

void sendData(unsigned long _inputNumber) {
  //  const uint8_t *peer_addr = slave.peer_addr;
  // Encrypt
  for (int i = 0; i < 16; i++) enc_iv[i] = 0;
  itoa(_inputNumber, cleartext, 10);
  uint16_t messLen = String(cleartext).length();
  for (int i = messLen + 1; i < 16; i++) cleartext[i] = 0; // patting to 16 bytes
  Serial.print("messLen: "); Serial.println(messLen);

  for (int i = 0; i < 16; i++) {
    Serial.print(cleartext[i]); Serial.print("/");
  }
  Serial.println();
  String encrypted = encrypt(cleartext, messLen, enc_iv);
  sprintf(ciphertext, "%s", encrypted.c_str());

  Serial.print("Sending: ");
  esp_now_send(broadcastAddress, (uint8_t *) &ciphertext, 32);
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, savedCounter);
  Serial.println(savedCounter.magic);
  if (savedCounter.magic != MAGIC) {
    savedCounter.magic = MAGIC;
    savedCounter.counter = 0;
    EEPROM.put(0, savedCounter);
    Serial.println ("...................Please pair..................");
    EEPROM.commit();
  }
  cryptoInit();
}

void loop() {
  savedCounter.counter++;
  EEPROM.put(0, savedCounter);
  EEPROM.commit();
  Serial.println(savedCounter.counter);
  // savedCounter.counter=101;
  sendData(savedCounter.counter);
  delay(1000);
}
