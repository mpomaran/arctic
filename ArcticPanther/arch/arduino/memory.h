#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#ifdef ARDUINO_ARCH_AVR

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *dst, const void *src, size_t len);

#ifdef __cplusplus
}
#endif

#endif

#endif
