/*
   CBUS Module Library - RasberryPi Pico SDK port
   Copyright (c) Kevin Kimber 2024

   Based on work by Duncan Greenwood
   Copyright (c) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

   This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

#include "CBUSConfig.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <pico/stdlib.h>

#include "mocklib.h"

#include <CBUSLED.h>
#include <CBUSSwitch.h>

using testing::_;
using testing::Return;
using testing::ReturnArg;
using testing::Invoke;
using testing::AnyNumber;

/// Flash Backend

TEST(CBUSConfig, basic)
{
   static constexpr const auto canID {2};
   static constexpr const auto nodeID {3};
   static constexpr const auto flimMode {true};
   static constexpr const auto minCanID {1};
   static constexpr const auto maxCanID {99};

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(mockPicoSdk, flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

   dummyFlashInit();

   CBUSConfig config;
   config.setEEPROMtype(EEPROM_TYPE::EEPROM_USES_FLASH);

   // Set sizing params
   config.EE_NVS_START = 10;    // Offset start of Node Variables
   config.EE_NUM_NVS = 10;      // Number of Node Variables
   config.EE_EVENTS_START = 20; // Offset start of Events
   config.EE_MAX_EVENTS = 10;   // Maximum number of events
   config.EE_NUM_EVS = 1;       // Number of Event Variables per event (the InEventID)
   config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);

   // Initialize defaults
   config.begin();

   // Validate default settings
   ASSERT_EQ(config.getCANID(), 1);
   ASSERT_EQ(config.getNodeNum(), 0);
   ASSERT_EQ(config.getFLiM(), false);

   // Assign new settings
   ASSERT_TRUE(config.setCANID(canID));
   config.setNodeNum(nodeID);
   config.setFLiM(flimMode);

   // Validate new settings
   ASSERT_EQ(config.getCANID(), canID);
   ASSERT_EQ(config.getNodeNum(), nodeID);
   ASSERT_EQ(config.getFLiM(), flimMode);

   // Test CAN ID limits
   ASSERT_FALSE(config.setCANID(minCanID - 1));
   ASSERT_FALSE(config.setCANID(maxCanID + 1));

   // Free memory
   ASSERT_GT(config.freeSRAM(), 1);
}

TEST(CBUSConfig, events)
{
   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(mockPicoSdk, flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

   dummyFlashInit();

   CBUSConfig config;
   config.setEEPROMtype(EEPROM_TYPE::EEPROM_USES_FLASH);

   // Set sizing params
   config.EE_NVS_START = 10;    // Offset start of Node Variables
   config.EE_NUM_NVS = 10;      // Number of Node Variables
   config.EE_EVENTS_START = 20; // Offset start of Events
   config.EE_MAX_EVENTS = 10;   // Maximum number of events
   config.EE_NUM_EVS = 1;       // Number of Event Variables per event (the InEventID)
   config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);

   // Initialize defaults
   config.begin();

   ASSERT_EQ(config.findEventSpace(), 0);

   // Write first event
   EVENT_INFO_t evInfo;
   evInfo.eventNumber = 1;
   evInfo.nodeNumber = 10;
   config.writeEvent(0, evInfo, true);
   config.writeEventEV(0, 1, 20);

   // recreate event hash table entry
   config.updateEvHashEntry(0);

   ASSERT_EQ(config.numEvents(), 1);

   // Create max events
   for (int_fast8_t ev = 1; ev < config.EE_MAX_EVENTS; ev++)
   {
      uint8_t eventNo = config.findEventSpace();
      evInfo.eventNumber++;
      evInfo.nodeNumber++;

      config.writeEvent(eventNo, evInfo, false);
      config.writeEventEV(eventNo, 1, ev + 20);
      config.updateEvHashEntry(eventNo);

      ASSERT_EQ(config.numEvents(), ev + 1);
   }

   // Check event table full
   ASSERT_EQ(config.findEventSpace(), config.EE_MAX_EVENTS);

   // Check recorded events
   for (int_fast8_t ev = 0; ev < config.EE_MAX_EVENTS; ev++)
   {
      config.readEvent(ev, evInfo);

      ASSERT_EQ(evInfo.eventNumber, ev + 1);
      ASSERT_EQ(evInfo.nodeNumber, ev + 10);
      ASSERT_EQ(config.getEventEVval(ev, 1), ev + 20);
   }

   // Find events via hash
   for (int_fast8_t ev = 0; ev < config.EE_MAX_EVENTS; ev++)
   {
      uint8_t eventNo = config.findExistingEvent(ev + 10, ev + 1);

      ASSERT_LT(eventNo, config.EE_MAX_EVENTS);
      ASSERT_EQ(eventNo, ev);
   }

   // Clear the events
   config.clearEventsEEPROM();

   // Find events via hash - none should be found
   for (int_fast8_t ev = 0; ev < config.EE_MAX_EVENTS; ev++)
   {
      uint8_t eventNo = config.findExistingEvent(ev + 10, ev + 1);

      ASSERT_EQ(eventNo, config.EE_MAX_EVENTS);
   }

   // Check down the hash table
   config.clearEvHashTable();

   // Create new event 
   uint8_t eventNo = config.findEventSpace();
   ASSERT_EQ(eventNo, 0);
   evInfo.eventNumber = 1;
   evInfo.nodeNumber = 1;

   config.writeEvent(eventNo, evInfo, false); // no flush
   config.writeEventEV(eventNo, 1, 1);
   config.updateEvHashEntry(eventNo);

   ASSERT_EQ(config.numEvents(), 1);

   // Add a duplicate event - force hash clash
   eventNo = config.findEventSpace();
   ASSERT_EQ(eventNo, 1);
   config.writeEvent(eventNo, evInfo, true); // flush
   config.updateEvHashEntry(eventNo);

   // get hash of event #0 and #1
   uint8_t hashEv0 = config.getEvTableEntry(0);
   uint8_t hashEv1 = config.getEvTableEntry(0);
   ASSERT_EQ(hashEv0, hashEv1);

   // recreate hash table
   config.makeEvHashTable();

   // find event by hash, should return first in table
   ASSERT_EQ(config.findExistingEvent(1,1), 0);

   // attempt to get hash of invalid event
   ASSERT_EQ(config.getEvTableEntry(config.EE_MAX_EVENTS + 1), 0);
}

TEST(CBUSConfig, nodeVars)
{
   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(mockPicoSdk, flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

   dummyFlashInit();

   CBUSConfig config;
   config.EE_NVS_START = 10;    // Offset start of Node Variables
   config.EE_NUM_NVS = 10;      // Number of Node Variables
   config.EE_EVENTS_START = 20; // Offset start of Events
   config.EE_MAX_EVENTS = 10;   // Maximum number of events
   config.EE_NUM_EVS = 1;       // Number of Event Variables per event (the InEventID)
   config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);
  
   config.setEEPROMtype(EEPROM_TYPE::EEPROM_USES_FLASH);

   // Write all NV's
   for (int_fast8_t nv=0; nv < config.EE_NUM_NVS; nv++)
   {
      config.writeNV(nv, nv + 1);
   }

   // Write invalid NV
   config.writeNV(config.EE_NUM_NVS, 1);

   // Read back NV's
   for (int_fast8_t nv=0; nv < config.EE_NUM_NVS; nv++)
   {
      ASSERT_EQ(config.readNV(nv), nv + 1);
   }
}

TEST(CBUSConfig, resetModule)
{
   uint64_t sysTime = 0ULL;
   bool pinState = true; // Active LOW, initially not pressed

   static constexpr const auto pinSwitch {1};

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(mockPicoSdk, flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

   // Manage system time via lambda
   EXPECT_CALL(mockPicoSdk, get_absolute_time)
       .WillRepeatedly(testing::Invoke(
        [&sysTime]() -> uint64_t {
            return sysTime += 1000; // time specified in milliseconds
        }
    ));

   EXPECT_CALL(mockPicoSdk, gpio_get(pinSwitch))
       .WillRepeatedly(testing::Invoke(
        [&pinState]() -> bool {
            return pinState;
        }
   ));

   dummyFlashInit();

   CBUSConfig config;
   config.EE_NVS_START = 10;    // Offset start of Node Variables
   config.EE_NUM_NVS = 10;      // Number of Node Variables
   config.EE_EVENTS_START = 20; // Offset start of Events
   config.EE_MAX_EVENTS = 10;   // Maximum number of events
   config.EE_NUM_EVS = 1;       // Number of Event Variables per event (the InEventID)
   config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);

   config.setEEPROMtype(EEPROM_TYPE::EEPROM_USES_FLASH);

   // Reset no UI
   config.resetModule();

   // Reset with UI
   CBUSLED green;
   CBUSLED yellow;
   CBUSSwitch sw;
   sw.setPin(pinSwitch, false);

   // Reset - Button not pressed
   config.resetModule(green, yellow, sw);

   // Press the button
   pinState = false;

   // Reset - Button pressed
   config.resetModule(green, yellow, sw);

   // Reset flag should be set after reset module
   ASSERT_TRUE(config.isResetFlagSet());

   // Test flag clear
   config.clearResetFlag();
   ASSERT_FALSE(config.isResetFlagSet());

   // Test force set flag
   config.setResetFlag();
   ASSERT_TRUE(config.isResetFlagSet());
}

/// I2C Backend

TEST(CBUSConfig, i2cBackend)
{
   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, i2c_init(_, 100*1000)); // Init I2C 100K
   EXPECT_CALL(mockPicoSdk, gpio_set_function(_, GPIO_FUNC_I2C)).Times(2); // Set 2 pins
   EXPECT_CALL(mockPicoSdk, i2c_write_blocking(_,_,_,_,_)) // Return success 
     .WillRepeatedly(::testing::ReturnArg<3>());
   EXPECT_CALL(mockPicoSdk, i2c_read_blocking(_,_,_,_,_)) // Return success
     .WillRepeatedly(::testing::ReturnArg<3>());

   CBUSConfig config;
   config.setEEPROMtype(EEPROM_TYPE::EEPROM_EXTERNAL_I2C);
   config.setExtEEPROMAddress(1);
   config.begin();
   config.resetModule();
}

TEST(CBUSConfig, flashAPI)
{
   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(mockPicoSdk, flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

   dummyFlashInit();

   CBUSConfig config;
   config.EE_NVS_START = 10;    // Offset start of Node Variables
   config.EE_NUM_NVS = 10;      // Number of Node Variables
   config.EE_EVENTS_START = 20; // Offset start of Events
   config.EE_MAX_EVENTS = 10;   // Maximum number of events
   config.EE_NUM_EVS = 1;       // Number of Event Variables per event (the InEventID)
   config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);
  
   config.setEEPROMtype(EEPROM_TYPE::EEPROM_USES_FLASH);

   config.begin();

   static constexpr const auto NUM_BYTES = 8;
   uint8_t writeBytes[NUM_BYTES] = {1,2,3,4,5,6,7,8};
   config.writeBytesEEPROM(0, writeBytes, NUM_BYTES);

   uint8_t readBytes[NUM_BYTES] = {};
   config.readBytesEEPROM(0, NUM_BYTES, readBytes);

   ASSERT_EQ(memcmp(readBytes, writeBytes, NUM_BYTES), 0);

   // Read beyond allocated flags
   ASSERT_EQ(config.getChipEEPROMVal(FLASH_SECTOR_SIZE + 1), 0xFF);
}

// Manage simple I2C write/read
uint8_t saveData = {};

int writeI2C(i2c_inst_t*, uint8_t, const uint8_t* data, size_t len, bool)
{
   if (len == 2) saveData = data[1];
   return len;
}

int readI2C(i2c_inst_t *, uint8_t, uint8_t * data, size_t len, bool, absolute_time_t)
{
   *data = saveData;
   return len;
}

TEST(CBUSConfig, i2cAPI)
{
   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;
   
   EXPECT_CALL(mockPicoSdk, i2c_init(_, 100*1000)); // Init I2C 100K
   EXPECT_CALL(mockPicoSdk, gpio_set_function(_, GPIO_FUNC_I2C)).Times(2); // Set 2 pins
   EXPECT_CALL(mockPicoSdk, i2c_write_blocking(_,_,_,_,_)) 
     .WillOnce(Return(0))                                 // Fail first call
     .WillRepeatedly(Invoke(writeI2C));
   EXPECT_CALL(mockPicoSdk, i2c_read_blocking(_,_,_,_,_))
     .WillRepeatedly(ReturnArg<3>());
   EXPECT_CALL(mockPicoSdk, i2c_read_blocking_until(_,_,_,_,_,_))
     .WillRepeatedly(Invoke(readI2C));

   CBUSConfig config;
   config.EE_NVS_START = 10;    // Offset start of Node Variables
   config.EE_NUM_NVS = 10;      // Number of Node Variables
   config.EE_EVENTS_START = 20; // Offset start of Events
   config.EE_MAX_EVENTS = 10;   // Maximum number of events
   config.EE_NUM_EVS = 1;       // Number of Event Variables per event (the InEventID)
   config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);

   // Should fail to init I2C and revert to flash - as 1st i2c_write_blocking fails
   ASSERT_FALSE(config.setEEPROMtype(EEPROM_TYPE::EEPROM_EXTERNAL_I2C));

   // Should pass on second I2C init attempt
   ASSERT_TRUE(config.setEEPROMtype(EEPROM_TYPE::EEPROM_EXTERNAL_I2C));

   config.begin();

   // Mock I2C responses will only work for single byte transfer!!
   static constexpr const auto NUM_BYTES = 1;
   uint8_t writeBytes[NUM_BYTES] = {0xAB};
   config.writeBytesEEPROM(0, writeBytes, NUM_BYTES);

   uint8_t readBytes[NUM_BYTES] = {};
   config.readBytesEEPROM(0, NUM_BYTES, readBytes);

   ASSERT_EQ(memcmp(readBytes, writeBytes, NUM_BYTES), 0);
}

int main(int argc, char **argv)
{
   // Need to init the flash mock
   dummyFlashInit();

   // The following line must be executed to initialize Google Mock
   // (and Google Test) before running the tests.
   ::testing::InitGoogleMock(&argc, argv);
   return RUN_ALL_TESTS();
}