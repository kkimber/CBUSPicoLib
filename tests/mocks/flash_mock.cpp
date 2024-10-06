// Mock Pico SDK Flash API
#include "flash_mock.h"

uint8_t dummyFlash[FLASH_SECTOR_SIZE] {0xFF};

flashMock* flashMockObj = nullptr;

void dummyFlashInit()
{
   memset(dummyFlash, 0xFF, FLASH_SECTOR_SIZE);
}

//Create mock functions definitions for link-time replacement
extern "C"
{
   void flash_range_erase (uint32_t flash_offs, size_t count)
   {
      if (flashMockObj == nullptr) ADD_FAILURE() << "FunctionsMock::instance == nullptr";
      flashMockObj->flash_range_erase (flash_offs, count);
   }

   void flash_range_program (uint32_t flash_offs, const uint8_t *data, size_t count)
   {
      if (flashMockObj == nullptr) ADD_FAILURE() << "FunctionsMock::instance == nullptr";
      flashMockObj->flash_range_program (flash_offs, data, count);
   }
}