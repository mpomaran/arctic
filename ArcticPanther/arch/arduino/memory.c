#ifdef ARDUINO_ARCH_AVR

#include "memory.h"
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t len)
{
        size_t i;

        /*
         * memcpy does not support overlapping buffers, so always do it
         * forwards. (Don't change this without adjusting memmove.)
         *
         * For speedy copying, optimize the common case where both pointers
         * and the length are word-aligned, and copy word-at-a-time instead
         * of byte-at-a-time. Otherwise, copy by bytes.
         *
         * The alignment logic below should be portable. We rely on
         * the compiler to be reasonably intelligent about optimizing
         * the divides and modulos out. Fortunately, it is.
         */

        if ((uintptr_t)dst % sizeof(long) == 0 &&
            (uintptr_t)src % sizeof(long) == 0 &&
            len % sizeof(long) == 0) {

                long *d = dst;
                const long *s = src;

                for (i=0; i<len/sizeof(long); i++) {
                        d[i] = s[i];
                }
        }
        else {
                char *d = dst;
                const char *s = src;

                for (i=0; i<len; i++) {
                        d[i] = s[i];
           }
   }

         return dst;
}

#else
#error This file is for arduino only
#endif
