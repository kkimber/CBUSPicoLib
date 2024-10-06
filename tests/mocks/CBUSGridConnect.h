#pragma once

#include <gmock/gmock.h>
#include <CBUSGridConnectBase.h>

class CBUSGridConnect : public CBUSGridConnectBase
{
public:
   MOCK_METHOD(bool, available, (), (override));
   MOCK_METHOD(bool, canSend, (), (override));
   MOCK_METHOD(CANFrame, get, (), (override));
   MOCK_METHOD(void, sendCANFrame, (const CANFrame &msg, bool bMore), (override));
};