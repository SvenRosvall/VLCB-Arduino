# CMake definition for VLCB-Arduino project
# This is necessary for working with the CLion IDE.

cmake_minimum_required(VERSION 3.25)
project(VLCB_Arduino)

set(CMAKE_CXX_STANDARD 11)

include_directories(examples)
include_directories(examples/CBUS_1in1out)
include_directories(examples/CBUS_empty)
include_directories(examples/CBUS_long_message_example)
include_directories(src)
include_directories(Arduino)

add_library(hardware_library OBJECT
        src/CAN2515.cpp
        src/CAN2515.h
        src/LED.cpp
        src/LED.h
        src/Switch.cpp
        src/Switch.h
        src/CreateDefaultStorageForPlatform.cpp
        src/DueEepromEmulationStorage.cpp
        src/DueEepromEmulationStorage.h
        src/EepromExternalStorage.cpp
        src/EepromExternalStorage.h
        src/EepromInternalStorage.cpp
        src/EepromInternalStorage.h
        src/FlashStorage.cpp
        src/FlashStorage.h
        src/LEDUserInterface.cpp
        src/LEDUserInterface.h
)

add_library(core_library OBJECT

        src/Controller.cpp
        src/Controller.h
        src/Configuration.cpp
        src/Configuration.h
        src/LongMessageService.cpp
        src/Parameters.cpp
        src/Parameters.h
        src/Transport.h
        src/Storage.h
        src/UserInterface.h
        src/Service.h
        src/CbusService.cpp
        src/CbusService.h
        src/LongMessageService.h
        src/MinimumNodeService.cpp
        src/MinimumNodeService.h
        src/CanService.cpp
        src/CanService.h
        src/EventConsumerService.cpp
        src/EventTeachingService.cpp
        src/vlcbdefs.hpp
)

target_link_libraries(core_library PUBLIC)

add_executable(VLCB_Arduino
        $<TARGET_OBJECTS:core_library>
        $<TARGET_OBJECTS:hardware_library>

        Arduino/Arduino.h
        Arduino/Streaming.h
        Arduino/SPI.h
        Arduino/Wire.h
        Arduino/ACAN2515.h
        Arduino/EEPROM.h
        examples/VLCB_1in1out/VLCB_1in1out.ino
        examples/VLCB_empty/VLCB_empty.ino
        examples/VLCB_long_message_example/VLCB_long_message_example.ino)

add_executable(testAll
        $<TARGET_OBJECTS:core_library>
        
        test/ArduinoMock.cpp
        test/TestTools.cpp
        test/testArduino.cpp
        test/testAll.cpp
        test/testMinimumNodeService.cpp
        test/testMinimumNodeService.h
        test/MockUserInterface.cpp
        test/MockUserInterface.h
        test/MockTransport.cpp
        test/MockTransport.h
        test/MockStorage.cpp
        test/MockStorage.h
)
