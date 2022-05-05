#ifndef __WEB_H
#define __WEB_H

#include <Arduino.h>
#ifdef ESP32
# include <WebServer.h>
# include <ESPmDNS.h>
#else
# include <ESP8266WebServer.h>
# include <ESP8266mDNS.h>
# include <ESP8266HTTPUpdateServer.h>
# define WebServer ESP8266WebServer
#endif
#include <WiFiManager.h>

#include "state.h"
#include "esp_generic.h"

#include "web_index.html.h"

#define LOG_REQUEST(path) Serial.printf("[GET] " path "\n")

#define HTTP_STATUS_OK            (200)
#define HTTP_STATUS_OK_NO_CONTENT (204)
#define HTTP_STATUS_NOT_FOUND     (404)

#define METRIC(name, type) \
  "# TYPE vindriktning_" name " " type "\n" \
  "vindriktning_" name ""

#define GAUGE(name) METRIC(name, "gauge")

namespace Web {
  WebServer *server;
  struct state_t *global_state;

  #ifndef ESP32
  ESP8266HTTPUpdateServer http_ota_updater;
  #endif

  static char const *metrics_response_template =
    GAUGE("core_version") "{source=\"%s\",version=\"%s\"} 1\n"

    GAUGE("pm25") "{source=\"%s\"} %d\n"

    GAUGE("temperature") "{source=\"%s\"} %0.2f\n"
    GAUGE("humidity")    "{source=\"%s\"} %0.2f\n"
    GAUGE("pressure")    "{source=\"%s\"} %0.2f\n"

    GAUGE("system_free_heap")    "{source=\"%s\"} %d\n"
    GAUGE("system_free_sketch")  "{source=\"%s\"} %d\n"
    GAUGE("system_pm1006_state") "{source=\"%s\"} %d\n"
    GAUGE("system_bme280_state") "{source=\"%s\"} %d\n"
  ;

  inline void send_cors_headers() {
    // We don't want CORS for an air quality monitor...
    server->sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server->sendHeader(F("Access-Control-Max-Age"), F("10000"));
    server->sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server->sendHeader(F("Access-Control-Allow-Headers"), F("*"));
  }

  void handle_not_found() {
    if (server->method() == HTTP_OPTIONS) {
      // Disable CORS checks on not found as we don't have any other handler
      // for HTTP_OPTIONS
      send_cors_headers();
      server->send(HTTP_STATUS_OK_NO_CONTENT);
    } else {
      server->send(HTTP_STATUS_NOT_FOUND);
    }
  }

  void handle_root() {
    LOG_REQUEST("/");

    server->send(HTTP_STATUS_OK, "text/plain; charset=utf-8", web_index_html);
  }

  void handle_metrics_request() {
    LOG_REQUEST("/metrics");

    char response[2048];
    snprintf(response, 2048, metrics_response_template,
      global_state->hostname, ESPG::getESPVersion(),
      global_state->hostname, global_state->pm25,
      global_state->hostname, global_state->temperature,
      global_state->hostname, global_state->humidity,
      global_state->hostname, global_state->pressure,
      global_state->hostname, global_state->free_heap_size,
      global_state->hostname, global_state->free_sketch_size,
      global_state->hostname, global_state->pm1006_init_ok,
      global_state->hostname, global_state->bme280_init_ok
    );

    server->send(HTTP_STATUS_OK, "text/plain; charset=utf-8", response);
  }

  void handle_factory_reset() {
    LOG_REQUEST("/reset");

    #ifdef ESP32
      mdns_free();
    #endif

    // Clear wifi settings
    WiFi.persistent(true);
    WiFi.disconnect(true);
    ESP.restart();
  }

  void setup(struct state_t *global_state_ref) {
    server = new WebServer(80);

    #ifdef ESP32
      esp_err_t err = mdns_init();
      if (err) {
        Serial.printf("MDNS Init failed: %d\n", err);
      }
      mdns_hostname_set(global_state_ref->hostname);
    #else
      MDNS.begin(global_state_ref->hostname);
      http_ota_updater.setup(server, "/ota");
    #endif

    server->onNotFound(handle_not_found);
    server->on("/", HTTP_GET, handle_root);
    server->on("/metrics", HTTP_GET, handle_metrics_request);
    server->on("/reset", HTTP_GET, handle_factory_reset);
    server->begin();

    #ifdef ESP32
      mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    #else
      MDNS.addService("http", "tcp", 80);
    #endif

    global_state = global_state_ref;
  }

  void handle() {
    #ifndef ESP32
      MDNS.update();
    #endif

    server->handleClient();
  }
}

#endif // __WEB_H //
