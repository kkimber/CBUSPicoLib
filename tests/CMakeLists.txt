cmake_minimum_required(VERSION 3.5)

project(tests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DEFAULT_STARTUP_PROJECT CBUSLEDtest)

include_directories("..")

# vscode cmake tools (necessary only in old version ??)
 include(CMakeToolsHelpers OPTIONAL)

# CBUS LED Tests
 add_executable(CBUSLEDtest
   ../CBUSLED.cpp
   ./pico/stdlib.cpp
   ./SystemTickFake.cpp
   ./CBUSLED_test.cpp
)
target_link_libraries(CBUSLEDtest gtest gmock)

# CTest
add_test(CBUSLED CBUSLEDtest)

# CBUS Switch Tests
add_executable(CBUSSwitchtest
   ../CBUSSwitch.cpp
   ./pico/stdlib
   ./SystemTickFake
   ./CBUSSwitch_test.cpp
)
target_link_libraries(CBUSSwitchtest gtest gmock)

# CTest
add_test(CBUSSwitch CBUSSwitchtest)
