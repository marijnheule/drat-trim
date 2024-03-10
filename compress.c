/************************************************************************************[compress.c]
Copyright (c) 2020-2023 Marijn Heule, Carnegie Mellon University
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

#define EMPTY	-1
#define INIT	1000

#ifdef LONGTYPE
  typedef long long ltype;
#else
  typedef int ltype;
#endif

long long table_size, table_alloc, *lookup;
ltype lookup_size, lookup_alloc, *table;

int abscompare (const void *a, const void *b) {
#ifdef LONGTYPE
  return (llabs(*(ltype*)a) - llabs(*(ltype*)b)); }
#else
  return (abs(*(ltype*)a) - abs(*(ltype*)b)); }
#endif

void write_lit (FILE *output, ltype lit, int sort) {
  if (sort == 0) {
    unsigned long long l = llabs (lit) << 1;
    if (lit < 0) l++;

    do {
      if (l <= 127) { fputc ((char)                 l, output); }
      else          { fputc ((char) (128 + (l & 127)), output); }
      l = l >> 7;
    } while (l); }
  else {
    if (table_size >= table_alloc) {
      table_alloc = (table_alloc * 3) >> 1;
      table = (ltype*) realloc (table, sizeof(ltype) * table_alloc); }
    table[ table_size++ ] = lit; }
}

int main (int argc, char** argv) {

  if (argc <= 2) {
    printf("c %s needs two arguments: an input and an output file\n", argv[0]);
    return 0; }

  FILE *input  = fopen (argv[1], "r");
  FILE *output = fopen (argv[2], "w");

  int i, tmp, line, lit, del, size = 0;
  int sort = 1;

  if (sort) {
    lookup_size  =    0;
    lookup_alloc = INIT;
    lookup = (long long*) malloc (sizeof (long long) * lookup_alloc);
    for (i = 0; i < lookup_alloc; i++) lookup[i] = EMPTY;
    table_size   =    0;
    table_alloc  = INIT;
    table = (ltype*) malloc (sizeof (ltype) * table_alloc); }

  while (1) {
    tmp = fscanf (input ," %i ", &line);
    if (tmp == EOF) break;

    if (tmp == 0) {
      char ignore[1<<16];
      if (fgets (ignore, sizeof (ignore), input) == NULL) printf ("c\n");
      for (i = 0; i < sizeof ignore; i++) { if (ignore[i] == '\n') break; }
      if (i == sizeof ignore) {
        printf ("c ERROR: comment longer than %zu characters: %s\n", sizeof ignore, ignore);
        exit (3); }
      continue; }

    if (tmp) size++;
    if (size == 1) {
      tmp = fscanf (input, " %i ", &lit);
      if (tmp == 1) { del = 0; }
      else { tmp = fscanf (input, " d %i ", &lit); del = 1; }
      size++; }

    if (size == 2) {
      if (sort == 1) {
        ltype entry = 2 * line + del;
        if (entry >  lookup_size) lookup_size = entry;
        if (entry >= lookup_alloc) {
          ltype old = lookup_alloc;
          lookup_alloc = 3 * entry >> 1;
          lookup = (long long*) realloc (lookup, sizeof (long long) * lookup_alloc);
          for (i = old; i < lookup_alloc; i++) lookup[i] = EMPTY; }
        lookup[entry] = table_size; }
      if (del == 0) { if (sort == 0) fputc ('a', output); write_lit (output, line, sort); write_lit (output, lit, sort); if (lit == 0) del = 1; }
      else          { if (sort == 0) fputc ('d', output); write_lit (output, lit,  sort); if (lit == 0) size = 0; } }

    if (size > 2) write_lit (output, line, sort);

    if (line == 0 && del == 1) size = 0;
    if (line == 0 && del == 0) del  = 1; }

  fclose (input);

  if (sort == 0) return 1;

  for (i = 1; i <= lookup_size; i++) {
    int zeros = 0;
    if (lookup[i] == EMPTY) continue;
    if (i % 2) { fputc ('d', output); zeros = 1; }
    else       { fputc ('a', output); zeros = 2; }
    ltype *list = table + lookup[i];

    if (zeros == 2) {
      write_lit (output, *list++, 0); // write index;
      if (*list) write_lit (output, *list++, 0); // write pivot;

      ltype* start = list; int count = 0;
      while (*list) { count++; list++; }
//      qsort (start, count, sizeof (ltype), abscompare);

      list = start;
      while (*list) write_lit (output, *list++, 0);
      write_lit (output, *list++, 0);
      zeros--;
    }

    if (zeros == 1) {
      while (*list) write_lit (output, *list++, 0);
      write_lit (output, *list++, 0); }
  }
}
