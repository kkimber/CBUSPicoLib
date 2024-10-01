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

#include "CBUSSwitch.h"
#include "SystemTickFake.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <pico/stdlib.h>

TEST(CBUSSwitch, initActiveLow)
{
   CBUSSwitch sw;

   // Run before init
   sw.run();

   // Unconfigured assumes pin is pull up and should therefore return high
   ASSERT_TRUE(sw.getState());

   // Init pin as active low
   sw.setPin(1, false);
   sw.run();
   
   // Pin should read as high (pulled up)
   ASSERT_TRUE(sw.getState());
}

TEST(CBUSSwitch, initActiveHigh)
{
   CBUSSwitch sw;

   // Init pin as active high
   sw.setPin(1, true);
   sw.run();
   
   // Pin should read as low (pulled down)
   ASSERT_FALSE(sw.getState());
}

TEST(CBUSSwitch, readState)
{
   static constexpr const auto debounceDuration {20};
   static constexpr const auto heldDuration {100};

   CBUSSwitch sw;

   // Init active LOW switch
   sw.setPin(1, false);
   sw.setDebounceDuration(debounceDuration);
   sw.run();

   ASSERT_FALSE(sw.isPressed());

   // Set pin as pushed
   setFakePinState(false);
   sw.run();

   // Should still indicate not pressed 
   ASSERT_TRUE(sw.getState());
   ASSERT_FALSE(sw.isPressed());
   ASSERT_FALSE(sw.stateChanged());

   // half debounce period, should still be false/off
   setFakeSystemTime(0);
   incFakeSystemTime(debounceDuration / 2);
   sw.run();
   ASSERT_TRUE(sw.getState());
   ASSERT_FALSE(sw.isPressed());

   // full debounce period, should indicate true/pressed
   incFakeSystemTime(debounceDuration / 2);
   sw.run();
   ASSERT_FALSE(sw.getState());
   ASSERT_TRUE(sw.isPressed());
   ASSERT_TRUE(sw.stateChanged());

   // check pressed duration
   ASSERT_EQ(sw.getCurrentStateDuration(), 0);

   // continue held pressed
   incFakeSystemTime(heldDuration);
   sw.run();

   ASSERT_EQ(sw.getCurrentStateDuration(), heldDuration);

   // release the button
   setFakePinState(true);
   sw.run();

   // advance time for off debounce
   incFakeSystemTime(debounceDuration);
   sw.run();

   // check release
   ASSERT_FALSE(sw.isPressed());

   // advance time 
   incFakeSystemTime(heldDuration);
   sw.run();

   // check held duration of previous press
   ASSERT_EQ(sw.getLastStateDuration(), heldDuration + debounceDuration);

   // check time of previous state change
   ASSERT_EQ(sw.getLastStateChangeTime(), heldDuration + (2 * debounceDuration));

   // press the button again  disable debounce
   setFakePinState(false);
   sw.setDebounceDuration(0);
   sw.run();

   // advance time
   incFakeSystemTime(heldDuration);
   sw.run();

   // check held duration
   ASSERT_EQ(sw.getCurrentStateDuration(), heldDuration);

   // reset held duration timer
   sw.resetCurrentDuration();

   // continue to hold button
   incFakeSystemTime(heldDuration * 2);
   sw.run();

   // check held duration
   ASSERT_EQ(sw.getCurrentStateDuration(), heldDuration * 2);
}

int main(int argc, char **argv)
{
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}