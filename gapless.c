/**************************************************************************************[gapless.c]
Copyright (c) 2022 Marijn Heule, Carnegie Mellon University
Last edit: October 8, 2022

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

#define INIT	1000

//#define LINEAR

int binarySearch(long long* map, long long num, int low, int high) {
  while (low != high) {
    int mid = (low + high)/2;
    if (num == map[mid]) {
      return mid;
    }
    else if (num > map[mid]) // num is on the right side
      low  = mid + 1;
    else                     // num is on the left side
      high = mid - 1;
  }
  return low;
}

void printNum (long long num, long long* map, int mapSize, long long offset) {
  if (llabs(num) < offset) {
    printf ("%lli ", num);
    return; }


#ifdef LINEAR
  for (int i = 0; map[i] <= num; i++)
    if (map[i] == num) {
      printf ("%lli ", i + offset + 1);
      return; }
#else
  int pos = binarySearch (map, num, 0, mapSize);
  if (map[pos] == num) {
    printf ("%lli ", pos + offset + 1);
    return;
  }
#endif

  printf ("X");
}

int main (int argc, char** argv) {

  if (argc <= 2) {
    printf("c %s needs two arguments: LRAT file and an offset\n", argv[0]);
    return 0; }

  FILE *input  = fopen (argv[1], "r");
  long long offset = (long long) atoi (argv[2]);

  int mapSize = 0, mapAlloc = INIT;

  long long *map = (long long*) malloc (sizeof (long long) * mapAlloc);

  long long line, lit;
  int del, zeros, size = 0;

  long long max = offset;

//  int limit = 0;
  while (1) {
    int tmp = fscanf (input ," %lli ", &line);
    if (tmp == EOF) break;

    if (tmp == 0) {
      char ignore[1<<16];
      if (fgets (ignore, sizeof (ignore), input) == NULL) printf ("c\n");
      int i;
      for (i = 0; i < sizeof ignore; i++) { if (ignore[i] == '\n') break; }
      if (i == sizeof ignore) {
        printf ("c ERROR: comment longer than %zu characters: %s\n", sizeof ignore, ignore);
        exit (3); }
      continue; }

    if (tmp) size++;
    if (size == 1) {
      if (map[mapSize-1] != line) {
        map[mapSize++] = line; }
      if (mapSize == mapAlloc) {
//        printf ("c realloc map\n");
        mapAlloc *= 2;
        map = realloc (map, sizeof (long long) * mapAlloc); }

      tmp = fscanf (input, " %lli ", &lit);
      if (tmp == 1) { del = 0; zeros = 0; }
      else {
        tmp = fscanf (input, " d %lli ", &lit);
        del = 1; zeros = 1; }
      size++; }

    if (size == 2) {
      if (del == 0) {
        printNum (line, map, mapSize, offset);
        printNum (lit,  map, mapSize, offset);
        if (lit == 0) { printf ("\n"); del  = 1; } }
      else {
        printNum (line, map, mapSize, offset);
        printf ("d ");
        printNum (lit, map, mapSize, offset);
        if (lit == 0) { printf ("\n"); size = 0; } }
    }

    if (size > 2) {
      printNum (line, map, mapSize, offset);
      if (line == 0) {
        zeros++;
        if (zeros == 2)
          printf ("\n"); }
    }

    if (line == 0 && del == 1) size = 0;
    if (line == 0 && del == 0) del  = 1;

//    limit++;
//    if (limit > 2250000) exit (0);
  }

  fclose (input);
}
