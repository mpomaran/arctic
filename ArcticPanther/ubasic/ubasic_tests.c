#include <tests.h>

#ifdef RUN_UBASIC_UNIT_TESTS

/*
 * Copyright (c) 2006, Adam Dunkels
 * Copyright (c) 2013, Danyil Bohdan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

//#include <time.h>
//#include <stdio.h>
#include <ytypes.h>
#include <yassert.h>
#include <ystring.h>
#include "ubasic.h"


static const char program_fibs[] =
"2 let a = 1\n\
4 let b = 1\n\
6 for i = 0 to 8\n\
7 let b = a + b\n\
8 let a = b - a\n\
8 next i\n\
10 end\n";

#ifndef ARDUINO_ARCH_AVR

static const char program_let[] =
"1 let a = 42\n\
2 end\n";

static const char program_loop[] =
"1 for i = 0 to 12\n\
2 for j = 0 to 16\n\
3 for k = 0 to 10\n\
4 let a = i * j * k\n\
5 next k\n\
6 next j\n\
7 next i\n\
8 end\n";

static const char program_goto[] =
"10 goto 50\n\
20 goto 40\n\
30 goto 60\n\
40 goto 30\n\
50 goto 20\n\
60 let c = 108\n\
70 end\n";

static const char program_peek_poke[] =
"10 peek 100 + 20 + 3, a\n\
20 peek 123, z\n\
30 poke 100 - 1, 99\n\
40 poke 0, 0\n\
50 end\n";

#endif

/*---------------------------------------------------------------------------*/
VARIABLE_TYPE peek(VARIABLE_TYPE arg) {
    return arg;
}

/*---------------------------------------------------------------------------*/
void poke(VARIABLE_TYPE arg, VARIABLE_TYPE value) {
    yassert(arg == value);
}

/*---------------------------------------------------------------------------*/
void run(const char program[]) {
  static uint8_t test_num = 0;
  test_num++;
  ytrace(test_num);

  ubasic_init_peek_poke(program, &peek, &poke);

  ytracestr(program);

  do {
    ubasic_run();
  } while(!ubasic_finished());
}

/*---------------------------------------------------------------------------*/
void
ubasic_unit_tests(void)
{
  run(program_fibs);
  yassert(ubasic_get_variable(1) == 89);

#ifndef ARDUINO_ARCH_AVR
  run(program_let);
  ytrace(ubasic_get_variable(0));
  yassert(ubasic_get_variable(0) == 42);

  run(program_loop);
  yassert(ubasic_get_variable(0) == (VARIABLE_TYPE)(12L * 16L * 10L));

  run(program_goto);
  yassert(ubasic_get_variable(2) == 108);


  run(program_peek_poke);
  yassert(ubasic_get_variable(0) == 123);
  yassert(ubasic_get_variable(25) == 123);
#endif
}
/*---------------------------------------------------------------------------*/
#endif