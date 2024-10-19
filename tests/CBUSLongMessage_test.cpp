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
#include "CBUSConfig.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mocklib.h"

#include <pico/stdlib.h>

using testing::_;

using testing::Return;

// Receive callback mock

static constexpr const auto streamLen {10};
static constexpr const auto numContext{4};

class RcvMock
{
public:
   uint8_t buf[streamLen];
   MOCK_METHOD(void, longMsgHandler, (void *fragment, const uint32_t fragment_len, const uint8_t stream_id, const uint8_t status));
};

RcvMock* pMock;

void Handler(void *fragment, const uint32_t fragment_len, const uint8_t stream_id, const uint8_t status)
{
   // Cache result up to max size of receive buffer cache
   memcpy(pMock->buf, fragment, streamLen);
   pMock->longMsgHandler(fragment, fragment_len, stream_id, status);
}

TEST(CBUSLongMessage, basic)
{
   static constexpr const auto timeout {10};
   static constexpr const auto delay {20};

   CBUSConfig config;
   CBUSMock cbus(config);
   CBUSLongMessage longMsg(&cbus);

   // Setup class
   longMsg.setTimeout(timeout);
   longMsg.setDelay(delay);

   // CRC check
   extern uint32_t crc32(const uint8_t *s, size_t n);
   extern uint16_t crc16(uint8_t *data_p, uint16_t length);
   uint8_t data[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};

   // 32-bit CRC-32/ISO-HDLC
   uint32_t crc32bit = crc32(data, sizeof(data));
   ASSERT_EQ(crc32bit, 0x456CD746);
   
   // Zero length crc
   crc32bit = crc32(data, 0); 
   ASSERT_EQ(crc32bit, 0);

   // 16-bit CRC [check which polynomial!!]
   uint16_t crc16bit = crc16(data, sizeof(data));
   ASSERT_EQ(crc16bit, 0xE22F);

   // Zero length crc
   crc16bit = crc16(data, 0); 
   ASSERT_EQ(crc16bit, 0);
}

TEST(CBUSLongMessage, sendMsg)
{
   static constexpr const auto streamID {1};
   static constexpr const auto priority {11};
   static constexpr const auto delay{1};

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

   CBUSConfig config;
   CBUSMock cbus(config);

   EXPECT_CALL(cbus, sendMessageImpl(_,false,false,priority)).WillRepeatedly(Return(true));

   CBUSLongMessage longMsg(&cbus);
   longMsg.setDelay(delay);

   // Long message #1, should be sent as 1x header, 1x segment
   uint8_t msg1[] = {0x00,0x01,0x02,0x03,0x04};

   ASSERT_TRUE(longMsg.sendLongMessage(msg1, sizeof(msg1), streamID, priority));

   // Transmit header
   ASSERT_TRUE(longMsg.process());
   ASSERT_TRUE(longMsg.isSending());
   sysTime += delay;

   // Try sending another message before completion
   ASSERT_FALSE(longMsg.sendLongMessage(msg1, sizeof(msg1), streamID+1, priority));

   // Transmit segment#1 - should complete sending
   ASSERT_TRUE(longMsg.process());
   ASSERT_FALSE(longMsg.isSending());
   sysTime += delay;

   // Long message #2, should be send as 6x segments of 5 chars each
   //             1----2----3----4----5----6----
   char msg2[] = "This is a long message to send";
   uint8_t msgLen = strlen(msg2); // Length to exclude NULL

   ASSERT_TRUE(longMsg.sendLongMessage(msg2, msgLen, streamID, priority));

   uint8_t nCalls = 0;
   while (longMsg.isSending()) 
   {
      ASSERT_TRUE(longMsg.process());
      sysTime += delay;
      nCalls++;
   }
   
   // Should now be complete
   ASSERT_EQ(nCalls, msgLen / 5);
   ASSERT_FALSE(longMsg.isSending());
}

