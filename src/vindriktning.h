#ifndef VINDRIKTNING_H
#define VINDRIKTNING_H

#if !defined(PIN_UART_RX)
#  define PIN_UART_RX /*GPIO*/4 // Default to D2 on D1 mini
#endif

// tx is unused as we only read input values
#if !defined(PIN_UART_TX)
#  define PIN_UART_TX /*GPIO*/13 // Default to D7 on D1 mini
#endif

#if defined(ESP32)
#  include <HardwareSerial.h>
#  define _Serial HardwareSerial
#else
#  include <SoftwareSerial.h>
#  define _Serial SoftwareSerial
#endif

#define PAYLOAD_SIZE 20

#include <Arduino.h>

/* Based on https://github.com/obgm/pm1006 */
namespace Vindriktning {
  typedef enum {
    VALUE_PM1  = 1,
    VALUE_PM25 = 2,
    VALUE_PM10 = 4,
  } pm1006_value_t;

  struct pm1006_data_t {
    uint16_t pm1;
    uint16_t pm25;
    uint16_t pm10;
    uint8_t valid;
  };

  #ifdef ESP32
    _Serial serial(1);
  #else
    _Serial serial(PIN_UART_RX, PIN_UART_TX);
  #endif

  const bool inline is_valid_header(size_t rx_buf_size, uint8_t *rx_buffer) {
    const bool is_valid =
      rx_buf_size > 3 &&
      rx_buffer[0] == 0x16 &&
      rx_buffer[1] == 0x11 &&
      rx_buffer[2] == 0x0b;

    return is_valid;
  }
  
  const bool inline is_valid_checksum(size_t rx_buf_size, uint8_t *rx_buffer) {
    uint8_t checksum = 0;

    while(rx_buf_size--) {
      checksum += *rx_buffer++;
    }
    return checksum == 0;
  }

  void parse(size_t rx_buf_size, uint8_t *rx_buffer, struct pm1006_data_t *result) {
    memset(result, 0, sizeof(pm1006_data_t));
    enum { DF3 = 5, DF4, DF5, DF6, DF7, DF8, DF9, DF10, DF11, DF12 };

    if (rx_buf_size > DF4) {
      result->pm25 = (rx_buffer[DF3] << 8) + rx_buffer[DF4];
      result->valid |= VALUE_PM25;
    }
    if (rx_buf_size > DF8) {
      result->pm1 = (rx_buffer[DF7] << 8) + rx_buffer[DF8];
      result->valid |= VALUE_PM1;
    }
    if (rx_buf_size > DF12) {
      result->pm10 = (rx_buffer[DF11] << 8) + rx_buffer[DF12];
      result->valid |= VALUE_PM10;
    }
  }

  void setup() {
    #ifdef ESP32
    serial.begin(9600, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);
    #else
    serial.begin(9600);
    #endif
    while(!serial) { }

    serial.setTimeout(100);

    Serial.printf("vindriktning serial(rx=%d,tx=%d) ready.\n", PIN_UART_RX, PIN_UART_TX);
  }

  void update(struct pm1006_data_t &state) {
    if (!serial.available()) return;

    uint8_t rx_buffer[PAYLOAD_SIZE];
    size_t rx_buf_size = serial.readBytes(rx_buffer, sizeof(rx_buffer));
    if (rx_buf_size <= 0) {
      return;
    }

    const bool valid_header = is_valid_header(rx_buf_size, rx_buffer);
    const bool valid_checksum = is_valid_checksum(rx_buf_size, rx_buffer);

    if (!valid_header || !valid_checksum) {
      Serial.printf("[err] header:%d checksum:%d || ", valid_header, valid_checksum);

      if (rx_buf_size > 0 && (!valid_header || !valid_checksum)) {
        Serial.printf("buf: ");
        for (uint8_t i = 0; i < rx_buf_size; i++) {
          Serial.printf("%02x ", rx_buffer[i]);
        }
        Serial.printf("\n");
      }

      return;
    }

    parse(rx_buf_size, rx_buffer, &state);
  }
}

#endif /* VINDRIKTNING_H */
