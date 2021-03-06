#include <Arduino.h>
#include <WiFiManager.h>

#include "esp_generic.h"

#if !defined(SERIAL_SPEED)
#  define SERIAL_SPEED 9600
#endif

#include "state.h"
#include "web.h"
#include "vindriktning.h"

struct state_t global_state;

void print_startup_header() {
  Serial.printf("versions: type=%s core=%s\n", ESPG::chip_type(), ESPG::getESPVersion().c_str());
  #ifndef ESP32
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

  snprintf(global_state.hostname, sizeof(global_state.hostname), "VINDRIKTNING-%X", ESPG::chip_id());
  print_startup_header();

  #ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  #endif

  global_state.pm1006_init_ok = Vindriktning::setup();
  global_state.bme280_init_ok = BME::setup();

  // This is blocking! Will not go further ahead until we can connect
  // to a wifi.
  setup_wifi();

  Web::setup(&global_state);
  Serial.println(F("post wifi init ok."));
}

void loop() {
  if (global_state.pm1006_init_ok) {
    Vindriktning::pm25_t pm25 = Vindriktning::update();
    if (pm25 > 0) {
      global_state.pm25 = pm25;
      Serial.printf("pm2.5: %u\n", global_state.pm25);
    }
  }

  if (global_state.bme280_init_ok) {
    BME::measurement_t *bme_measurement = BME::update();
    if (bme_measurement != NULL) {
      global_state.temperature = bme_measurement->temperature;
      global_state.pressure = bme_measurement->pressure;
      global_state.humidity = bme_measurement->humidity;

      Serial.printf("temperature=%.02fC  pressure=%.02fhPa  humidity=%.02f%%RH\n",
        bme_measurement->temperature,
        bme_measurement->pressure,
        bme_measurement->humidity);
    }
  }

  global_state.free_heap_size = ESPG::getFreeHeapSize();
  global_state.free_sketch_size = ESPG::getFreeSketchSpace();

  Web::handle();
}
