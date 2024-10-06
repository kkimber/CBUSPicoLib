// FAKE STUB HEADER

#pragma once

#include <cstdint>
#include <cstdlib>

static constexpr const uint16_t FLASH_SECTOR_SIZE {1u << 12};

// Provide faked single sector of flash that can be read/written
extern uint8_t dummyFlash[FLASH_SECTOR_SIZE];

static const uintptr_t XIP_BASE = reinterpret_cast<uintptr_t>(&dummyFlash[0]);

static constexpr const auto PICO_FLASH_SIZE_BYTES {FLASH_SECTOR_SIZE};

extern "C"
{
void flash_range_erase (uint32_t flash_offs, size_t count);

void flash_range_program (uint32_t flash_offs, const uint8_t *data, size_t count);
}