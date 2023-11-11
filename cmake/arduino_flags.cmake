if (CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")

# Basic compilation flags
add_library(ArduinoFlags INTERFACE)
target_compile_options(ArduinoFlags INTERFACE
    "-fno-exceptions"
    "-ffunction-sections"
    "-fdata-sections"
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>"
    "-mmcu=${ARDUINO_MCU}"
)
target_compile_definitions(ArduinoFlags INTERFACE
    "F_CPU=${ARDUINO_F_CPU}"
    "ARDUINO=${ARDUINO_VERSION}"
    "ARDUINO_${ARDUINO_BOARD}"
    "ARDUINO_ARCH_AVR"
)
target_link_options(ArduinoFlags INTERFACE
    "-mmcu=${ARDUINO_MCU}"
    "-fuse-linker-plugin"
    "LINKER:--gc-sections"
)
target_compile_features(ArduinoFlags INTERFACE cxx_std_11 c_std_11)

else()

add_library(ArduinoFlags INTERFACE)
target_compile_options(ArduinoFlags INTERFACE
    "-Wall"
    "-Wextra"
    "-Wno-unused-variable"
    "-Wno-unused-parameter"
    "-fno-exceptions"
    "-ffunction-sections"
    "-fdata-sections"
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>"
)
target_compile_definitions(ArduinoFlags INTERFACE
    "ARDUINO=${ARDUINO_VERSION}"
    "ARDUINO_${ARDUINO_BOARD}"
    "__HOST__"
)
target_link_options(ArduinoFlags INTERFACE
#    "-fuse-ld=lld"
    "-fuse-linker-plugin"
    "LINKER:--gc-sections"
)
target_compile_features(ArduinoFlags INTERFACE cxx_std_11 c_std_11)

endif()
