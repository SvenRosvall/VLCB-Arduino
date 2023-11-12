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
    "-fuse-ld=lld"
    "-fuse-linker-plugin"
    "LINKER:--gc-sections"
)
target_compile_features(ArduinoFlags INTERFACE cxx_std_11 c_std_11)

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(isMultiConfig)
    if(NOT "Asan" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Asan)
    endif()
    if(NOT "Coverage" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Coverage)
    endif()
else()
    set(allowedBuildTypes Asan Coverage Debug Release RelWithDebInfo MinSizeRel)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

    if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

set(CMAKE_C_FLAGS_ASAN
    "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined" CACHE STRING
    "Flags used by the C compiler for Asan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_ASAN
    "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined" CACHE STRING
    "Flags used by the C++ compiler for Asan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_ASAN
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined" CACHE STRING
    "Linker flags to be used to create executables for Asan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_ASAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined" CACHE STRING
    "Linker flags to be used to create shared libraries for Asan build type." FORCE)

set(CMAKE_C_FLAGS_COVERAGE
    "${CMAKE_C_FLAGS_DEBUG} -fprofile-instr-generate -fcoverage-mapping" CACHE STRING
    "Flags used by the C compiler for Asan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_COVERAGE
    "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-instr-generate -fcoverage-mapping" CACHE STRING
    "Flags used by the C++ compiler for Asan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fprofile-instr-generate" CACHE STRING
    "Linker flags to be used to create executables for Asan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fprofile-instr-generate" CACHE STRING
    "Linker flags to be used to create shared libraries for Asan build type." FORCE)

endif()
