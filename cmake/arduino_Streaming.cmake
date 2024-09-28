if(NOT ARDUINO_PATH)
    message(FATAL_ERROR "Arduino-specific variables are not set. \
                         Did you select the right toolchain file?")
endif()

add_library(ArduinoStreaming INTERFACE)

target_include_directories(ArduinoStreaming INTERFACE
    ${ARDUINO_USER_LIBS_PATH}/Streaming/src
)
