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

#include "CBUSLED.h"
#include "SystemTickFake.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <pico/stdlib.h>

TEST(CBUSLED, init)
{
   CBUSLED led;

   // Check initialization - LED should be OFF
   led.setPin(1);
   ASSERT_FALSE(led.getState());
}

TEST(CBUSLED, turnOnOff)
{
   CBUSLED led;

   led.setPin(1);
  
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
   CBUSLED led;

   led.setPin(1);
  
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

   CBUSLED led;
   led.setPin(1);
   led.setShortPulseDuration(SHORT_FLICKER_TIME);
   led.setLongPulseDuration(LONG_FLICKER_TIME);

   ASSERT_FALSE(led.getState());

   // Long pulse - should turn ON
   led.pulse(false);
   led.run();
   ASSERT_TRUE(led.getState());

   // Half pulse duration expired - should still be OK
   incFakeSystemTime(LONG_FLICKER_TIME / 2);
   led.run();
   ASSERT_TRUE(led.getState());

   // Full pulse duration expired - should be OFF
   incFakeSystemTime(LONG_FLICKER_TIME / 2);
   led.run();
   ASSERT_FALSE(led.getState());

   // Short pulse - should turn OK
   led.pulse(true);
   led.run();
   ASSERT_TRUE(led.getState());

   // Half pulse duration expired - should still be OK
   incFakeSystemTime(SHORT_FLICKER_TIME / 2);
   led.run();
   ASSERT_TRUE(led.getState());

   // Full pulse duration expired - should be OFF
   incFakeSystemTime(SHORT_FLICKER_TIME / 2);
   led.run();
   ASSERT_FALSE(led.getState());
}

TEST(CBUSLED, blink)
{
   static constexpr const auto BLINK_RATE = 500;

   CBUSLED led;
   led.setPin(1);
   led.setBlinkRate(BLINK_RATE);

   ASSERT_FALSE(led.getState());

   led.blink();
   led.run();

   // Should initially ON
   ASSERT_TRUE(led.getState());

   // at half blink rate - should still be ON
   incFakeSystemTime(BLINK_RATE / 2);
   led.run();
   ASSERT_TRUE(led.getState());

   // at complete blink rate - should be OFF
   incFakeSystemTime(BLINK_RATE / 2);
   led.run();
   ASSERT_FALSE(led.getState());

   // Check blinking continues
   for (auto i=0; i < 10; i++)
   {
      incFakeSystemTime(BLINK_RATE);
      led.run();

      ASSERT_TRUE(led.getState());

      incFakeSystemTime(BLINK_RATE);
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