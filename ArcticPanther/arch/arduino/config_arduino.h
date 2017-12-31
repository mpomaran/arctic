#ifndef ARDUINO_ARCH_AVR
#error Wrong config!!!
#endif

#ifndef YCONFIG_H_HAS_BEEN_INCLUDED
#define YCONFIG_H_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include <ytypes.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#define read_uint32(ptr) pgm_read_dword_near(ptr)
#define read_uint8(ptr) pgm_read_byte_near(ptr)
#define read_ptr(ptr) ((void *)pgm_read_word(ptr))

typedef enum {
	Y_LOW = LOW,
	Y_HIGH = HIGH
} pin_state_t;

#ifdef __cplusplus
}
#endif

#endif
