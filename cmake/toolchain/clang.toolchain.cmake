set(CMAKE_C_COMPILER clang CACHE FILEPATH
    "Path to clang" FORCE)
set(CMAKE_CXX_COMPILER clang++ CACHE FILEPATH
    "Path to clang++" FORCE)

include(${CMAKE_CURRENT_LIST_DIR}/host.toolchain.cmake)
