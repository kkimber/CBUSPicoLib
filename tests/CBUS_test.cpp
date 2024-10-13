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

#include <cstdint>
#include <cstdlib>

// CBUS Mocks
#include "CBUS_mock.h"

#include "CBUS.h"
#include "CBUSLED.h"
#include "CBUSSwitch.h"
#include "CBUSConfig.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mocklib.h"

#include <pico/stdlib.h>

#include <queue>

using namespace std;

using testing::_;
using testing::AnyNumber;
using testing::Return;
using testing::DoAll;

//-----------------------------------------------------------------------------
// Mock CAN transport, saves all frames posted for transmission to a vector

queue<CANFrame> canRxFrames;  // Frames received off the wire into CBUS
queue<CANFrame> canTxFrames;  // Frames generated by CBUS to be sent to the wire

// Add a frame to the mock to be processed by CBUS
void mockAddRxFrame(CANFrame& frame)
{
   canRxFrames.push(frame);
}

// Are there any frames to be received off the wire
bool mockCanRxAvailable(void)
{
   return !canRxFrames.empty();
}

// Retrieve the next frame off the wire to be processed by CBUS
CANFrame mockCanRx(void)
{
   if (canRxFrames.size() > 0)
   {
      CANFrame frame = canRxFrames.front();
      canRxFrames.pop();
      return frame;
   }
   else
   { 
      // THIS SHOULD REALY BE AN ASSERT!
      CANFrame frame;
      return frame;
   }
}

// Test for frames and retrieve first frame sent by CBUS
bool mockGetCanTx(CANFrame& msg)
{
   // Are there any frames transmitted?
   if (!canTxFrames.empty())
   {
      // Retreive first frame queued
      msg = canTxFrames.front();
      canTxFrames.pop();
      return true;
   }

   // No frames available
   return false;
}

// Capture frames transmitted by CBUS (send to the wire)
bool canTxReturn {true};
bool mockCanTx(CANFrame &msg, bool rtr, bool ext, uint8_t priority)
{
   canTxFrames.push(msg);
   return canTxReturn;
}

//-----------------------------------------------------------------------------

TEST(CBUS, basic)
{
   static constexpr const auto timeout {10};
   static constexpr const auto delay {20};

   uint64_t sysTime = 0ULL;

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   // Manage system time via lambda
   EXPECT_CALL(mockPicoSdk, get_absolute_time)
       .WillRepeatedly(testing::Invoke(
        [&sysTime]() -> uint64_t {
            return sysTime * 1000; // time specified in milliseconds
        }
    ));

   // Configuration
   CBUSConfig config;
   config.EE_NVS_START = 10;    // Offset start of Node Variables
   config.EE_NUM_NVS = 10;      // Number of Node Variables
   config.EE_EVENTS_START = 20; // Offset start of Events
   config.EE_MAX_EVENTS = 10;   // Maximum number of events
   config.EE_NUM_EVS = 1;       // Number of Event Variables per event (the InEventID)
   config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);
   config.begin();

   // Create UUT - with mocked I/O interfaces
   CBUSMock cbus(config);

   // CAN Frames for sending and receiving
   CANFrame canRxFrame;
   CANFrame canTxFrame;

   // Hook get message into mock CAN transport
   EXPECT_CALL(cbus, getNextMessage)
      .WillRepeatedly(testing::Invoke(&mockCanRx));

   // Hook frame available API into mock CAN transport
   EXPECT_CALL(cbus, available)
      .WillRepeatedly(testing::Invoke(&mockCanRxAvailable));

   // Hook frame transmit capture into mock CAN transport
   EXPECT_CALL(cbus, sendMessageImpl(_,false,false,_))
      .WillRepeatedly(testing::Invoke(&mockCanTx));

   // Set SLiM and run Process, no frame available to process
   cbus.setSLiM();
   cbus.process();

   //--------------------------------------------
   // RQNPN - read parameter NPARAMS
   canRxFrame = {.data{OPC_RQNPN, 0x00, 0x00, PAR_NPARAMS}};
   mockAddRxFrame(canRxFrame);
   cbus.process();

   // No params assigned, should not return a frame
   ASSERT_FALSE(mockGetCanTx(canTxFrame));

   // Assign params and try again
   mockAddRxFrame(canRxFrame);
   CBUSParams params(config);
   cbus.setParams(params.getParams());
   cbus.process();

   // Should return a single PARAN frame with value for NPARAMS
   ASSERT_TRUE(mockGetCanTx(canTxFrame));
   ASSERT_EQ(canTxFrame.data[0], OPC_PARAN);
   ASSERT_EQ(canTxFrame.data[3], PAR_NPARAMS);
   ASSERT_EQ(canTxFrame.data[4], params.getParams()->param[PAR_NPARAMS]);

//// TEST MOCK CAN TRANSPORT ////

   // Assign params and try again
   mockAddRxFrame(canRxFrame);
   cbus.process();

   // Should return PARAN value for NPARAMS
   ASSERT_TRUE(mockGetCanTx(canTxFrame));
   ASSERT_EQ(canTxFrame.data[0], OPC_PARAN);
   ASSERT_EQ(canTxFrame.data[3], PAR_NPARAMS);
   ASSERT_EQ(canTxFrame.data[4], params.getParams()->param[PAR_NPARAMS]);

//// TEST MOCK CAN TRANSPORT ////
}

int main(int argc, char **argv)
{
   // The following line must be executed to initialize Google Mock
   // (and Google Test) before running the tests.
   ::testing::InitGoogleMock(&argc, argv);
   return RUN_ALL_TESTS();
}