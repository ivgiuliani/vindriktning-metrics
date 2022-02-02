#include <Arduino.h>
#include <WiFiManager.h>

#if !defined(SERIAL_SPEED)
#  define SERIAL_SPEED 9600
#endif

#include "vindriktning.h"

struct {
  char hostname[24];
  Vindriktning::pm25_t pm25 = 0;
} global_state;

void print_startup_header() {
  #ifdef ESP32
  Serial.printf("versions: core=%s\n", esp_get_idf_version());
  #else
  Serial.printf("versions: core=%s boot=%u\n", ESP.getCoreVersion().c_str(), ESP.getBootVersion());
  Serial.printf("boot mode: %u\n", ESP.getBootMode());
  Serial.printf("cpu freq: %uMHz\n", ESP.getCpuFreqMHz());
  Serial.printf("reset reason: %s\n", ESP.getResetReason().c_str());
  Serial.printf("hostname: %s\n", global_state.hostname);
  #endif

  Serial.println(F("System start OK."));
}

void setup_wifi() {
  WiFiManager mgr;
  WiFi.hostname(global_state.hostname);
  mgr.setDebugOutput(false);

  // `autoconnect` blocks until we successfully connect to a wifi.
  mgr.autoConnect(global_state.hostname);

  Serial.printf("ip address: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(SERIAL_SPEED);
  Serial.setTimeout(2000);
  while(!Serial) { }

  snprintf(global_state.hostname, sizeof(global_state.hostname), "VINDRIKTNING-%X", ESP.getChipId());
  print_startup_header();

  #ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  #endif

  Vindriktning::setup();

  // This is blocking! Will not go further ahead until we can connect
  // to a wifi.
  setup_wifi();
}

void loop() {
  Vindriktning::pm25_t pm25 = Vindriktning::update();

  if (pm25 > 0) {
    global_state.pm25 = pm25;
    Serial.printf("pm2.5: %u\n", global_state.pm25);
  }
}
