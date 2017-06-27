#include <stdio.h>
#include <stdlib.h>

#define EMPTY	-1

int table_size, table_alloc, lookup_size, lookup_alloc, *table, *lookup;

void write_lit (FILE *output, int lit, int sort) {
  if (sort == 0) {
    unsigned int l = abs (lit) << 1;
    if (lit < 0) l++;

    do {
      if (l <= 127) { fputc ((char)                 l, output); }
      else          { fputc ((char) (128 + (l & 127)), output); }
      l = l >> 7;
    } while (l); }
  else {
    if (table_size >= table_alloc) {
      table_alloc = (table_alloc * 3) >> 1;
      table = (int*) realloc (table, sizeof(int) * table_alloc); }
    table[ table_size++ ] = lit; }
}

int main (int argc, char** argv) {

  if (argc <= 2) {
    printf("needs two arguments: an input and an output file\n");
    return 0; }

  FILE *input  = fopen (argv[1], "r");
  FILE *output = fopen (argv[2], "w");

  int i, tmp, line, lit, del, size = 0;
  int sort = 1;

  if (sort) {
    lookup_size  =    0;
    lookup_alloc = 1000;
    lookup = (int*) malloc (sizeof (int) * lookup_alloc);
    for (i = 0; i < lookup_alloc; i++) lookup[i] = EMPTY;
    table_size   =    0;
    table_alloc  = 1000;
    table = (int*) malloc (sizeof (int) * table_alloc); }

  while (1) {
    tmp = fscanf (input ," %i ", &line);
    if (tmp == EOF) break;
    if (tmp) size++;
//    printf("%i (%i) ", size, line);
    if (size == 1) {
      tmp = fscanf (input, " %i ", &lit);
      if (tmp == 1) { del = 0; }
      else { tmp = fscanf (input, " d %i ", &lit); del = 1; }
      size++; }

    if (size == 2) {
      if (sort == 1) {
        int entry = 2 * line + del;
        if (entry >  lookup_size) lookup_size = entry;
        if (entry >= lookup_alloc) {
          int old = lookup_alloc;
          lookup_alloc = 3 * entry >> 1;
          lookup = (int*) realloc (lookup, sizeof (int) * lookup_alloc);
          for (i = old; i < lookup_alloc; i++) lookup[i] = EMPTY; }
//        printf("c adding entry %i (%i, %i, %i, %i)\n", entry, line, lit, del, size);
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
//    printf ("c printing line %i\n", i);
    if (i % 2) { fputc ('d', output); zeros = 1; }
    else       { fputc ('a', output); zeros = 2; }
    int *list = table + lookup[i];
    while (zeros) {
      while (*list) write_lit (output, *list++, 0);
      write_lit (output, *list++, 0);
      zeros--; } }
}
