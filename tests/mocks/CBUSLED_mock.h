#include <cstdint>
#include <gmock/gmock.h>

//
/// CBUSLED class mock
//

class CBUSLED
{
public:
   MOCK_METHOD(void, setPin, (const uint8_t pin));
   MOCK_METHOD(bool, getState, (), (const));
   MOCK_METHOD(void, on, ());
   MOCK_METHOD(void, off, ());
   MOCK_METHOD(void, toggle, ());
   MOCK_METHOD(void, blink, ());
   MOCK_METHOD(void, run, ());
   MOCK_METHOD(void, pulse, (bool bShort));
   MOCK_METHOD(void, setBlinkRate, (uint16_t msBlink));
   MOCK_METHOD(void, setShortPulseDuration, (uint16_t msPulseShort));
   MOCK_METHOD(void, setLongPulseDuration, (uint16_t msPulseLong));
};