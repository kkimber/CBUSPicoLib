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

uint8_t dummyFlash[FLASH_SECTOR_SIZE] {0xFF};

void dummyFlashInit()
{
   memset(dummyFlash, 0xFF, FLASH_SECTOR_SIZE);
}

void flash_range_erase (uint32_t flash_offs, size_t count)
{
    mockPicoSdkApi.mockPicoSdk->flash_range_erase (flash_offs, count);
}

void flash_range_program (uint32_t flash_offs, const uint8_t *data, size_t count)
{
    mockPicoSdkApi.mockPicoSdk->flash_range_program (flash_offs, data, count);
}
