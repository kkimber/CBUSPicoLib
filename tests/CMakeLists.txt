cmake_minimum_required(VERSION 3.5)

project(tests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DEFAULT_STARTUP_PROJECT CBUSLEDtest)

include_directories("..")

# vscode cmake tools (necessary only in old version ??)
 include(CMakeToolsHelpers OPTIONAL)

# CBUS LED Tests ================================
 add_executable(CBUSLEDtest
   ../CBUSLED.cpp
   ./pico/stdlib.cpp
   mocks/SystemTickFake.cpp
   ./CBUSLED_test.cpp
)
target_include_directories(CBUSLEDtest PUBLIC mocks)
target_link_libraries(CBUSLEDtest gtest gmock)

# CTest
add_test(CBUSLED CBUSLEDtest)

# CBUS Switch Tests =============================
add_executable(CBUSSwitchtest
   ../CBUSSwitch.cpp
   ./pico/stdlib.cpp
   mocks/SystemTickFake.cpp
   ./CBUSSwitch_test.cpp
)
target_include_directories(CBUSSwitchtest PUBLIC mocks)
target_link_libraries(CBUSSwitchtest gtest gmock)

# CTest
add_test(CBUSSwitch CBUSSwitchtest)


# CBUS Circular Buffer Tests ====================
add_executable(CBUSCircularBuffertest
   ../CBUSCircularBuffer.cpp
   ./pico/stdlib.cpp
   mocks/SystemTickFake.cpp
   ./CBUSCircularBuffer_test.cpp
)
target_include_directories(CBUSCircularBuffertest PUBLIC mocks)
target_link_libraries(CBUSCircularBuffertest gtest gmock)

# CTest
add_test(CBUSCircularBuffer CBUSCircularBuffertest)

# CBUS Config Tests ====================
add_executable(CBUSConfigtest
   ../CBUSConfig.cpp
   ../CBUSLED.cpp
   ../CBUSSwitch.cpp
   ./pico/stdlib.cpp
   mocks/SystemTickFake.cpp
   mocks/i2c_mock.cpp
   mocks/flash_mock.cpp
   ./CBUSConfig_test.cpp
)
target_include_directories(CBUSConfigtest PUBLIC mocks)
target_link_libraries(CBUSConfigtest gtest gmock)

# CTest
add_test(CBUSConfig CBUSConfigtest)

# CBUS Long Message Tests =============
add_executable(CBUSLongMessagetest
   ../CBUSLongMessage.cpp
   ../CBUSConfig.cpp
   ../CBUSCircularBuffer
   ../CBUS.cpp
   ../CBUSLED.cpp
   ../CBUSSwitch.cpp
   ./pico/stdlib.cpp
   mocks/SystemTickFake.cpp
   mocks/i2c_mock.cpp
   mocks/flash_mock.cpp
   ./CBUSLongMessage_test.cpp
)
target_include_directories(CBUSLongMessagetest PUBLIC mocks)
target_link_libraries(CBUSLongMessagetest gtest gmock)

# CTest
add_test(CBUSLongMessage CBUSLongMessagetest)
