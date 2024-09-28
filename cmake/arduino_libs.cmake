if(NOT ARDUINO_PATH)
    message(FATAL_ERROR "Arduino-specific variables are not set. \
                         Did you select the right toolchain file?")
endif()

add_library(ArduinoLibs STATIC
    ${ARDUINO_LIBS_PATH}/SPI/src/SPI.cpp
    ${ARDUINO_LIBS_PATH}/HID/src/HID.cpp
    ${ARDUINO_LIBS_PATH}/SoftwareSerial/src/SoftwareSerial.cpp
    ${ARDUINO_LIBS_PATH}/Wire/src/Wire.cpp
    ${ARDUINO_LIBS_PATH}/Wire/src/utility/twi.c
)
target_link_libraries(ArduinoLibs PUBLIC ArduinoFlags)
target_link_libraries(ArduinoLibs PUBLIC ArduinoCore)

target_compile_features(ArduinoLibs PUBLIC cxx_std_11 c_std_11)
target_include_directories(ArduinoLibs PUBLIC
    ${ARDUINO_LIBS_PATH}/SPI/src
    ${ARDUINO_LIBS_PATH}/HID/src
    ${ARDUINO_LIBS_PATH}/SoftwareSerial/src
    ${ARDUINO_LIBS_PATH}/Wire/src
    ${ARDUINO_LIBS_PATH}/EEPROM/src
)
