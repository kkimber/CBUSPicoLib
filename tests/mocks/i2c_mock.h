// Mock Pico SDK I2C API
#pragma once

#include <memory>
#include <gmock/gmock.h>
#include "hardware/i2c.h"

// Create the mocks
struct i2cMock
{
   MOCK_METHOD(uint, i2c_init,(i2c_inst_t* i2c, uint baudrate));

   MOCK_METHOD(int, i2c_write_blocking, (i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop));

   MOCK_METHOD(int, i2c_read_blocking, (i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop));

   MOCK_METHOD(int, i2c_read_blocking_until, (i2c_inst_t * i2c, uint8_t addr, uint8_t * dst, size_t len, bool nostop, absolute_time_t until));
};

extern std::unique_ptr<i2cMock> i2cMockObj;