/*
   CBUS Module Library - RasberryPi Pico SDK port
   Copyright (c) Kevin Kimber 2023

   Based on work by Duncan Greenwood
   Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

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
#include "SystemTickFake.h"
#include "flash_mock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <pico/stdlib.h>

#include <CBUSLED.h>
#include <CBUSSwitch.h>

using testing::_;
using testing::AnyNumber;

TEST(CBUSConfig, basic)
{
   static constexpr const auto canID {2};
   static constexpr const auto nodeID {3};
   static constexpr const auto flimMode {true};
   static constexpr const auto minCanID {1};
   static constexpr const auto maxCanID {99};

   std::unique_ptr<flashMock> myFlashMock (new flashMock);
   flashMockObj = myFlashMock.get();

   EXPECT_CALL(*myFlashMock.get(), flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(*myFlashMock.get(), flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

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

}

TEST(CBUSConfig, events)
{
   // Flash mock
   std::unique_ptr<flashMock> myFlashMock (new flashMock);
   flashMockObj = myFlashMock.get();

   EXPECT_CALL(*myFlashMock.get(), flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(*myFlashMock.get(), flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

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

   // Check down the hash table
   config.clearEvHashTable();
}

TEST(CBUSConfig, nodeVars)
{
   // Flash mock
   std::unique_ptr<flashMock> myFlashMock (new flashMock);
   flashMockObj = myFlashMock.get();

   EXPECT_CALL(*myFlashMock.get(), flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(*myFlashMock.get(), flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

   dummyFlashInit();

   CBUSConfig config;
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
   // Flash mock
   std::unique_ptr<flashMock> myFlashMock (new flashMock);
   flashMockObj = myFlashMock.get();

   EXPECT_CALL(*myFlashMock.get(), flash_range_program(_,_,_)).Times(AnyNumber());
   EXPECT_CALL(*myFlashMock.get(), flash_range_erase(0, FLASH_SECTOR_SIZE)).Times(AnyNumber());

   dummyFlashInit();

   CBUSConfig config;
   config.setEEPROMtype(EEPROM_TYPE::EEPROM_USES_FLASH);

   CBUSLED green;
   CBUSLED yellow;
   CBUSSwitch sw;

   config.resetModule();
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