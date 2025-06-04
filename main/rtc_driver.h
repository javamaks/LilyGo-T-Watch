#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t weekday;
    uint8_t month;
    uint8_t year;
} rtc_time_t;

// Только системные функции
bool rtc_set_sys_time(const rtc_time_t *time);
bool rtc_get_sys_time(rtc_time_t *time);   

#ifdef __cplusplus
}
#endif
