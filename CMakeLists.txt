# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==

# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Revisao C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Revisao Revisao.c inc/ssd1306.c )

pico_set_program_name(Revisao "Revisao")
pico_set_program_version(Revisao "0.1")

pico_generate_pio_header(Revisao ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Revisao 1)
pico_enable_stdio_usb(Revisao 1)

# Add the standard library to the build
target_link_libraries(Revisao
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Revisao PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(Revisao 
        hardware_i2c
        hardware_pio
        hardware_pwm
        hardware_adc
        )

pico_add_extra_outputs(Revisao)

