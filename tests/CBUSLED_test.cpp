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

#include "CBUSLED.h"
#include "SystemTick.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <pico/stdlib.h>

#include "mocklib.h"

using testing::_;
using testing::AnyNumber;
using testing::Return;

TEST(CBUSLED, systemTime)
{
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

   sysTime = 1234ULL;

   EXPECT_EQ(SystemTick::GetMilli(), sysTime);
   EXPECT_EQ(SystemTick::GetMicros(), sysTime * 1000);

   // Test clamp to 32-bits
   sysTime = 0x1FFFFFFFF;
   EXPECT_EQ(SystemTick::GetMilli(), (uint32_t)sysTime);
}

// Lets us pin 1 as the LED pin
static constexpr const auto pinLED {1};

TEST(CBUSLED, init)
{
   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, gpio_init(pinLED));
   EXPECT_CALL(mockPicoSdk, gpio_set_dir(pinLED, GPIO_OUT));
   EXPECT_CALL(mockPicoSdk, gpio_put(pinLED, false));
   EXPECT_CALL(mockPicoSdk, get_absolute_time())
      .Times(AnyNumber()).WillRepeatedly(Return(0));

   CBUSLED led;

   // Check initialization - LED should be OFF
   led.setPin(pinLED);
   ASSERT_FALSE(led.getState());
}

TEST(CBUSLED, turnOnOff)
{
   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, gpio_init(pinLED));
   EXPECT_CALL(mockPicoSdk, gpio_set_dir(pinLED, GPIO_OUT));
   EXPECT_CALL(mockPicoSdk, gpio_put(pinLED, false)).Times(2);
   EXPECT_CALL(mockPicoSdk, gpio_put(pinLED, true)).Times(1);
   EXPECT_CALL(mockPicoSdk, get_absolute_time())
      .Times(AnyNumber()).WillRepeatedly(Return(0));

   CBUSLED led;

   // Set OFF initially
   led.setPin(pinLED);
   ASSERT_FALSE(led.getState());

   // Turn ON
   led.on();
   led.run();
   ASSERT_TRUE(led.getState());

   // Turn OFF
   led.off();
   led.run();
   ASSERT_FALSE(led.getState());
}

TEST(CBUSLED, toggle)
{
   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, gpio_init(pinLED));
   EXPECT_CALL(mockPicoSdk, gpio_set_dir(pinLED, GPIO_OUT));
   EXPECT_CALL(mockPicoSdk, gpio_put(pinLED, false)).Times(2);
   EXPECT_CALL(mockPicoSdk, gpio_put(pinLED, true)).Times(1);
   EXPECT_CALL(mockPicoSdk, get_absolute_time())
      .Times(AnyNumber()).WillRepeatedly(Return(0));

   CBUSLED led;

   // Set OFF initially
   led.setPin(pinLED);
   ASSERT_FALSE(led.getState());

   // Toggle ON
   led.toggle();
   led.run();
   ASSERT_TRUE(led.getState());

   // Toggle OFF
   led.toggle();
   led.run();
   ASSERT_FALSE(led.getState());
}

TEST(CBUSLED, pulse)
{
   static constexpr const auto SHORT_FLICKER_TIME = 100;
   static constexpr const auto LONG_FLICKER_TIME = 500;

   uint64_t sysTime = 0ULL;

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, gpio_init(pinLED));
   EXPECT_CALL(mockPicoSdk, gpio_set_dir(pinLED, GPIO_OUT));
   EXPECT_CALL(mockPicoSdk, gpio_put(pinLED, _)).Times(AnyNumber());

   // Manage system time via lambda
   EXPECT_CALL(mockPicoSdk, get_absolute_time)
       .WillRepeatedly(testing::Invoke(
        [&sysTime]() -> uint64_t {
            return sysTime * 1000; // time specified in milliseconds
        }
    ));

   CBUSLED led;
   led.setPin(pinLED);
   led.setShortPulseDuration(SHORT_FLICKER_TIME);
   led.setLongPulseDuration(LONG_FLICKER_TIME);

   ASSERT_FALSE(led.getState());

   // Long pulse - should turn ON
   led.pulse(false);
   led.run();
   ASSERT_TRUE(led.getState());

   // Half pulse duration expired - should still be OK
   sysTime += (LONG_FLICKER_TIME / 2);
   led.run();
   ASSERT_TRUE(led.getState());

   // Full pulse duration expired - should be OFF
   sysTime += (LONG_FLICKER_TIME / 2);
   led.run();
   ASSERT_FALSE(led.getState());

   // Short pulse - should turn OK
   led.pulse(true);
   led.run();
   ASSERT_TRUE(led.getState());

   // Half pulse duration expired - should still be OK
   sysTime += (SHORT_FLICKER_TIME / 2);
   led.run();
   ASSERT_TRUE(led.getState());

   // Full pulse duration expired - should be OFF
   sysTime += (SHORT_FLICKER_TIME / 2);
   led.run();
   ASSERT_FALSE(led.getState());
}

TEST(CBUSLED, blink)
{
   uint64_t sysTime = 0ULL;

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   EXPECT_CALL(mockPicoSdk, gpio_init(pinLED));
   EXPECT_CALL(mockPicoSdk, gpio_set_dir(pinLED, GPIO_OUT));
   EXPECT_CALL(mockPicoSdk, gpio_put(pinLED, _)).Times(AnyNumber());

   // Manage system time via lambda
   EXPECT_CALL(mockPicoSdk, get_absolute_time)
       .WillRepeatedly(testing::Invoke(
        [&sysTime]() -> uint64_t {
            return sysTime * 1000; // time specified in milliseconds
        }
    ));

   static constexpr const auto BLINK_RATE = 500;

   CBUSLED led;
   led.setPin(pinLED);
   led.setBlinkRate(BLINK_RATE);
   led.run();

   ASSERT_FALSE(led.getState());

   sysTime += BLINK_RATE;

   led.blink();
   led.run();

   // Should initially ON
   ASSERT_TRUE(led.getState());

   // at half blink rate - should still be ON
   sysTime += (BLINK_RATE / 2);
   led.run();
   ASSERT_TRUE(led.getState());

   // at complete blink rate - should be OFF
   sysTime += (BLINK_RATE / 2);
   led.run();
   ASSERT_FALSE(led.getState());

   // Check blinking continues
   for (auto i=0; i < 10; i++)
   {
      sysTime += (BLINK_RATE);
      led.run();

      ASSERT_TRUE(led.getState());

      sysTime += (BLINK_RATE);
      led.run();

      ASSERT_FALSE(led.getState());
   }

   // Cancel blinking
   led.on();
   led.run();

   ASSERT_TRUE(led.getState());
}

int main(int argc, char **argv)
{
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}