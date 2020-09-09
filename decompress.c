/**********************************************************************************[decompress.c]
Copyright (c) 2020 Marijn Heule, Carnegie Mellon University
Last edit: September 5, 2020

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

int read_lit (FILE* input, int *lit) {
  int l = 0, lc, shift = 0;
  do {
    lc = getc_unlocked (input);
    if ((shift == 0) && (lc == EOF)) return EOF;
    l |= (lc & 127) << shift;
    shift += 7; }
  while (lc > 127);
  if (l % 2) *lit = (l >> 1) * -1;
  else       *lit = (l >> 1);
  return 1; }


int main (int argc, char** argv) {

  if (argc <= 2) {
    printf("c %s needs two arguments: an input and an output file\n", argv[0]);
    return 0; }

  FILE *input  = fopen (argv[1], "r");
  if (input == NULL) { printf ("c ERROR opening %s\n", argv[1]); exit (1); }

  FILE *output = fopen (argv[2], "w");
  if (output == NULL) { printf ("c ERROR opening %s\n", argv[2]); exit (2); }

  int lit, index;

  while (1) {
    read_lit (input, &lit);
    if (lit == ADD) {
      int zeros = 0;
      read_lit (input, &lit);
      index = lit;
      printf ("%i ", index);
      while (zeros < 2) {
        read_lit (input, &lit);
        if (lit == 0) zeros++;
        if (zeros == 2) printf ("0\n");
        else printf ("%i ", lit); }
    }
    else if (lit == DEL) {
      printf ("%i d ", index);
      int zeros = 0;
      while (zeros < 1) {
        read_lit (input, &lit);
        if (lit == 0) zeros++;
        if (zeros == 1) printf ("0\n");
        else printf ("%i ", lit); }
    }
    else break;
  }
}
