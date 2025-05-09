# CMake definition for VLCB-Arduino project
# This is necessary for working with the CLion IDE.

cmake_minimum_required(VERSION 3.25)
project(VLCB_Arduino)

set(CMAKE_CXX_STANDARD 11)

include_directories(examples)
include_directories(examples/VLCB_1in1out)
include_directories(examples/VLCB_4in4out)
include_directories(examples/VLCB_empty)
include_directories(examples/VLCB_long_message_example)
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
        src/SerialUserInterface.cpp
        src/SerialUserInterface.h
        src/SerialGC.h
        src/SerialGC.cpp
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
        src/CanTransport.h
        src/Storage.h
        src/Service.h
        src/Service.cpp
        src/LongMessageService.h
        src/MinimumNodeService.cpp
        src/MinimumNodeService.h
        src/MinimumNodeServiceWithDiagnostics.h
        src/MinimumNodeServiceWithDiagnostics.cpp
        src/CanService.cpp
        src/CanService.h
        src/CanServiceWithDiagnostics.cpp
        src/CanServiceWithDiagnostics.h
        src/EventConsumerService.cpp
        src/EventConsumerService.h
        src/EventTeachingService.cpp
        src/EventTeachingService.h
        src/EventProducerService.cpp
        src/EventProducerService.h
        src/NodeVariableService.cpp
        src/NodeVariableService.h
        src/vlcbdefs.hpp
        src/ConsumeOwnEventsService.h
        src/ConsumeOwnEventsService.cpp
        src/GridConnect.cpp
        src/GridConnect.h
        src/CircularBuffer.h
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
        examples/VLCB_long_message_example/VLCB_long_message_example.ino
        examples/VLCB_SerialGC_1in1out/VLCB_SerialGC_1in1out.ino)

add_executable(testAll
        $<TARGET_OBJECTS:core_library>
        
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
        test/MockCanTransport.h
        test/MockTransportService.cpp
        test/MockTransportService.h
        test/VlcbCommon.cpp
        test/testEventProducerService.cpp
        test/testEventConsumerService.cpp
        test/testEventTeachingService.cpp
        test/testConsumeOwnEventsService.cpp
        test/testLongMessageService.cpp
        test/testGridConnect.cpp
        test/testConfiguration.cpp
        test/testCircularBuffer.cpp
        test/MockUserInterface.h
)
