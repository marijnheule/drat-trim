/************************************************************************************[lrat-check.c]
Copyright (c) 2017 Marijn Heule, The University of Texas at Austin.
Last edit, June 10, 2017

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

#define ADD		1
#define DEL		0
#define SUCCESS		1
#define FAILED		0
#define CNF		100
#define LRAT		200
#define CLRAT		300

long long *mask, now;

int *clsList, clsAlloc;
int *table, tableSize, tableAlloc, maskAlloc;

int  getType   (int* list) { return list[0]; }
int  getIndex  (int* list) { return list[1]; }
int  getLength (int* list) { int i = 2; while (list[i]) i++; return i - 2; }
int* getHints  (int* list) { return list + getLength (list) + 1; }
int  getRATs   (int* list) { int c = 0; while (*list) if ((*list++) < 0) c++; return c; }

int convertLit (int lit)   { return (abs(lit) * 2) + (lit < 0); }

int checkRedundancy (int *hints, int thisMask) {
  int res = abs(*hints++);

  if (res > 0) {
    int unit = 0, *clause = table + clsList[res];
    while (*clause) {
      int clit = convertLit (*clause++);
      if (mask[clit] >= thisMask) continue; // lit is falsified
      if (unit != 0) return FAILED;
      unit = clit; }
    if (unit == 0) return SUCCESS;
    mask[unit ^ 1] = thisMask; }

  while (*hints > 0) {
    int unit = 0, *clause = table + clsList[*hints++];
    while (*clause) {
      int clit = convertLit (*clause++);
      if (mask[clit] >= now) continue; // lit is falsified
      if (unit != 0) return FAILED;
      unit = clit; }
    if (unit == 0) return SUCCESS;
    mask[unit ^ 1] = thisMask; }

  if (res == 0) return SUCCESS;
  return FAILED; }

int checkClause (int* list, int size, int* hints) {
  now++;
  int i, pivot = convertLit (list[0]);
  int RATs = getRATs (hints);
  for (i = 0; i < size; i++) {
    int clit = convertLit (list[i]);
    if (clit > maskAlloc) {
      maskAlloc = (clit * 3) >> 1;
      mask = (long long *) realloc (mask, sizeof(long long) * maskAlloc); }
    mask [clit] = now + RATs; }

  int res = checkRedundancy (hints, now + RATs);
  if (res  == FAILED) return FAILED;
  if (RATs == 0)      return SUCCESS;

  hints++;
  while (*hints) {
    while (*hints > 0) hints++;
    now++;
    if (checkRedundancy (hints, now) == FAILED) return FAILED; }

  return SUCCESS; }

void printClause (int* clause) {
  while (*clause) printf ("%i ", *clause++);
  printf ("0\n"); }

void addClause (int index, int* literals, int size) {
  if (index >= clsAlloc) {
    clsAlloc = (index * 3) >> 1;
    clsList = (int*) realloc (clsList, sizeof(int) * clsAlloc); }

  if (tableSize + size >= tableAlloc) {
    tableAlloc = (tableAlloc * 3) >> 1;
    table = (int*) realloc (table, sizeof (int) * tableAlloc); }

  clsList[index] = tableSize;
  int i; for (i = 0; i < size; i++) table[tableSize++] = literals[i]; }

void deleteClauses (int* list) {
  while (*list) {
    int index = *list++;
    if (table[index] == 0) {
      printf ("c WARNING: clause %i is already deleted\n", index); }
    table[index] = 0; } }

int parseLine (FILE* file, int *list, int mode) {
  int lit, tmp, count = 0;

  if (mode == CNF) {
    while (1) {
      tmp = fscanf (file, " %i ", &lit);
      if (tmp == 0 || tmp == EOF) return 0;
      list[count++] = lit;
      if (lit == 0) return count; } }

  return 0; }

int main (int argc, char** argv) {
  now = 1;

  int nVar, nCls;
  FILE* cnf   = fopen (argv[1], "r");
  fscanf (cnf, " p cnf %i %i ", &nVar, &nCls);

  clsAlloc = nCls * 2;
  clsList  = (int*) malloc (sizeof(int) * clsAlloc);

  tableSize  = 0;
  tableAlloc = nCls * 2;
  table = (int *) malloc (sizeof(int) * tableAlloc);

  int* list = (int*) malloc (sizeof (int) * nVar * 10);
  int index = 1;
  while (1) {
    int size = parseLine (cnf, list, CNF);
    if (size == 0) break;
    printClause (list);
    addClause (index++, list, size); }
  fclose (cnf);

  maskAlloc = 2 * nVar;
  mask = (long long*) malloc (sizeof(long long) * maskAlloc);

  FILE *proof = fopen (argv[2], "r");
  int mode = CLRAT;
  while (1) {
    int size = parseLine (proof, list, mode);
    if (size == 0) break;

    printClause (list);

    if (getType (list) == DEL) {
      deleteClauses (list + 2); }
    else if (getType (list) == ADD) {
      int index  = getIndex  (list);
      int length = getLength (list);
      int *hints = getHints  (list);

      if (checkClause (list + 2, length, hints) == SUCCESS) {
        addClause (index, list + 2, length); }
      else {
        printf("c failed to check clause: ");
        printClause (list + 2);
        printf("c NOT VERIFIED\n");
        exit (0); }

      if (length == 0) {
        printf ("c VERIFIED\n");
        exit (1); }
    }
    else {
      printf ("c failed type\n");
      exit (0); }
  }
}
