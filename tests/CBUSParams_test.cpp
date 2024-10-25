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

#include "CBUSParams.h"
#include "CBUSConfig.h"
#include "cbusdefs.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <pico/stdlib.h>

#include "mocklib.h"

TEST(CBUSParams, init)
{
   uint64_t sysTime = 0ULL;

   MockPicoSdk mockPicoSdk;
   mockPicoSdkApi.mockPicoSdk = &mockPicoSdk;

   CBUSConfig config;
   config.EE_NVS_START = 10;    // Offset start of Node Variables
   config.EE_NUM_NVS = 10;      // Number of Node Variables
   config.EE_EVENTS_START = 20; // Offset start of Events
   config.EE_MAX_EVENTS = 10;   // Maximum number of events
   config.EE_NUM_EVS = 1;       // Number of Event Variables per event (the InEventID)
   config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);

   CBUSParams params(config);
   cbusparam_t* pParams = params.getParams();

   EXPECT_EQ(pParams->param[PAR_NPARAMS], NUM_PARAMS);
   EXPECT_EQ(pParams->param[PAR_MANU], MANU_MERG);
   EXPECT_EQ(pParams->param[PAR_MINVER], 0);
   EXPECT_EQ(pParams->param[PAR_MTYP], 0);
   EXPECT_EQ(pParams->param[PAR_EVTNUM], config.EE_MAX_EVENTS);
   EXPECT_EQ(pParams->param[PAR_EVNUM], config.EE_NUM_EVS);
   EXPECT_EQ(pParams->param[PAR_NVNUM], config.EE_NUM_NVS);
   EXPECT_EQ(pParams->param[PAR_MAJVER], 0);
   EXPECT_EQ(pParams->param[PAR_FLAGS], 0);
   EXPECT_EQ(pParams->param[PAR_CPUID], 50);
   EXPECT_EQ(pParams->param[PAR_BUSTYPE], PB_CAN);
   EXPECT_EQ(pParams->param[PAR_LOAD], 0);
   EXPECT_EQ(pParams->param[PAR_CPUMID + 0], '2');
   EXPECT_EQ(pParams->param[PAR_CPUMID + 1], '0');
   EXPECT_EQ(pParams->param[PAR_CPUMID + 2], '4');
   EXPECT_EQ(pParams->param[PAR_CPUMID + 3], '0');
   EXPECT_EQ(pParams->param[PAR_CPUMAN], CPUM_ARM);
   EXPECT_EQ(pParams->param[PAR_BETA], 0);

   params.setFlags(0x01);
   EXPECT_EQ(pParams->param[PAR_FLAGS], 0x01);

   params.setModuleId(0x02);
   EXPECT_EQ(pParams->param[PAR_MTYP], 0x02);

   params.setVersion(0x04, 0x05, 0x06);
   EXPECT_EQ(pParams->param[PAR_MAJVER], 0x04);
   EXPECT_EQ(pParams->param[PAR_MINVER], 0x05);
   EXPECT_EQ(pParams->param[PAR_BETA], 0x06);
}

int main(int argc, char **argv)
{
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}