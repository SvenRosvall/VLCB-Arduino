if(NOT ARDUINO_PATH)
    message(FATAL_ERROR "Arduino-specific variables are not set. \
                         Did you select the right toolchain file?")
endif()

add_library(ArduinoACAN2515 STATIC
    ${ARDUINO_USER_LIBS_PATH}/ACAN2515/src/ACAN2515.cpp
    ${ARDUINO_USER_LIBS_PATH}/ACAN2515/src/ACAN2515Settings.cpp
)
target_link_libraries(ArduinoACAN2515 PUBLIC ArduinoFlags)
target_link_libraries(ArduinoACAN2515 PUBLIC ArduinoCore)
target_link_libraries(ArduinoACAN2515 PUBLIC ArduinoLibs)

target_include_directories(ArduinoACAN2515 PUBLIC
    ${ARDUINO_USER_LIBS_PATH}/ACAN2515/src
)