TEST(CBUSLongMessage, receive)
{
   static constexpr const auto numStreams {5};
   static constexpr const auto streamID {2};

   uint8_t streamIDs[numStreams] {0,1,2,3,4};
   uint8_t rcvBuffer[streamLen] {};
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

   CBUSConfig config;
   CBUSMock cbus(config);
   RcvMock rcvMock;
   pMock = &rcvMock;

   EXPECT_CALL(rcvMock, longMsgHandler(_,10,streamID,CBUS_LONG_MESSAGE_COMPLETE)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,0, streamID,CBUS_LONG_MESSAGE_TIMEOUT_ERROR)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,5, streamID,CBUS_LONG_MESSAGE_TIMEOUT_ERROR)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,0, streamID,CBUS_LONG_MESSAGE_SEQUENCE_ERROR)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,10,streamID,CBUS_LONG_MESSAGE_TRUNCATED)).Times(1);

   CBUSLongMessage longMsg(&cbus);

   longMsg.subscribe(streamIDs, numStreams, rcvBuffer, streamLen, Handler);

   // Two segment long message - 10 bytes as 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09
   CANFrame header = {.id=1, .ext=false, .rtr=false, .len=5, .data={0xE9,streamID,0x00,0x00,streamLen}};
   CANFrame seg1 =   {.id=1, .ext=false, .rtr=false, .len=8, .data={0xE9,streamID,0x01,0x00,0x01,0x02,0x03,0x04}};
   CANFrame seg2 =   {.id=1, .ext=false, .rtr=false, .len=8, .data={0xE9,streamID,0x02,0x05,0x06,0x07,0x08,0x09}};

   // segment 1 - invalid sequence number
   CANFrame seg1Seq= {.id=1, .ext=false, .rtr=false, .len=8, .data={0xE9,streamID,0x03,0x00,0x01,0x02,0x03,0x04}};

   // Exceed buffer size
   CANFrame hdrBig = {.id=1, .ext=false, .rtr=false, .len=5, .data={0xE9,streamID,0x00,0x00,streamLen + 1}};
   CANFrame seg3 =   {.id=1, .ext=false, .rtr=false, .len=4, .data={0xE9,streamID,0x03,0x0a}};

   longMsg.processReceivedMessageFragment(header);
   ASSERT_TRUE(longMsg.process());

   longMsg.processReceivedMessageFragment(seg1);
   ASSERT_TRUE(longMsg.process());

   longMsg.processReceivedMessageFragment(seg2);
   ASSERT_TRUE(longMsg.process());

   // Validate received message
   ASSERT_EQ(memcmp(&rcvMock.buf[0], &seg1.data[3], streamLen / 2), 0);
   ASSERT_EQ(memcmp(&rcvMock.buf[streamLen / 2], &seg2.data[3], streamLen / 2), 0);

   // Receive with timeout after header
   longMsg.setTimeout(1);
   longMsg.processReceivedMessageFragment(header);
   ASSERT_TRUE(longMsg.process());
   sysTime += 10;
   ASSERT_TRUE(longMsg.process());

   // Receive timeout after first segment
   longMsg.processReceivedMessageFragment(header);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg1);
   ASSERT_TRUE(longMsg.process());

   sysTime += 10;
   ASSERT_TRUE(longMsg.process());

   // Invalid sequence number
   longMsg.processReceivedMessageFragment(header);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg1Seq);
   ASSERT_TRUE(longMsg.process());

   // Frame exceeds buffer size
   longMsg.processReceivedMessageFragment(hdrBig);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg1);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg2);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg3);
   ASSERT_TRUE(longMsg.process());
}

