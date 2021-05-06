#=============================================================================#
# Author: QueezyTheGreat                                                      #
# Date:   26.04.2011                                                          #
#                                                                             #
# Description: Arduino CMake example                                          #
#                                                                             #
#=============================================================================#
cmake_minimum_required(VERSION 3.0...3.19)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/toolchain/ArduinoToolchain.cmake) # Arduino Toolchain
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
add_definitions(-DARDUINO_ARCH_AVR) # for conditional compilation

foreach(PLATFORM ${ARDUINO_PLATFORMS})
set(ARDUINO_BOARD_LIST ${ARDUINO_BOARD_LIST} ${${PLATFORM}_BOARDS})
endforeach()
set(ARDUINO_DEFAULT_BOARD uno CACHE STRING "Default Board") # Default Board ID, when not specified
set_property(CACHE ARDUINO_DEFAULT_BOARD PROPERTY STRINGS ${ARDUINO_BOARD_LIST})

set(${FIRMWARE_NAME}_PORT com3)

#====================================================================#
#  Setup Project                                                     #
#====================================================================#

project(ArcticPantherArduino C CXX)

print_board_list()
print_programmer_list()

add_subdirectory(ArcticPanther)   # add the source directory into build
