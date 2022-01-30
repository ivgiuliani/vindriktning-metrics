#include <Arduino.h>

#if !defined(SERIAL_SPEED)
#  define SERIAL_SPEED 9600
#endif

void print_startup_header() {
  Serial.printf("versions: core=%s boot=%u\n", ESP.getCoreVersion().c_str(), ESP.getBootVersion());
  Serial.printf("boot mode: %u\n", ESP.getBootMode());
  Serial.printf("cpu freq: %uMHz\n", ESP.getCpuFreqMHz());
  Serial.printf("reset reason: %s\n", ESP.getResetReason().c_str());

  Serial.println(F("System start OK."));
}

void setup() {
  Serial.begin(SERIAL_SPEED);
  Serial.setTimeout(2000);
  while(!Serial) { }

  print_startup_header();
}

void loop() {
  delay(100);
}
