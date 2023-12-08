if(NOT ARDUINO_USER_LIBS_PATH)
    message(FATAL_ERROR "Arduino-specific variables are not set. \
                         Did you select the right toolchain file?")
endif()

add_library(ArduinoBounce2 STATIC
    ${ARDUINO_USER_LIBS_PATH}/Bounce2/src/Bounce2.cpp
)
target_link_libraries(ArduinoBounce2 PUBLIC ArduinoFlags)
target_link_libraries(ArduinoBounce2 PUBLIC ArduinoCore)
target_link_libraries(ArduinoBounce2 PUBLIC ArduinoLibs)

target_include_directories(ArduinoBounce2 PUBLIC
    ${ARDUINO_USER_LIBS_PATH}/Bounce2/src
)
