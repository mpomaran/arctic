#include <ytypes.h>

/* below code comes from the wikipedia */

static int64_t seed = 0;

uint64_t yrand_rand(void) {
  seed = seed * 1664525L + 1013904223L;

  return seed & 0xFFFFFFFFL;
}

void yrand_seed(uint64_t a, uint64_t b) { seed = (int64_t)(a + b); }
