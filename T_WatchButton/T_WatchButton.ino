
#include "config.h"
#include "ESP_Now_header.h"


TTGOClass *ttgo;

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_CLICKED) {
    Serial.printf("Clicked\n");
    sendData();
  } else if (event == LV_EVENT_VALUE_CHANGED) {
    Serial.printf("Toggled\n");
  }
}

void setup()
{
  Serial.begin(115200);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->lvgl_begin();

  lv_obj_t *label;

  lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_event_cb(btn1, event_handler);
  lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, 0);

  label = lv_label_create(btn1, NULL);
  lv_label_set_text(label, "Open");

/*  
  lv_obj_t *btn2 = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_event_cb(btn2, event_handler);
  lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 0, 40);
  lv_btn_set_checkable(btn2, true);
  lv_btn_toggle(btn2);
  lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);

  label = lv_label_create(btn2, NULL);
  lv_label_set_text(label, "Pair");
  */

  //***** ESP_Now ***
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  Serial.println("ESPNow / Basic / Master Example");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  EEPROM.get(0, savedCounter);
  Serial.println(savedCounter.magic);
  if (savedCounter.magic != MAGIC) {
    savedCounter.magic = MAGIC;
    EEPROM.put(0, savedCounter);
    Serial.println ("...................Please pair..................");
    EEPROM.commit();
  }
  cryptoInit();

}

void loop()
{
  lv_task_handler();
  delay(5);
}
