// Mock Pico SDK Flash API
#pragma once

#include <memory>
#include <gmock/gmock.h>
#include "hardware/flash.h"

void dummyFlashInit(void);

// Create the mocks
struct flashMock
{
   MOCK_METHOD2(flash_range_erase,void(uint32_t flash_offs, size_t count));

   MOCK_METHOD3(flash_range_program,void(uint32_t flash_offs, const uint8_t *data, size_t count));
};

extern flashMock* flashMockObj;