# This CMake configuration file is created on example
# provided with the CMake build system available here:
# https://github.com/queezythegreat/arduino-cmake
#

cmake_minimum_required(VERSION 2.8)

set(ARDUINO_DEFAULT_BOARD uno) # Default Board ID, when not specified
# set(ARDUINO_DEFAULT_PORT /dev/ttyUSB0) # Default Port, when not specified
set(ARDUINO_DEFAULT_PROGRAMMER arduinoisp)

#====================================================================#
# Build ylib
#====================================================================#

include_directories (arch/arduino pt-1.4 ubasic)

set (UBASIC_LIB_HEADERS
  ubasic/ubasic.h
)

set (UBASIC_LIB_SOURCES
  ubasic/ubasic_tests.c
  ubasic/ubasic_tokenizer.c
  ubasic/ubasic_use-ubasic.c
  ubasic/ubasic.c
)

set (LIB_HEADERS
  config.h
  arch/arduino/config_arduino.h
  arch/arduino/memory.h
  arch/arduino/AltSoftSerial.h
  arch/arduino/AltSoftSerial_Boards.h
  arch/arduino/AltSoftSerial_Timers.h
  arch/arduino/LowPower.h
  pt-1.4/lc.h
  pt-1.4/lc-addrlabels.h
  pt-1.4/lc-switch.h
  pt-1.4/pt.h
  pt-1.4/pt-sem.h
  yscript.h
  yassert.h
  ygsmmodem.h
  yparser.h
  yserial.h
  ytimer.h
  ycircbuf.h
  ytypes.h
  tests.h
  ycollection.h
  ycollection_private.h
  ygsmmodem.h
  ystring.h
  yparser.h
  yserial.h
  ysim900_emulator.h
  ytimer.h
  ymessage.h
)

set (LIB_SOURCES
  arch/arduino/memory.c
  arch/arduino/yserial_arduino.cpp
  arch/arduino/AltSoftSerial.cpp
  arch/arduino/LowPower.cpp
  arch/arduino/yhal_arduino.c
  arch/arduino/yhal_lowpower.cpp
  arch/arduino/yeeprom.c
  yscript.c
  ygsmmodem.c
  ymessage.c
  ygsmmodem_unit_tests.c
  yparser.c
  yparser_unit_test.c
  ytimer.c
  ycircbuf.c
  ycircbuf_unit_tests.c
  yserial_unit_tests.c
  ytimer_unit_test.c
  tests.c
  ycollection_private.c
  ygsmmodem.c
  ylist.c
  ylist_test_suite.c
  yparser.c
  yparser_unit_test.c
  yserial_unit_tests.c
  ysim900_emulator.c
  ysim900_emulator_unit_tests.c
  ytimer.c
  ytimer_unit_test.c
  ystring_unit_tests.c
  ymessage_unit_tests.c
)

generate_arduino_library(ylib
    BOARD uno
    SRCS ${LIB_SOURCES}
    HDRS  ${LIB_HEADERS}
    )

generate_arduino_library(ubasic
  BOARD uno
  SRCS ${UBASIC_LIB_SOURCES}
  HDRS ${UBASIC_LIB_HEADERS}
  )

#====================================================================#
# Build firmware
#====================================================================#
generate_arduino_firmware(arctic_panther
    SRCS ArcticPanther.cpp
    LIBS ylib ubasic
    BOARD uno
    PROGRAMMER arduinoisp
    )
set(${TARGET_NAME}_PROGRAMMER programmer_id)
