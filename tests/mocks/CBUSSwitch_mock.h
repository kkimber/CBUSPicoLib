#include <cstdint>
#include <gmock/gmock.h>

//
/// CBUSSwitch class mock
//

class CBUSSwitch
{
public:
   MOCK_METHOD(void, setPin, (const uint8_t pin, const bool pressedState));
   MOCK_METHOD(void, run, ());
   MOCK_METHOD(void, reset, ());
   MOCK_METHOD(bool, stateChanged, (), (const));
   MOCK_METHOD(bool, getState, (), (const));
   MOCK_METHOD(bool, isPressed, (), (const));
   MOCK_METHOD(uint32_t, getCurrentStateDuration, ());
   MOCK_METHOD(uint32_t, getLastStateDuration, ());
   MOCK_METHOD(uint32_t, getLastStateChangeTime, ());
   MOCK_METHOD(void, resetCurrentDuration, ());
   MOCK_METHOD(void, setDebounceDuration, (uint32_t debounceDuration));
};