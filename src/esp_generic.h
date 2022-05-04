#ifndef __ESP_GENERIC_H
#define __ESP_GENERIC_H

#include <Arduino.h>

/*
 * Abstraction over framework methods that differ between ESP32 and ESP8266.
 */
namespace ESPG {
  static inline const char *chip_type() {
    #ifdef ESP32
      return "esp32";
    #else
      return "esp8266";
    #endif
  }

  static inline const uint32_t chip_id() {
    uint32_t chip_id = 0;
    #ifdef ESP32
      for(int32_t i = 0; i < 17; i = i + 8) {
        chip_id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
      }
    #else
      chip_id = ESP.getChipId();
    #endif
    return chip_id;
  }

  static inline const String getESPVersion() {
    #ifdef ESP32
      return String(esp_get_idf_version());
    #else
      return String(ESP.getCoreVersion().c_str());
    #endif
  }

  static inline const uint32_t getFreeHeapSize() {
    #ifdef ESP32
      return ESP.getFreeHeap();
    #else
      return system_get_free_heap_size();
    #endif
  }

  static inline const uint32_t getFreeSketchSpace() {
    return ESP.getFreeSketchSpace();
  }
};

#endif
