#include "cmos.h"
#include "string.h"
#include <intrin.h>

char datetime_buffer[DATE_BUFFER_SIZE];

int get_update_in_progress_flag() {
    __outbyte(CMOS_ADDRESS, 0x0A);
    return (__inbyte(CMOS_DATA) & 0x80);
}

BYTE get_RTC_register(int reg) {
    __outbyte(CMOS_ADDRESS, reg);
    return __inbyte(CMOS_DATA);
}

int bcd_to_binary(int value) {
    return ((value / 16) * 10) + (value & 0x0F);
}

char* GetCurrentDate() {
    int seconds, minutes, hours, day, month, year;
    int last_seconds, last_minutes, last_hours, last_day, last_month, last_year;
    int registerB;

    do {
        while (get_update_in_progress_flag());

        seconds = get_RTC_register(0x00);
        minutes = get_RTC_register(0x02);
        hours = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);

        last_seconds = seconds;
        last_minutes = minutes;
        last_hours = hours;
        last_day = day;
        last_month = month;
        last_year = year;

        while (get_update_in_progress_flag());

        seconds = get_RTC_register(0x00);
        minutes = get_RTC_register(0x02);
        hours = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);

    } while ((last_seconds != seconds) || (last_minutes != minutes) || (last_hours != hours) ||
        (last_day != day) || (last_month != month) || (last_year != year));

    registerB = get_RTC_register(0x0B);

    if (!(registerB & 0x04)) {
        seconds = bcd_to_binary(seconds);
        minutes = bcd_to_binary(minutes);
        hours = bcd_to_binary(hours);
        day = bcd_to_binary(day);
        month = bcd_to_binary(month);
        year = bcd_to_binary(year);
    }

    if (!(registerB & 0x02) && (hours & 0x80)) {
        hours = ((hours & 0x7F) + 12) % 24;
    }

    year += 2000;
    cl_snprintf(datetime_buffer, DATE_BUFFER_SIZE, "%02d/%02d/%04d %02d:%02d:%02d",
        day, month, year, hours, minutes, seconds);

    return datetime_buffer;
}
