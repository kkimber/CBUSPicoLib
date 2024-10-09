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

#include "CBUSCircularBuffer.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <pico/stdlib.h>

#include "mocklib.h"

using testing::_;
using testing::Return;

// Uninitialized usage test
TEST(CBUSCircularBuffer, noInit)
{
   static constexpr const auto numItems {0};

   CBUSCircularBuffer buffer(numItems);
   CANFrame frame;
   buffer.put(frame);

   ASSERT_EQ(buffer.free_slots(), numItems);
   ASSERT_EQ(buffer.get(), nullptr);
   ASSERT_EQ(buffer.peek(), nullptr);
   ASSERT_EQ(buffer.insert_time(), 0);
}

// Initialization test
TEST(CBUSCircularBuffer, init)
{
   static constexpr const auto numItems {10};

   CBUSCircularBuffer buffer(numItems);

   ASSERT_EQ(buffer.size(), 0);
   ASSERT_TRUE(buffer.empty());
   ASSERT_FALSE(buffer.full());
}

// Simplistic test
TEST(CBUSCircularBuffer, basicUsage)
{
   static constexpr const auto numItems {1};
   static constexpr const auto frameID {1};
   static constexpr const auto sysTime {1234};

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, get_absolute_time()).WillRepeatedly(Return(sysTime));

   CBUSCircularBuffer buffer(numItems);
   CANFrame frame;
   frame.id = frameID;

   ASSERT_EQ(buffer.peek(), nullptr);

   buffer.put(frame);

   ASSERT_EQ(buffer.size(), 1);
   ASSERT_TRUE(buffer.full());
   ASSERT_TRUE(buffer.available());

   // Peek stored frame
   CANFrame* peekFrame = buffer.peek();
   ASSERT_EQ(peekFrame->id, frameID);
   ASSERT_EQ(buffer.size(), 1);
   ASSERT_EQ(buffer.insert_time(), sysTime);

   // Clear the buffer
   buffer.clear();
   ASSERT_EQ(buffer.size(), 0);
   ASSERT_EQ(buffer.free_slots(), numItems);
   ASSERT_FALSE(buffer.full());
   ASSERT_TRUE(buffer.empty());

   // Insert frame
   buffer.put(frame);
   ASSERT_EQ(buffer.size(), 1);

   // Remove frame
   CANFrame* gotFrame = buffer.get();
   ASSERT_EQ(gotFrame->id, frameID);
   ASSERT_EQ(buffer.size(), 0);
   
   ASSERT_EQ(buffer.peek(), nullptr);

   ASSERT_TRUE(buffer.empty());
   ASSERT_EQ(buffer.puts(), 2);
   ASSERT_EQ(buffer.gets(), 1);
}

// Advanced usage test
TEST(CBUSCircularBuffer, advancedUsage)
{
   static constexpr const auto numItems {10};

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, get_absolute_time()).WillRepeatedly(Return(0));

   CBUSCircularBuffer buffer(numItems);
   CANFrame frame;

   // Fill the buffer
   for (auto i=0; i < numItems; i++)
   {
      frame.id = i;
      buffer.put(frame);
      ASSERT_EQ(buffer.size(), i + 1);
      ASSERT_EQ(buffer.highWaterMark(), i + 1);
   }

   ASSERT_TRUE(buffer.full());
   ASSERT_TRUE(buffer.available());

   // Empty the buffer
   for (auto i=0; i < numItems; i++)
   {
      CANFrame* gotFrame = buffer.get();
      ASSERT_EQ(gotFrame->id, i);
      ASSERT_EQ(buffer.size(), numItems - i - 1);
   }

   ASSERT_TRUE(buffer.empty());
   ASSERT_EQ(buffer.puts(), numItems);
   ASSERT_EQ(buffer.gets(), numItems);
   ASSERT_EQ(buffer.highWaterMark(), numItems);
}

// Overflow test
TEST(CBUSCircularBuffer, overflow)
{
   static constexpr const auto numItems {2};
   static constexpr const auto frameID1 {1};
   static constexpr const auto frameID2 {2};
   static constexpr const auto frameID3 {3};

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, get_absolute_time()).WillRepeatedly(Return(0));

   CBUSCircularBuffer buffer(numItems);
   CANFrame frame;

   // Insert first frame
   frame.id = frameID1;
   buffer.put(frame);

   ASSERT_EQ(buffer.size(), 1);
   ASSERT_TRUE(buffer.available());

   // Insert second frame
   frame.id = frameID2;
   buffer.put(frame);

   ASSERT_EQ(buffer.size(), 2);
   ASSERT_TRUE(buffer.available());
   ASSERT_TRUE(buffer.full());

   // Peek first available frame, should be the first inserted
   CANFrame* peekFrame = buffer.peek();

   ASSERT_EQ(peekFrame->id, frameID1); 
   ASSERT_EQ(buffer.size(), 2);

   // Overflow the buffer size, oldest item is overwritten
   frame.id = frameID3;
   buffer.put(frame);

   ASSERT_EQ(buffer.size(), 2);
   ASSERT_EQ(buffer.overflows(), 1);

   // Peek first available frame, should now be the second inserted
   peekFrame = buffer.peek();

   ASSERT_EQ(peekFrame->id, frameID2); 
   ASSERT_EQ(buffer.size(), 2);

   // Remove first inserted frame
   CANFrame* gotFrame = buffer.get();
   
   ASSERT_EQ(gotFrame->id, frameID2);
   ASSERT_EQ(buffer.size(), 1);

   // Remove other frame, should be the last inserted (3rd frame)
   gotFrame = buffer.get();

   ASSERT_EQ(gotFrame->id, frameID3);
   ASSERT_EQ(buffer.size(), 0);

   // Should now be empty
   ASSERT_TRUE(buffer.empty());
   ASSERT_EQ(buffer.puts(), 3);
   ASSERT_EQ(buffer.gets(), 2);
   ASSERT_EQ(buffer.overflows(), 1);
}

int main(int argc, char **argv)
{
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}