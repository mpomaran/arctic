#=============================================================================#
# Author: QueezyTheGreat                                                      #
# Date:   26.04.2011                                                          #
#                                                                             #
# Description: Arduino CMake example                                          #
#                                                                             #
#=============================================================================#
cmake_minimum_required(VERSION 2.8)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/toolchain/ArduinoToolchain.cmake) # Arduino Toolchain
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
add_definitions(-DARDUINO_ARCH_AVR) # for conditional compilation

#====================================================================#
#  Setup Project                                                     #
#====================================================================#

project(ArcticPantherArduino C CXX)

print_board_list()
print_programmer_list()

add_subdirectory(ArcticPanther)   # add the source directory into build
