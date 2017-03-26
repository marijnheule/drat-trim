#include <stdio.h>
#include <stdlib.h>

int write_lit (FILE *output, int lit) {
  unsigned int l = abs (lit) << 1;
  if (lit < 0) l++;

  do {
    if (l <= 127) { fputc ((char)                 l, output); }
    else          { fputc ((char) (128 + (l & 127)), output); }
    l = l >> 7;
  } while (l);

  return 1; }

int main (int argc, char** argv) {

  if (argc <= 2) {
    printf("needs two arguments: an input and an output file\n");
    return 0; }

  FILE *input  = fopen (argv[1], "r");
  FILE *output = fopen (argv[2], "w");

  int tmp, line, lit, del, size = 0;

  while (1) {
    tmp = fscanf (input ," %i ", &line);
    if (tmp == EOF) break;
    if (tmp) size++;
    if (size == 1) {
      tmp = fscanf (input, " %i ", &lit);
      if (tmp == 1) { del = 0; }
      else { tmp = fscanf (input, " d %i ", &lit); del = 1; }
      size++; }

    if (size == 2) {
      if (del == 0) { fputc ('a', output); write_lit (output, line); write_lit (output, lit); }
      else          { fputc ('d', output); /*write_lit (output, line);*/ write_lit (output, lit); } }

    if (size > 2) write_lit (output, line);

    if (line == 0 && del == 1) size = 0;
    if (line == 0 && del == 0) del  = 1; }

  close (input);
  return 1;
}
