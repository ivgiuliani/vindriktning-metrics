#include <Arduino.h>

#if !defined(SERIAL_SPEED)
#  define SERIAL_SPEED 9600
#endif

#include "vindriktning.h"

Vindriktning::pm1006_data_t state;

void print_startup_header() {
  #ifdef ESP32
  Serial.printf("versions: core=%s\n", esp_get_idf_version());
  #else
  Serial.printf("versions: core=%s boot=%u\n", ESP.getCoreVersion().c_str(), ESP.getBootVersion());
  Serial.printf("boot mode: %u\n", ESP.getBootMode());
  Serial.printf("cpu freq: %uMHz\n", ESP.getCpuFreqMHz());
  Serial.printf("reset reason: %s\n", ESP.getResetReason().c_str());
  #endif

  Serial.println(F("System start OK."));
}

void setup() {
  Serial.begin(SERIAL_SPEED);
  Serial.setTimeout(2000);
  while(!Serial) { }

  print_startup_header();

  #ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  #endif

  Vindriktning::setup();
}

void loop() {
  Vindriktning::update(state);

  if (state.valid & Vindriktning::VALUE_PM1)
    Serial.printf("pm1: %u\n", state.pm1);
  if (state.valid & Vindriktning::VALUE_PM25)
    Serial.printf("pm2.5: %u\n", state.pm25);
  if (state.valid & Vindriktning::VALUE_PM10)
    Serial.printf("pm10: %u\n", state.pm10);
}
