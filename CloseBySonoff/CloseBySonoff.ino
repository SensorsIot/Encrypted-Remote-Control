/*
    Close By example for the Approximate Library
    -
    Find the MAC address of close by devices - and trigger ARRIVE & DEPART events
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020

    Example documented here > https://github.com/davidchatting/Approximate/tree/master#when-were-close-using-a-proximate-device-handler
*/

#include <Approximate.h>
#include <credentials.h>
Approximate approx;


String macaddress = "4A:CD:D5:A3:D4:AE";

//Define for your board, not all have built-in LED and/or button:
#if defined(ESP8266)
const int LED_PIN = 12;
#elif defined(ESP32)
const int LED_PIN = 2;
#endif

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Begin");

  if (approx.init(mySSID, myPASSWORD)) {
    approx.setProximateDeviceHandler(onProximateDevice, APPROXIMATE_PERSONAL_RSSI, 5000);
    approx.begin();
  }
}

void loop() {
  approx.loop();
}

void onProximateDevice(Device *device, Approximate::DeviceEvent event) {
  switch (event) {
    case Approximate::ARRIVE:
      Serial.println("ARRIVE\t" + device->getMacAddressAsString());
      Serial.println(device->getRSSI());
      if (device->getMacAddressAsString() == macaddress) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("**ARRIVE\t" + device->getMacAddressAsString());
      }
      break;
    case Approximate::DEPART:
      Serial.println("DEPART\t" + device->getMacAddressAsString());
      Serial.println(device->getRSSI());
      if (device->getMacAddressAsString() == macaddress) {
        digitalWrite(LED_PIN, LOW);
        Serial.println("**DEPART\t" + device->getMacAddressAsString());
      }
      break;
  }
}
