/*
Based on mocklib from the SmartFilamentSensor distribution
Copyright (C) 2023 Slava Zanko

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

#include "mocklib.h"

uint i2c_init (i2c_inst_t * i2c, uint baudrate)
{
   return mockPicoSdkApi.mockPicoSdk->i2c_init(i2c, baudrate);
}

int i2c_write_blocking (i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop)
{
   return mockPicoSdkApi.mockPicoSdk->i2c_write_blocking (i2c, addr, src, len, nostop);
}

int i2c_read_blocking (i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop)
{
   return mockPicoSdkApi.mockPicoSdk->i2c_read_blocking (i2c, addr, dst, len, nostop);
}

int i2c_read_blocking_until (i2c_inst_t * i2c, uint8_t addr, uint8_t * dst, size_t len, bool nostop, absolute_time_t until)
{
   return mockPicoSdkApi.mockPicoSdk->i2c_read_blocking_until (i2c, addr, dst, len, nostop, until);
}
