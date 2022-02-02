#ifndef __WEB_H
#define __WEB_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "state.h"

#define LOG_REQUEST(path) Serial.printf("[GET] " path "\n")

namespace Web {
  ESP8266WebServer *server;
  struct state_t *global_state;

  inline void send_cors_headers() {
    // We don't want CORS for an air quality monitor...
    server->sendHeader("Access-Control-Allow-Origin", "*");
    server->sendHeader("Access-Control-Max-Age", "10000");
    server->sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    server->sendHeader("Access-Control-Allow-Headers", "*");
  }

  void handle_not_found() {
    if (server->method() == HTTP_OPTIONS) {
      // Disable CORS checks on not found as we don't have any other handler
      // for HTTP_OPTIONS
      send_cors_headers();
      server->send(204);
    }
  }

  void handle_metrics_request() {
    LOG_REQUEST("/metrics");

    static char const *response_template =
      "# TYPE vindriktning_core_version gauge\n"
      "vindriktning_core_version{source=\"%s\",version=\"%s\"} 1\n"
      
      "# TYPE vindriktning_boot_version gauge\n"
      "vindriktning_boot_version{source=\"%s\"} %u\n"

      "# TYPE vindriktning_pm25 gauge\n"
      "vindriktning_pm25{source=\"%s\"} %d\n";

    char response[1024];
    snprintf(response, 1024, response_template,
      global_state->hostname, ESP.getCoreVersion().c_str(),
      global_state->hostname, ESP.getBootVersion(),
      global_state->hostname, global_state->pm25);

    server->send(200, "text/plain; charset=utf-8", response);
  }

  void handle_factory_reset() {
    LOG_REQUEST("/reset");

    // Clear wifi settings
    WiFi.persistent(true);
    WiFi.disconnect(true);
    ESP.restart();
  }

  void setup(struct state_t *global_state_ref) {
    server = new ESP8266WebServer(80);
    server->onNotFound(handle_not_found);
    server->on("/metrics", HTTP_GET, handle_metrics_request);
    server->on("/reset", HTTP_GET, handle_factory_reset);
    server->begin();

    global_state = global_state_ref;
  }

  void handle() {
    server->handleClient();
  }
}

#endif // __WEB_H //
