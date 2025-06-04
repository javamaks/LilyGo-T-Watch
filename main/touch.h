#pragma once
#include "touch.h"

#ifdef __cplusplus
extern "C" {
#endif

void read_touch();
void i2c_touch_bus_init(void);
void i2c_touch_init();

#ifdef __cplusplus
}
#endif
