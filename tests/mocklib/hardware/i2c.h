// FAKE STUB HEADER

#pragma once

#include <cstdint>
#include <cstdlib>

#include "pico/time.h"

// Fake I2C instance struct type
typedef struct i2c_inst
{
   void* addr;
} i2c_inst_t;

static i2c_inst_t i2c_1{.addr = reinterpret_cast<void*>(0x1234)};

static i2c_inst_t* i2c_default = &i2c_1;

uint i2c_init (i2c_inst_t * i2c, uint baudrate);

int i2c_write_blocking (i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop);

int i2c_read_blocking (i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop);

int i2c_read_blocking_until (i2c_inst_t *i2c, uint8_t addr, uint8_t * dst, size_t len, bool nostop, absolute_time_t until);
