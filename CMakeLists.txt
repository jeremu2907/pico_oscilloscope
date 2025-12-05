cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

set(sdkVersion 2.2.0)
set(toolchainVersion 14_3_rel1)
set(picotoolVersion 2.2.0-a4)
# ====================================================================================
# Valid values:
#   pico
#   pico2
#   pico_w
#   pico2_w
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Valid values:
#   PICO
#   PICO_W
#   PICO_2
#   PICO_2_W
add_compile_definitions(PICO_BOARD_TYPE=PICO_2)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(/root/pico-sdk/external/pico_sdk_import.cmake)

project(Main C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add pico_lib dir
add_subdirectory(../pico_lib build)

add_executable(Main
    main.cpp
)

pico_set_program_name(Main "Main")
pico_set_program_version(Main "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Main 1)
pico_enable_stdio_usb(Main 1)

set(BASE_LIBS
    Shared
)

if((PICO_BOARD STREQUAL "pico_w") OR (PICO_BOARD STREQUAL "pico2_w"))
    list(APPEND BASE_LIBS pico_cyw43_arch_none)
endif()

# Add the standard library to the build
target_link_libraries(Main ${BASE_LIBS})

# Add the standard include files to the build
target_include_directories(Main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/pico_lib/includes
)

pico_add_extra_outputs(Main)
