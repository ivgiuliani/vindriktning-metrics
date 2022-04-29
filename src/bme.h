#ifndef __BME_H
#define __BME_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BME280.h>

#ifndef BME_DEFAULT_MEASUREMENT_DELAY
#  define BME_DEFAULT_MEASUREMENT_DELAY (15 * 1000) // seconds
#endif

namespace BME {
  typedef float temperature_c_t;
  typedef float pressure_hpa_t;
  typedef float humidity_rh_perc_t;

  Adafruit_BME280 bme; // I2C

  struct measurement_t {
    temperature_c_t temperature;
    pressure_hpa_t pressure;
    humidity_rh_perc_t humidity;
  } last_measurement;

  uint16_t measurement_delay;
  long last_update = millis();

  void setup(uint16_t measurement_delay_s = BME_DEFAULT_MEASUREMENT_DELAY) {
    Wire.begin();
    while (!bme.begin(BME280_ADDRESS_ALTERNATE)) {  
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      delay(5000);
    }

    measurement_delay = measurement_delay_s;
    bme.setSampling(
      Adafruit_BME280::MODE_FORCED, // Force reading after delayTime
      Adafruit_BME280::SAMPLING_X1, // Temperature sampling set to 1
      Adafruit_BME280::SAMPLING_X1, // Pressure sampling set to 1
      Adafruit_BME280::SAMPLING_X1, // Humidity sampling set to 1
      Adafruit_BME280::FILTER_OFF   // Filter off - immediate 100% step response
    );
  }

  measurement_t *update() {
    const long now = millis();

    if (last_update > 0 && (now - last_update) <= measurement_delay) {
      return NULL;
    }

    bme.takeForcedMeasurement();
    last_update = now;

    last_measurement = {
      .temperature = bme.readTemperature(),
      .pressure = bme.readPressure(),
      .humidity = bme.readHumidity()
    };

    return &last_measurement;
  }
};

#endif // __BME_H
