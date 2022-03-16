#ifndef __VINDRIKTNING_H
#define __VINDRIKTNING_H

#if !defined(PIN_UART_RX)
#  define PIN_UART_RX /*GPIO*/0 // Default to D3 on D1 mini
#endif

// tx is unused as we only read input values
#if !defined(PIN_UART_TX)
#  define PIN_UART_TX /*GPIO*/13 // Default to D7 on D1 mini
#endif

#define PM1006_BAUD_RATE (9600)

#if defined(ESP32)
#  include <HardwareSerial.h>
#  define _Serial HardwareSerial
#else
#  include <SoftwareSerial.h>
#  define _Serial SoftwareSerial
#endif

#define PAYLOAD_SIZE 20

#include <Arduino.h>

namespace Vindriktning {
  /* http://www.jdscompany.co.kr/download.asp?gubun=07&filename=PM1006_LED_PARTICLE_SENSOR_MODULE_SPECIFICATIONS.pdf
    * Payload looks like:
    * 16 11 0B DF1 DF2 DF3 DF4 DF5 DF6 DF7 DF8 DF9 DF10 DF11 DF12 DF13 DF14 DF15 DF16[CS]
    * |header |unused | pm25  | unsupported by pm1006, but needed for checksum          |
    */
  struct __attribute__((__packed__)) pm1006_payload_t {
    uint8_t header[3];
    uint8_t _unused1[2]; // DF1..DF2
    uint8_t df3;
    uint8_t df4;
    uint8_t _unused2[12]; // DF5..DF16
    uint8_t _cs; // CS
  };

  typedef int16_t pm25_t;

  #ifdef ESP32
    _Serial serial(1);
  #else
    _Serial serial(PIN_UART_RX, PIN_UART_TX);
  #endif

  const bool inline is_valid_header(struct pm1006_payload_t *payload) {
    return payload->header[0] == 0x16 &&
           payload->header[1] == 0x11 &&
           payload->header[2] == 0x0b;
  }
  
  const bool inline is_valid_checksum(struct pm1006_payload_t *payload) {
    uint8_t checksum = 0, idx = sizeof(struct pm1006_payload_t);
    const char *buf = reinterpret_cast<char*>(payload);

    while(idx--) {
      checksum += *buf++;
    }
    return checksum == 0;
  }

  void setup() {
    #ifdef ESP32
    serial.begin(PM1006_BAUD_RATE, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);
    #else
    serial.begin(PM1006_BAUD_RATE);
    #endif
    while(!serial) { }

    serial.setTimeout(1000);

    Serial.printf("vindriktning serial(rx=%d,tx=%d) ready.\n", PIN_UART_RX, PIN_UART_TX);
  }

  const pm25_t update() {
    if (!serial.available()) {
      return -1;
    }

    struct pm1006_payload_t payload;
    size_t rx_buf_size = serial.readBytes(payload.header, sizeof(pm1006_payload_t));
    if (rx_buf_size <= 0 || rx_buf_size != sizeof(pm1006_payload_t)) {
      return -1;
    }

    const bool is_valid = is_valid_header(&payload) && is_valid_checksum(&payload);
    if (!is_valid) {
      const char *rx_buffer = reinterpret_cast<char*>(&payload);
      Serial.printf("[error] buf: ");
      for (uint8_t i = 0; i < rx_buf_size; i++) {
        Serial.printf("%02x ", rx_buffer[i]);
      }
      Serial.printf("\n");

      return -1;
    }

    // According to the datasheet, "PM2.5(μg/m³)= DF3*256+DF4"
    return (payload.df3 << 8) | payload.df4;
  }
}

#endif /* __VINDRIKTNING_H */
