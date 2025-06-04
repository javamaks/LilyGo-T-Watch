#include "rtc_driver.h"
#include <time.h>
#include <sys/time.h>

bool rtc_set_sys_time(const rtc_time_t *time)
{
    struct tm t = {
        .tm_sec  = time->seconds,
        .tm_min  = time->minutes,
        .tm_hour = time->hours,
        .tm_mday = time->day,
        .tm_mon  = time->month - 1,
        .tm_year = time->year + 100,
    };
    time_t now = mktime(&t);
    if (now == -1) return false;

    struct timeval tv = {
        .tv_sec = now,
        .tv_usec = 0,
    };
    return settimeofday(&tv, NULL) == 0;
}

bool rtc_get_sys_time(rtc_time_t *time)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *t = localtime(&tv.tv_sec);
    if (!t) return false;

    time->seconds = t->tm_sec;
    time->minutes = t->tm_min;
    time->hours   = t->tm_hour;
    time->day     = t->tm_mday;
    time->month   = t->tm_mon + 1;
    time->year    = t->tm_year - 100;

    return true;
}