TEST(CBUSLongMessageEx, sendMsg)
{
   static constexpr const auto streamID {1};
   static constexpr const auto priority {11};
   static constexpr const auto delay{1};

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

   CBUSConfig config;
   CBUSMock cbus(config);

   EXPECT_CALL(cbus, sendMessageImpl(_,false,false,priority)).WillRepeatedly(Return(true));

   CBUSLongMessageEx longMsg(&cbus);
   longMsg.setDelay(delay);

   // Long message #1, should be sent as 1x header, 1x segment
   uint8_t msg1[] = {0x00,0x01,0x02,0x03,0x04};

   // These should all fail, as no contexts allocated (yet)
   ASSERT_FALSE(longMsg.sendLongMessage(msg1, sizeof(msg1), streamID, priority));
   ASSERT_FALSE(longMsg.process());
   ASSERT_FALSE(longMsg.isSending());
   CANFrame dummy{};
   longMsg.processReceivedMessageFragment(dummy);

   // Allocate contexts
   longMsg.allocateContexts(numContext, streamLen, numContext);

   // With contexts, should now send successfully
   ASSERT_TRUE(longMsg.sendLongMessage(msg1, sizeof(msg1), streamID, priority));

   // Transmit header
   ASSERT_TRUE(longMsg.process());
   ASSERT_TRUE(longMsg.isSending());
   sysTime += delay;

   // Try sending another message before completion - should succeed
   ASSERT_TRUE(longMsg.sendLongMessage(msg1, sizeof(msg1), streamID+1, priority));

   // Complete sending first message
   ASSERT_TRUE(longMsg.process());
   ASSERT_TRUE(longMsg.isSending());
   sysTime += delay;

   // Sending second message - should do nothing - inter message delay
   ASSERT_TRUE(longMsg.process());
   ASSERT_TRUE(longMsg.isSending());
   sysTime += delay;

   // Transmit header second message
   ASSERT_TRUE(longMsg.process());
   ASSERT_TRUE(longMsg.isSending());
   sysTime += delay;

   // Transmit remainder second message
   ASSERT_TRUE(longMsg.process());
   ASSERT_FALSE(longMsg.isSending());
   sysTime += delay;

   // Long message #2, should be send as 6x segments of 5 chars each
   //             1----2----3----4----5----6----
   char msg2[] = "This is a long message to send";
   uint8_t msgLen = strlen(msg2); // Length to exclude NULL

   ASSERT_TRUE(longMsg.sendLongMessage(msg2, msgLen, streamID, priority));

   uint8_t nCalls = 0;
   while (longMsg.isSending()) 
   {
      ASSERT_TRUE(longMsg.process());
      sysTime += delay;
      nCalls++;
   }
   
   // Should now be complete - messages are processed round robin by context
   ASSERT_EQ(nCalls, (msgLen / 5) * numContext);
   ASSERT_FALSE(longMsg.isSending());

   // Send with CRC
   longMsg.use_crc(true);

   ASSERT_TRUE(longMsg.sendLongMessage(msg2, msgLen, streamID, priority));

   nCalls = 0;
   while (longMsg.isSending()) 
   {
      ASSERT_TRUE(longMsg.process());
      sysTime += delay;
      nCalls++;
   }
   
   // Should now be complete - messages are processed round robin by context
   ASSERT_EQ(nCalls, (msgLen / 5) * numContext);
   ASSERT_FALSE(longMsg.isSending());

   // Try sending same stream second time, while first is in progress
   ASSERT_TRUE(longMsg.sendLongMessage(msg2, msgLen, streamID, priority));
   ASSERT_TRUE(longMsg.process());
   ASSERT_FALSE(longMsg.sendLongMessage(msg2, msgLen, streamID, priority));
   ASSERT_TRUE(longMsg.process());

   // Complete transmission before next test
   while (longMsg.isSending()) 
   {
      ASSERT_TRUE(longMsg.process());
      sysTime += delay;
   }

   // Exceed number of transmit contexts - fill all contexts
   for (uint_fast8_t i=0; i < numContext; i++)
   {
      ASSERT_TRUE(longMsg.sendLongMessage(msg2, msgLen, streamID + i, priority));
   }

   // Try to send one more
   ASSERT_FALSE(longMsg.sendLongMessage(msg2, msgLen, numContext + 1, priority));

}

