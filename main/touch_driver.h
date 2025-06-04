#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool touch_init();
uint8_t touch_get_data(int16_t *x, int16_t *y, uint8_t point_num);

#ifdef __cplusplus
}
#endif
