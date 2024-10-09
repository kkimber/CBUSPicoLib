add_library(mocklib SHARED
        clocks.cpp
        gpio.cpp
        mocklib.cpp
        i2c.cpp
        pio.cpp
        time.cpp
        flash.cpp
        )
include_directories (./)
