// Mock Pico SDK I2C API
#include "i2c_mock.h"

std::unique_ptr<i2cMock> i2cMockObj;

//Create mock functions definitions for link-time replacement
extern "C"
{
   uint i2c_init (i2c_inst_t * i2c, uint baudrate)
   {
      return i2cMockObj->i2c_init(i2c, baudrate);
   }

   int i2c_write_blocking (i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop)
   {
      return i2cMockObj->i2c_write_blocking (i2c, addr, src, len, nostop);
   }

   int i2c_read_blocking (i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop)
   {
      return i2cMockObj->i2c_read_blocking (i2c, addr, dst, len, nostop);
   }

   int i2c_read_blocking_until (i2c_inst_t * i2c, uint8_t addr, uint8_t * dst, size_t len, bool nostop, absolute_time_t until)
   {
      return i2cMockObj->i2c_read_blocking_until (i2c, addr, dst, len, nostop, until);
   }
}