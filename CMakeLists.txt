# CMake definition for VLCB-Arduino project
# This is necessary for working with the CLion IDE.

cmake_minimum_required(VERSION 3.22.1)
project(VLCB_Arduino LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 11)

include(cmake/lto.cmake)
include(cmake/arduino_core.cmake)
include(cmake/arduino_libs.cmake)
include(cmake/arduino_Streaming.cmake)
include(cmake/arduino_ACAN2515.cmake)
include(cmake/arduino_hex.cmake)
include(cmake/arduino_upload.cmake)
include(cmake/arduino_flags.cmake)

include_directories(examples)
include_directories(examples/VLCB_1in1out)
include_directories(examples/VLCB_4in4out)
include_directories(examples/VLCB_empty)
include_directories(examples/VLCB_long_message_example)
include_directories(src)

add_library(hardware_library OBJECT
        src/CAN2515.cpp
        src/LED.cpp
        src/Switch.cpp
        src/CreateDefaultStorageForPlatform.cpp
        src/DueEepromEmulationStorage.cpp
        src/EepromExternalStorage.cpp
        src/EepromInternalStorage.cpp
        src/FlashStorage.cpp
        src/LEDUserInterface.cpp
        src/SerialUserInterface.cpp
        src/CombinedUserInterface.cpp
        src/SerialGC.cpp
        )
target_include_directories(hardware_library INTERFACE
        src
        )

target_link_libraries(hardware_library PUBLIC ArduinoFlags)
target_link_libraries(hardware_library PUBLIC ArduinoLibs)
target_link_libraries(hardware_library PUBLIC ArduinoStreaming)
target_link_libraries(hardware_library PUBLIC ArduinoACAN2515)

add_library(core_library OBJECT
        src/Controller.cpp
        src/Configuration.cpp
        src/LongMessageService.cpp
        src/Parameters.cpp
        src/CanTransport.cpp
        src/MinimumNodeService.cpp
        src/CanService.cpp
        src/EventConsumerService.cpp
        src/EventTeachingService.cpp
        src/EventProducerService.cpp
        src/NodeVariableService.cpp
        src/ConsumeOwnEventsService.cpp
        src/GridConnect.cpp
        )

target_include_directories(core_library INTERFACE
        src
        )

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
        target_link_libraries(core_library PUBLIC ArduinoFlags)
        target_link_libraries(core_library PUBLIC ArduinoLibs)
        target_link_libraries(core_library PUBLIC ArduinoStreaming)
        target_link_libraries(core_library PUBLIC ArduinoACAN2515)
else()
        target_link_libraries(core_library PUBLIC ArduinoFlags)
        target_link_libraries(core_library PUBLIC ArduinoMock)
endif()

add_library(ArduinoMock INTERFACE)
target_include_directories(ArduinoMock INTERFACE
        Arduino
        )

add_executable(testAll
        test/ArduinoMock.cpp
        test/TestTools.cpp
        test/testArduino.cpp
        test/testAll.cpp
        test/testMinimumNodeService.cpp
        test/testNodeVariableService.cpp
        test/testCanService.cpp
        test/MockUserInterface.cpp
        test/MockStorage.cpp
        test/MockCanTransport.cpp
        test/MockTransportService.cpp
        test/VlcbCommon.cpp
        test/testEventProducerService.cpp
        test/testEventConsumerService.cpp
        test/testEventTeachingService.cpp
        test/testConsumeOwnEventsService.cpp
        test/testLongMessageService.cpp
        test/testGridConnect.cpp
        test/testConfiguration.cpp
        test/testCircularBuffer.cpp
        )

target_link_libraries(testAll PUBLIC ArduinoFlags)
target_link_libraries(testAll PUBLIC ArduinoMock)
target_link_libraries(testAll PUBLIC core_library)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
        arduino_avr_hex(VLCB_1in1out)

        set(ARDUINO_PORT "/dev/ttyACM0" CACHE STRING
                "The serial port for uploading using avrdude")

        arduino_avr_upload(VLCB_1in1out ${ARDUINO_PORT})
else()
        enable_testing()
        add_test(NAME testAll COMMAND $<TARGET_FILE:testAll>)
endif()
