#ifndef __STATE__H
#define __STATE_H

#pragma once

#include "bme.h"
#include "vindriktning.h"

struct state_t {
  char hostname[24];
  Vindriktning::pm25_t pm25 = 0;
  BME::temperature_c_t temperature = 0;
  BME::pressure_hpa_t pressure = 0;
  BME::humidity_rh_perc_t humidity = 0;
};

#endif // __STATE_H