TEST(CBUSLongMessageEx, receive)
{
   static constexpr const auto numStreams {5};
   static constexpr const auto streamID {2};

   uint8_t streamIDs[numStreams] {0,1,2,3,4};
   uint8_t rcvBuffer[streamLen] {};
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

   CBUSConfig config;
   CBUSMock cbus(config);
   RcvMock rcvMock;
   pMock = &rcvMock;

   EXPECT_CALL(rcvMock, longMsgHandler(_,10,streamID,CBUS_LONG_MESSAGE_COMPLETE)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,0, streamID,CBUS_LONG_MESSAGE_TIMEOUT_ERROR)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,5, streamID,CBUS_LONG_MESSAGE_TIMEOUT_ERROR)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,0, streamID,CBUS_LONG_MESSAGE_SEQUENCE_ERROR)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,10,streamID,CBUS_LONG_MESSAGE_TRUNCATED)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,9,streamID,CBUS_LONG_MESSAGE_CRC_ERROR)).Times(1);
   EXPECT_CALL(rcvMock, longMsgHandler(_,9,streamID,CBUS_LONG_MESSAGE_COMPLETE)).Times(1);

   CBUSLongMessageEx longMsg(&cbus);

   // Allocate contexts
   longMsg.allocateContexts(numContext, streamLen, numContext);

   longMsg.subscribe(streamIDs, numStreams, Handler);

   // Two segment long message - 10 bytes as 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09
   CANFrame header = {.id=1, .ext=false, .rtr=false, .len=5, .data={0xE9,streamID,0x00,0x00,streamLen}};
   CANFrame seg1 =   {.id=1, .ext=false, .rtr=false, .len=8, .data={0xE9,streamID,0x01,0x00,0x01,0x02,0x03,0x04}};
   CANFrame seg2 =   {.id=1, .ext=false, .rtr=false, .len=8, .data={0xE9,streamID,0x02,0x05,0x06,0x07,0x08,0x09}};

   // segment 1 - invalid sequence number
   CANFrame seg1Seq= {.id=1, .ext=false, .rtr=false, .len=8, .data={0xE9,streamID,0x03,0x00,0x01,0x02,0x03,0x04}};

   // Exceed buffer size
   CANFrame hdrBig = {.id=1, .ext=false, .rtr=false, .len=5, .data={0xE9,streamID,0x00,0x00,streamLen + 1}};
   CANFrame seg3 =   {.id=1, .ext=false, .rtr=false, .len=4, .data={0xE9,streamID,0x03,0x0a}};

   // segment 1 - header with 16 bit CRC (invalid) 9 byte 
   CANFrame hdrCRCbad = {.id=1, .ext=false, .rtr=false, .len=7, .data={0xE9,streamID,0x00,0x00,streamLen-1,0x01,0x02}};

   // segment 1 - header with 16 bit CRC (valid) 9 bytes
   CANFrame hdrCRCgood = {.id=1, .ext=false, .rtr=false, .len=7, .data={0xE9,streamID,0x00,0x00,streamLen-1,0x40,0x5f}};

   // Receive valid stream - 3 fragments
   longMsg.processReceivedMessageFragment(header);
   ASSERT_TRUE(longMsg.process());

   longMsg.processReceivedMessageFragment(seg1);
   ASSERT_TRUE(longMsg.process());

   longMsg.processReceivedMessageFragment(seg2);
   ASSERT_TRUE(longMsg.process());

   // Validate received message
   ASSERT_EQ(memcmp(&rcvMock.buf[0], &seg1.data[3], streamLen / 2), 0);
   ASSERT_EQ(memcmp(&rcvMock.buf[streamLen / 2], &seg2.data[3], streamLen / 2), 0);

   // Receive with timeout after header
   longMsg.setTimeout(1);
   longMsg.processReceivedMessageFragment(header);
   ASSERT_TRUE(longMsg.process());
   sysTime += 10;
   ASSERT_TRUE(longMsg.process());

   // Receive timeout after first segment
   longMsg.processReceivedMessageFragment(header);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg1);
   ASSERT_TRUE(longMsg.process());

   sysTime += 10;
   ASSERT_TRUE(longMsg.process());

   // Invalid sequence number
   longMsg.processReceivedMessageFragment(header);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg1Seq);
   ASSERT_TRUE(longMsg.process());

   // Frame exceeds buffer size
   longMsg.processReceivedMessageFragment(hdrBig);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg1);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg2);
   ASSERT_TRUE(longMsg.process());
   longMsg.processReceivedMessageFragment(seg3);
   ASSERT_TRUE(longMsg.process());

   // Enable CRC use
   longMsg.use_crc(true);

   // Receive  stream - 3 fragments - invalid CRC
   longMsg.processReceivedMessageFragment(hdrCRCbad);
   ASSERT_TRUE(longMsg.process());

   longMsg.processReceivedMessageFragment(seg1);
   ASSERT_TRUE(longMsg.process());

   longMsg.processReceivedMessageFragment(seg2);
   ASSERT_TRUE(longMsg.process());

   // Receive  stream - 3 fragments - valid CRC
   longMsg.processReceivedMessageFragment(hdrCRCgood);
   ASSERT_TRUE(longMsg.process());

   longMsg.processReceivedMessageFragment(seg1);
   ASSERT_TRUE(longMsg.process());

   longMsg.processReceivedMessageFragment(seg2);
   ASSERT_TRUE(longMsg.process());

   // Validate received message - 9 byte stream
   ASSERT_EQ(memcmp(&rcvMock.buf[0], &seg1.data[3], streamLen / 2), 0);
   ASSERT_EQ(memcmp(&rcvMock.buf[streamLen / 2], &seg2.data[3], (streamLen / 2)-1), 0);
}

int main(int argc, char **argv)
{
   // The following line must be executed to initialize Google Mock
   // (and Google Test) before running the tests.
   ::testing::InitGoogleMock(&argc, argv);
   return RUN_ALL_TESTS();
}