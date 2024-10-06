#pragma once

#include <gmock/gmock.h>
#include <CBUS.h>

// Create the mocks
class CBUSMock : public CBUSbase
{
public:
   MOCK_METHOD(bool, begin, (), (override));
   MOCK_METHOD(bool, available, (), (override));
   MOCK_METHOD(CANFrame, getNextMessage,(), (override));
   MOCK_METHOD(bool, sendMessageImpl, (CANFrame &msg, bool rtr, bool ext, uint8_t priority));
   MOCK_METHOD(void, reset, (), (override));

   MOCK_METHOD(bool, validateNV, (const uint8_t NVindex, const uint8_t oldValue, const uint8_t NVvalue), (override));
   MOCK_METHOD(void, actUponNVchange, (const uint8_t NVindex, const uint8_t oldValue, const uint8_t NVvalue), (override));

   CBUSMock(CBUSConfig& config) : CBUSbase(config) {};

   bool sendMessage(CANFrame &msg, bool rtr = false, bool ext = false, uint8_t priority = DEFAULT_PRIORITY) override
   {
      return sendMessageImpl(msg, rtr, ext, priority);
   }
};