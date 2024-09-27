

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")

  add_library(hardware_library OBJECT
    CAN2515.cpp
    LED.cpp
    Switch.cpp
    CreateDefaultStorageForPlatform.cpp
    DueEepromEmulationStorage.cpp
    EepromExternalStorage.cpp
    EepromInternalStorage.cpp
    FlashStorage.cpp
    LEDUserInterface.cpp
    SerialUserInterface.cpp
  )
  target_include_directories(hardware_library PUBLIC .)

  target_link_libraries(hardware_library PUBLIC ArduinoFlags)
  target_link_libraries(hardware_library PUBLIC ArduinoLibs)
  target_link_libraries(hardware_library PUBLIC ArduinoStreaming)
  target_link_libraries(hardware_library PUBLIC ArduinoACAN2515)

endif()

add_library(core_library OBJECT
  Controller.cpp
  Configuration.cpp
  LongMessageService.cpp
  Parameters.cpp
  MinimumNodeService.cpp
  CanService.cpp
  EventConsumerService.cpp
  EventTeachingService.cpp
  EventProducerService.cpp
  NodeVariableService.cpp
  ConsumeOwnEventsService.cpp
  GridConnect.cpp
)
target_include_directories(core_library PUBLIC .)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
  target_link_libraries(core_library PUBLIC ArduinoFlags)
  target_link_libraries(core_library PUBLIC ArduinoLibs)
  target_link_libraries(core_library PUBLIC ArduinoStreaming)
  target_link_libraries(core_library PUBLIC ArduinoACAN2515)
else()
  target_link_libraries(core_library PUBLIC ArduinoFlags)
  target_link_libraries(core_library PUBLIC ArduinoMock)
endif()

source_group(
  TREE ${PROJECT_SOURCE_DIR}
  PREFIX "Header Files"
  FILES ${HEADER_LIST}
)
