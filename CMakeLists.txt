cmake_minimum_required(VERSION 3.22.1)
project(VLCB_Arduino LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 11)

include(cmake/lto.cmake)
include(cmake/arduino_flags.cmake)

add_subdirectory(src)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
  set(ARDUINO_PORT "/dev/ttyACM0" CACHE STRING
    "The serial port for uploading using avrdude")

  include(cmake/arduino_sdk.cmake)

  include(examples/VLCB_1in1out/VLCB_1in1out.cmake)
  include(examples/VLCB_4in4out/VLCB_4in4out.cmake)
  include(examples/VLCB_empty/VLCB_empty.cmake)
  include(examples/VLCB_long_message_example/VLCB_long_message_example.cmake)
else()
  add_subdirectory(Arduino)
  add_subdirectory(test)

  enable_testing()
  add_test(NAME testAll COMMAND $<TARGET_FILE:testAll>)
endif()
