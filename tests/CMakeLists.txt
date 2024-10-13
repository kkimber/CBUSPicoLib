cmake_minimum_required(VERSION 3.5)

project(tests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DEFAULT_STARTUP_PROJECT CBUSLEDtest)

include_directories("..")

add_subdirectory(mocklib)

# vscode cmake tools (necessary only in old version ??)
 include(CMakeToolsHelpers OPTIONAL)

# CBUS LED Tests ================================
 add_executable(CBUSLEDtest
   ../SystemTick.cpp
   ../CBUSLED.cpp
   ./CBUSLED_test.cpp
)
target_include_directories(CBUSLEDtest PUBLIC mocklib)
target_link_libraries(CBUSLEDtest mocklib gtest gmock)

# CTest
add_test(CBUSLED CBUSLEDtest)

# CBUS Switch Tests =============================
add_executable(CBUSSwitchtest
   ../SystemTick.cpp
   ../CBUSSwitch.cpp
   ./CBUSSwitch_test.cpp
)
target_include_directories(CBUSSwitchtest PUBLIC mocklib)
target_link_libraries(CBUSSwitchtest mocklib gtest gmock)

# CTest
add_test(CBUSSwitch CBUSSwitchtest)

# CBUS Circular Buffer Tests ====================
add_executable(CBUSCircularBuffertest
   ../SystemTick.cpp
   ../CBUSCircularBuffer.cpp
   ./CBUSCircularBuffer_test.cpp
)
target_include_directories(CBUSCircularBuffertest PUBLIC mocklib)
target_link_libraries(CBUSCircularBuffertest mocklib gtest gmock)

# CTest
add_test(CBUSCircularBuffer CBUSCircularBuffertest)

# CBUS Config Tests ====================
add_executable(CBUSConfigtest
   ../SystemTick.cpp
   ../CBUSConfig.cpp
   ../CBUSLED.cpp
   ../CBUSSwitch.cpp
   ./CBUSConfig_test.cpp
)
target_include_directories(CBUSConfigtest PUBLIC mocklib)
target_link_libraries(CBUSConfigtest mocklib gtest gmock)

# CTest
add_test(CBUSConfig CBUSConfigtest)

# CBUS Params Tests ====================
add_executable(CBUSParamstest
   ../SystemTick.cpp
   ../CBUSLED.cpp
   ../CBUSSwitch.cpp
   ../CBUSConfig.cpp
   ../CBUSParams.cpp
   ./CBUSParams_test.cpp
)
target_include_directories(CBUSParamstest PUBLIC mocklib mocks)
target_link_libraries(CBUSParamstest mocklib gtest gmock)

# CTest
add_test(CBUSParams CBUSParamstest)

# CBUS Long Message Tests =============
add_executable(CBUSLongMessagetest
   ../SystemTick.cpp
   ../CBUSLongMessage.cpp
   ../CBUSConfig.cpp
   ../CBUSCircularBuffer.cpp
   ../CBUS.cpp
   ../CBUSLED.cpp
   ../CBUSSwitch.cpp
   ./CBUSLongMessage_test.cpp
)
target_include_directories(CBUSLongMessagetest PUBLIC mocklib mocks)
target_link_libraries(CBUSLongMessagetest mocklib gtest gmock)

# CTest
add_test(CBUSLongMessage CBUSLongMessagetest)

# CBUS Tests ====================
add_executable(CBUStest
   ../SystemTick.cpp
   ../CBUSLongMessage.cpp
   ../CBUSConfig.cpp
   ../CBUSParams.cpp
   ../CBUSCircularBuffer.cpp
   ../CBUS.cpp
   ../CBUSLED.cpp
   ../CBUSSwitch.cpp
   ./CBUS_test.cpp
)
target_include_directories(CBUStest PUBLIC mocklib mocks)
target_link_libraries(CBUStest mocklib gtest gmock)

# CTest
add_test(CBUS CBUStest)
