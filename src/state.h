#ifndef __STATE__H
#define __STATE_H

#pragma once

#include "vindriktning.h"

struct state_t {
  char hostname[24];
  Vindriktning::pm25_t pm25 = 0;
};

#endif // __STATE_H
