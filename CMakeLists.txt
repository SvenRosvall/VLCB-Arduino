cmake_minimum_required(VERSION 3.25)
project(VLCB_Arduino)

set(CMAKE_CXX_STANDARD 14)

include_directories(examples)
include_directories(examples/CBUS_1in1out)
include_directories(examples/CBUS_empty)
include_directories(examples/CBUS_long_message_example)
include_directories(src)
include_directories(Arduino)

add_executable(VLCB_Arduino

        src/Controller.cpp
        src/Controller.cpp
        src/Controller.h
        src/CAN2515.cpp
        src/CAN2515.h
        src/Configuration.cpp
        src/Configuration.h
        src/LED.cpp
        src/LED.h
        src/LongMessageService.cpp
        src/Parameters.cpp
        src/Parameters.h
        src/Switch.cpp
        src/Switch.h
        src/Transport.h
        src/CreateDefaultStorageForPlatform.cpp
        src/DueEepromEmulationStorage.cpp
        src/DueEepromEmulationStorage.h
        src/EepromExternalStorage.cpp
        src/EepromExternalStorage.h
        src/EepromInternalStorage.cpp
        src/EepromInternalStorage.h
        src/FlashStorage.cpp
        src/FlashStorage.h
        src/Storage.h
        src/LEDUserInterface.cpp
        src/LEDUserInterface.h
        src/UserInterface.h
        Arduino/Arduino.h
        Arduino/Streaming.h
        Arduino/SPI.h
        Arduino/Wire.h
        Arduino/ACAN2515.h
        Arduino/EEPROM.h
        examples/VLCB_1in1out/VLCB_1in1out.ino
        examples/VLCB_empty/VLCB_empty.ino
        examples/VLCB_long_message_example/VLCB_long_message_example.ino
        src/Service.h
        src/CbusService.cpp
        src/CbusService.h
        src/LongMessageService.h
        src/initializer_list.h
        src/MinimumNodeService.cpp
        src/MinimumNodeService.h
        src/CanService.cpp
        src/CanService.h
        src/vlcbdefs.hpp)
add_executable(testAll
        test/ArduinoMock.cpp
        test/TestTools.cpp
        test/testArduino.cpp
        test/testAll.cpp)
