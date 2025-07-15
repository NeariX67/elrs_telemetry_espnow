
#if ESP32
#pragma message "ESP32 stuff happening!"
#else
#pragma message "ESP8266 stuff happening!"
#endif

#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#else
#include <ESP8266WiFi.h>
#include <espnow.h>
#endif

#include <ArduinoJson.h>
#include <EEPROM.h>

#define EEPROM_UID_ADDR 0
#define EEPROM_UID_SIZE 6

// Please use ExpressLRS Configurator Runtime Options to obtain your UID (unique MAC hashed
// from binding_phrase) Insert the six numbers between the curly brackets below

// For ESP NOW / ELRS Backpack you need to enter the UID resulting from hashing your
// secret binding phrase. This must be obtained by launching https://expresslrs.github.io/web-flasher/,
// enter your binding phrase, then make a note of the UID. Enter the 6 numbers between the commas
// Config Elrs Binding
// uint8_t UID[6] = {0,0,0,0,0,0}; // this is my UID. You have to change it to your once, should look
uint8_t UID[6] = {0, 0, 0, 0, 0, 0}; // this is my UID. You have to change it to your once (UID Input: 12345678)
// To calculate this: MD5("-DMY_BINDING_PHRASE=\""+bindingphrase+"\"")[0:6]

// Callback when data is received
#ifdef ESP32
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
#else
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
#endif
  /*
  {
    "event": "telemetry",
    "data": [...]
  }
  */

  JsonDocument doc;
  doc["event"] = "telemetry";
  JsonArray data = doc.createNestedArray("data");
  for (int i = 0; i < len; i++)
  {
    data.add(incomingData[i]);
  }
  serializeJson(doc, Serial);
}

void loadUIDFromEEPROM()
{
  EEPROM.begin(EEPROM_UID_SIZE);
  for (int i = 0; i < EEPROM_UID_SIZE; ++i)
  {
    UID[i] = EEPROM.read(EEPROM_UID_ADDR + i);
  }
  EEPROM.end();
}

void saveUIDToEEPROM()
{
  EEPROM.begin(EEPROM_UID_SIZE);
  for (int i = 0; i < EEPROM_UID_SIZE; ++i)
  {
    EEPROM.write(EEPROM_UID_ADDR + i, UID[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);
  Serial.println("Start");

  loadUIDFromEEPROM();

  // MAC address can only be set with unicast, so first byte must be even, not odd --> important for BACKPACK
  UID[0] = UID[0] & ~0x01;
  WiFi.mode(WIFI_STA);
  // Set device as a Wi-Fi Station
  // WiFi.mode(WIFI_AP_STA); // Ermöglicht AP und Station gleichzeitig
  WiFi.disconnect();
// Set a custom MAC address for the device in station mode (important for BACKPACK compatibility)
#ifdef ESP32
  esp_wifi_set_mac(WIFI_IF_STA, UID);
#else
  wifi_set_macaddr(STATION_IF, UID); //--> important for BACKPACK
#endif
// Init ESP-NOW
#ifdef ESP32
  if (esp_now_init() != ESP_OK)
  {
#else
  if (esp_now_init() != 0)
  {
#endif
    Serial.println("Error initializing ESP-NOW");
    return;
  }

#ifdef ESP32
  esp_now_register_recv_cb(OnDataRecv);
#else
  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  // Register peer
  esp_now_add_peer(UID, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  // Register a callback function to handle received ESP-NOW data
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
#endif
}

String inboundData = "";

void loop()
{
  while (Serial.available())
  {
    // Read incoming data from Serial
    String input = Serial.readStringUntil('\n');
    Serial.print("Received from Serial: ");
    Serial.println(input);

    inboundData += input;
    if (inboundData.startsWith("{") && inboundData.endsWith("}"))
    {
      // Parse the JSON data
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, inboundData);
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      String event = doc["event"];
      handleEvent(event, doc["data"]);
    }
  }
}

void handleEvent(String event, const JsonDocument &data)
{
  if (event == "configuration")
  {
    for (size_t i = 0; i < 6 && i < data.size(); ++i)
    {
      UID[i] = data[i];
    }
    saveUIDToEEPROM();
  }
  else
  {
    Serial.print("Unknown event: ");
    Serial.println(event);
  }
}