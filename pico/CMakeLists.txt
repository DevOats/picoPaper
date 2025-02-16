# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PicoPaper C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(lib/Config)
add_subdirectory(lib/e-Paper)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)
add_subdirectory(picoDisplay)

# add a header directory
include_directories(picoDisplay)
include_directories(./lib/Config)
include_directories(./lib/GUI)

# Add executable. Default name is the project name, version 0.1
add_executable(PicoPaper PicoPaper.c )

pico_set_program_name(PicoPaper "PicoPaper")
pico_set_program_version(PicoPaper "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(PicoPaper 0)
pico_enable_stdio_usb(PicoPaper 1)

# Add the standard library to the build
target_link_libraries(PicoPaper
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(PicoPaper PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(PicoPaper 
        hardware_timer
        hardware_clocks
        pico_cyw43_arch_none
        hardware_spi
        picoDisplay 
        ePaper 
        GUI 
        Fonts 
        Config
        )

pico_add_extra_outputs(PicoPaper)

