/*
This file is part of the SmartFilamentSensor distribution
Copyright (c) 2023 Slava Zanko

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MOCKLIB_TIME__H
#define MOCKLIB_TIME__H

#include "stdlib.h"

typedef uint64_t absolute_time_t;

static inline uint64_t to_us_since_boot(absolute_time_t t) {
    return t;
}

static inline uint32_t us_to_ms(uint64_t us) {
    if (us >> 32u) {
        return (uint32_t)(us / 1000u);
    } else {
        return ((uint32_t)us) / 1000u;
    }
}

static inline uint32_t to_ms_since_boot(absolute_time_t t) {
    uint64_t us = to_us_since_boot(t);
    return us_to_ms(us);
}

absolute_time_t get_absolute_time(void);

int64_t absolute_time_diff_us(absolute_time_t from, absolute_time_t to);

#endif
