#pragma once
#include "rtc_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

void rtc_user_init(void);
bool rtc_get_time(rtc_time_t *time);               // ← из PCF8563 по I2C
void rtc_set_time(const rtc_time_t *time);         // ← записывает в PCF8563

#ifdef __cplusplus
}
#endif
