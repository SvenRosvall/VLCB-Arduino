# User settings with sensible defaults
set(ARDUINO_PATH "$ENV{HOME}/.arduino15/packages/arduino" CACHE PATH
    "Path of the Arduino packages folder, e.g. ~/.arduino15/packages/arduino.")
set(ARDUINO_USER_PATH "$ENV{HOME}/Arduino" CACHE PATH
    "Path of the Arduino user folder")
set(ARDUINO_CORE_VERSION "1.8.6" CACHE STRING
    "Version of arduino/ArduinoCore-AVR")

set(ARDUINO_AVR_PATH ${ARDUINO_PATH}/hardware/avr/${ARDUINO_CORE_VERSION})
set(ARDUINO_CORE_PATH ${ARDUINO_AVR_PATH}/cores/arduino)
set(ARDUINO_LIBS_PATH ${ARDUINO_AVR_PATH}/libraries)
set(ARDUINO_USER_LIBS_PATH ${ARDUINO_USER_PATH}/libraries)

# Only look libraries etc. in the sysroot, but never look there for programs
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
