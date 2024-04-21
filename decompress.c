/**********************************************************************************[decompress.c]
Copyright (c) 2020-2024 Marijn Heule, Carnegie Mellon University
Last edit: March 9, 2024

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define ADD	-48
#define DEL	50
#define MODE	1	// DRAT: MODE=1; LRAT: MODE=2

#ifdef LONGTYPE
  typedef long long ltype;
#else
  typedef int ltype;
#endif

int read_lit (FILE* input, ltype *lit) {
  ltype l = 0;
  int lc, shift = 0;
  do {
    lc = getc_unlocked (input);
    if ((shift == 0) && (lc == EOF)) return EOF;
    printf ("%i\n", lc);
    l |= (ltype) (lc & 127) << shift;
    shift += 7; }
  while (lc > 127);
  if (l % 2) *lit = (l >> 1) * -1;
  else       *lit = (l >> 1);
  return 1; }

int main (int argc, char** argv) {
  int mode = MODE;
  FILE *input;

  if (argc <= 1) {
    printf("c %s needs one argument: an input in compressed DRAT/LRAT format\n", argv[0]);
    return 0; }


  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] == 'm') {
      mode = 3 - mode; }
    else {
      input = fopen (argv[i], "r");
      if (input == NULL) { printf ("c ERROR opening %s\n", argv[i]); exit (1); } }
    }

  ltype lit, index;

  while (1) {
    read_lit (input, &lit);
    if (lit == ADD) {
      int zeros = 0;
      read_lit (input, &lit);
      index = lit;
      printf ("%lli ", (long long) index);
      while (zeros < mode) {
        read_lit (input, &lit);
        if (lit == 0) zeros++;
        if (zeros == mode) printf ("0\n");
        else printf ("%lli ", (long long) lit); }
    }
    else if (lit == DEL) {
      if (mode == 2) {
        printf ("%lli ", (long long) index); }
      printf ("d ");
      int zeros = 0;
      while (zeros < 1) {
        read_lit (input, &lit);
        if (lit == 0) zeros++;
        if (zeros == 1) printf ("0\n");
        else printf ("%lli ", (long long) lit); }
    }
    else break;
  }
}
