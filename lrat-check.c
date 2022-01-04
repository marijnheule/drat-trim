/************************************************************************************[lrat-check.c]
Copyright (c) 2017-2021 Marijn Heule, Carnegie Mellon University
Last edit: September 12, 2021

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
#include <assert.h>
#include <sys/time.h>

#define DELETED		-1
#define SUCCESS		1
#define FAILED		0
#define CNF		100
#define LRAT		200
#define CLRAT		300

void usage(char *name) {
  printf("Usage: %s FILE1.cnf FILE2.lrat [optional: FILE3.drat]\n", name);
  exit(0);
}

//typedef long long ltype;
typedef int ltype;
typedef int mtype;

ltype added_clauses = 0;
ltype deleted_clauses = 0;
ltype live_clauses = 0;
ltype max_live_clauses = 0;

ltype *mask, *intro, now, lastIndex;

int *clsList, clsAlloc, clsLast;
int *table, tableSize, tableAlloc, maskAlloc;
int *litList, litCount, litAlloc;

int  getType   (int* list) { return list[1]; }
int  getIndex  (int* list) { return list[0]; }
int  getLength (int* list) { int i = 2; while (list[i]) i++; return i - 2; }
int* getHints  (int* list) { return list + getLength (list) + 2; }
int  getRATs   (int* list) { int c = 0; while (*list) if ((*list++) < 0) c++; return c; }

int convertLit (int lit)   { return (abs(lit) * 2) + (lit < 0); }

void printClause (int* clause) {
  while (*clause) printf ("%i ", *clause++); printf ("0\n"); }

int checkRedundancy (int pivot, int start, int *hints, ltype thisMask) {
  int res = abs(*hints++);
  assert (start <= res);

  if (res != 0) {
    while (start < res) {
      if (clsList[start++] != DELETED) {
        int *clause = table + clsList[start-1];
        while (*clause) {
          int clit = convertLit (*clause++);
          if (clit == (pivot^1)) return FAILED; } } }
    if (clsList[res] == DELETED) { printf ("c ERROR: using DELETED clause\n"); exit (2); };
    int flag = 0, *clause = table + clsList[res];
    while (*clause) {
      int clit = convertLit (*clause++);
      if (clit == (pivot^1)) { flag = 1; continue; }
      if (mask[clit  ] >= thisMask) continue;       // lit is falsified
      if (mask[clit^1] >= thisMask) return SUCCESS; // blocked
      mask[clit] = thisMask; }
    if (flag == 0) return FAILED; }

  while (*hints > 0) {
    if (clsList[*hints] == DELETED) { printf ("c ERROR: using DELETED clause\n"); exit (2); };
    int unit = 0, *clause = table + clsList[*(hints++)];
    while (*clause) {
      int clit = convertLit (*(clause++));
      if (mask[clit] >= thisMask) continue; // lit is falsified
      if (unit != 0) return FAILED;
      unit = clit; }
    if (unit == 0) return SUCCESS;
    if (mask[unit^1] == thisMask) printf ("c WARNING hint already satisfied in lemma with index %lli\n", (long long) lastIndex);
    mask[unit^1] = thisMask; }

  if (res == 0) return SUCCESS;
  return FAILED; }

int checkClause (int* list, int size, int* hints) {
  now++;
  int i, j, pivot = convertLit (list[0]);
  int RATs = getRATs (hints + 1);
  for (i = 0; i < size; i++) {
    int clit = convertLit (list[i]);
    if (clit >= maskAlloc) { // in case we encountered a new literal
      int old = maskAlloc;  // need to set intro?
      maskAlloc = (clit * 3) >> 1;
      mask  = (ltype *) realloc (mask,  sizeof (ltype) * maskAlloc);
      intro = (ltype *) realloc (intro, sizeof (ltype) * maskAlloc);
      if (!mask || !intro) { printf ("c Memory allocation failure\n"); exit (1); }
      for (j = old; j < maskAlloc; j++) mask[j] = intro[j] = 0; }
    mask [clit] = now + RATs; } // mark all literals in lemma with mask

  int res = checkRedundancy (pivot, 0, hints, now + RATs);
  if (res  == FAILED) return FAILED;
  if (RATs == 0)      return SUCCESS;

  int *first = hints; first++; while (*first > 0) first++;
  int start = intro[pivot ^ 1];
  while (start < -first[0]) {  // check whether no clause before -first[0] has -pivot.
    if (clsList[start] != DELETED) {
      int *clause = table + clsList[start];
      while (*clause) {
        int clit = convertLit (*clause++);
        if (clit == (pivot^1)) return FAILED; } }
    start++; }
  intro[pivot ^ 1] = -first[0];

  if (start == 0) return SUCCESS;
  while (1) {
    hints++; now++; while (*hints > 0) hints++;
    if (*hints == 0) break;
    if (-hints[0] < start) printf ("c %i %i\n", -hints[0], start);
    assert (-hints[0] >= start);
    if (checkRedundancy (pivot, start, hints, now) == FAILED) return FAILED;
    start = abs(*hints) + 1; }

  while (start <= clsLast) {
    if (clsList[start++] != DELETED) {
      int *clause = table + clsList[start-1];
      while (*clause) {
        int clit = convertLit (*clause++);
        if (clit == (pivot^1)) return FAILED; } } }

  return SUCCESS; }

void addClause (int index, int* literals, int size, FILE* drat) {
  if (index >= clsAlloc) {
    int i = clsAlloc;
    clsAlloc = (index * 3) >> 1;
    clsList = (int*) realloc (clsList, sizeof(int) * clsAlloc);
    while (i < clsAlloc) clsList[i++] = DELETED; }

  if (tableSize + size >= tableAlloc) {
//    printf ("c increasing tableSize %i\n", tableAlloc);
    tableAlloc = (tableAlloc * 3) >> 1;
    table = (int*) realloc (table, sizeof (int) * tableAlloc); }

  clsList[index] = tableSize;
  for (int i = 0; i < size; i++) {
    int clit = convertLit (literals[i]);
    if (intro[clit] == 0) intro[clit] = index;
    if (drat != NULL) fprintf (drat, "%i ", literals[i]);
    table[tableSize++] = literals[i]; }
  if (drat != NULL) fprintf (drat, "0\n");
  table[tableSize++] = 0;
  clsLast = index;
  added_clauses++;
  live_clauses++;
  if (live_clauses > max_live_clauses)
      max_live_clauses = live_clauses;
}

void deleteClauses (int* list, FILE* drat) {
  while (*list) {
    int index = *list++;
    if (clsList[index] == DELETED) {
      printf ("c WARNING: clause %i is already deleted\n", index); }
    else {
      if (drat) {
        int* clause = table + clsList[index];
        fprintf (drat, "d ");
        while (*clause) fprintf (drat, "%i ", *clause++);
        fprintf (drat, "0\n"); }
      clsList[index] = DELETED;
      deleted_clauses++;
      live_clauses--; }
  }
}

void compress (int index) {
   int* newTable = table;
   int j = 0;
   for (int i = 0; i < clsAlloc; i++) {
     if (clsList[i] == DELETED) continue;
     int* clause = table + clsList[i];
     clsList[i] = j;
     while (*clause != 0) { newTable[j++] = *clause++; }
     newTable[j++] = 0;
   }
   printf ("c compress at index %i: tableSize reduced from %i to %i\n", index, tableSize, j);
   tableSize = j;
}

static void addLit (int lit) {
  if (litCount >= litAlloc) {
    litAlloc = (litAlloc * 3) >> 1;
    litList = (int*) realloc (litList, sizeof (int) * litAlloc); }
  litList[litCount++] = lit; }

int parseLine (FILE* file, int mode, int line) {
  int lit, tmp;
  litCount = 0;
  char c = 0;
  while (1) {
    tmp = fscanf (file, " c%c", &c);
    if (tmp == EOF) return 0;
    if (tmp ==   0) break;
    while (1) {
      if (tmp == EOF ) return 0;
      if (c   == '\n') break;
      tmp = fscanf (file, "%c", &c); } }

  if (mode == CNF) {
    line++;
    while (1) {
      tmp = fscanf (file, " %i ", &lit);
      if (tmp == 0 || tmp == EOF) return 0;
      int clit = convertLit (lit);
      if (intro[clit] == 0) {
//        printf ("c setting intro[%i] = %i\n", lit, line - 1);
        intro[clit] = line - 1; }
      addLit (lit);
      if (lit == 0) return litCount; } }

  if (mode == LRAT) {
    int index;
    int zeros = 2;
    tmp = fscanf (file, " %i ", &index);
    if (tmp == 0 || tmp == EOF) return 0;
    addLit (index);

    tmp = fscanf (file, " d %i ", &lit);
    if (tmp == 1) {
      addLit ((int) 'd');
      addLit (lit);
      zeros--;
      if (lit   == 0) zeros--;
      if (zeros == 0) {
	  //	  printf("Processed line %d %c.  Length = %d\n", litList[1], litList[2], litCount);
	  return litCount; }
    }
    else { addLit((int) 'a'); }

    while (1) {
      tmp = fscanf (file, " %i ", &lit);
      if (tmp == 0 || tmp == EOF) return 0;
      addLit(lit);
      if (lit   == 0) zeros--;
      if (zeros == 0) {
	  //	  printf("Processed line %d %c.  Length = %d\n", litList[0], litList[1], litCount);
	  return litCount; }
    }
  }
  return 0; }

int main (int argc, char** argv) {
  if (argc < 3)
     usage(argv[0]);
  struct timeval start_time, finish_time;
  int return_code = 0;
  gettimeofday(&start_time, NULL);
  now = 0, clsLast = 0;

  int i, nVar = 0, nCls = 0;
  char ignore[1024];
  FILE* cnf   = fopen (argv[1], "r");
  if (!cnf) {
      printf("Couldn't open file '%s'\n", argv[1]);
      exit(1); }

  for (;;) {
    fscanf (cnf, " p cnf %i %i ", &nVar, &nCls);
    if (nVar > 0) break;
    fgets (ignore, sizeof (ignore), cnf);
    int j; for (j = 0; j < 1024; j++) { if (ignore[j] == '\n') break; }
    if (j == 1024) {
      printf ("c ERROR: comment longer than 1024 characters: %s\n", ignore); exit (0); } }

  clsAlloc = nCls * 2;
  clsList  = (int*) malloc (sizeof(int) * clsAlloc);
  for (i = 0; i < clsAlloc; i++) clsList[i] = DELETED;

  tableSize  = 0;
  tableAlloc = nCls * 2;
  table = (int *) malloc (sizeof(int) * tableAlloc);

  litAlloc = nVar * 10;
  litList = (int*) malloc (sizeof (int) * litAlloc);

  maskAlloc = 20 * nVar;
  mask  = (ltype*) malloc (sizeof(ltype) * maskAlloc);
  intro = (ltype*) malloc (sizeof(ltype) * maskAlloc);
  for (i = 0; i < maskAlloc; i++) mask[i] = intro[i] = 0;

  int index = 1;
  while (1) {
    int size = parseLine (cnf, CNF, index);
    if (size == 0) break;
    addClause (index++, litList, size, NULL); }
  fclose (cnf);

  printf ("c parsed a formula with %i variables and %i clauses\n", nVar, nCls);

  FILE* proof = fopen (argv[2], "r");
  if (!proof) {
    printf("c Couldn't open file '%s'\n", argv[2]);
    exit(1); }

  FILE* drat = NULL;
  if (argc > 3) {
    drat = fopen (argv[3], "w");
    if (!drat) {
      printf("c Couldn't open file '%s'\n", argv[3]);
      exit(1); } }

  int mode = LRAT;
  int line = 0;
  ltype del = 0;
  while (1) {
    if (live_clauses < 5 * (deleted_clauses - del)) {
      compress (line);
      del = deleted_clauses; }

    int size = parseLine (proof, LRAT, 0);
    if (size == 0) break;

    if (getType (litList) == (int) 'd') {
      deleteClauses (litList + 2, drat); }
    else if (getType (litList) == (int) 'a') {
      line  = getIndex  (litList);
      lastIndex = line;
      int  length = getLength (litList);
      int* hints  = getHints  (litList);

      if (checkClause (litList + 2, length, hints) == SUCCESS) {
        addClause (line, litList + 2, length, drat); }
      else {
        printf("c failed to check clause: "); printClause (litList + 2);
        printf("c NOT VERIFIED\n");
        return_code = 1;
      }
      if (length == 0)
        printf ("c VERIFIED\n");
    }
    else {
      printf ("c failed type\n");
      return_code = 1;
    }
  }

//  printf ("c allocated %i %i %i\n", clsAlloc, tableAlloc, litAlloc);

  gettimeofday(&finish_time, NULL);
  double secs = (finish_time.tv_sec + 1e-6 * finish_time.tv_usec) -
      (start_time.tv_sec + 1e-6 * start_time.tv_usec);
  printf("c Added clauses = %lld.  Deleted clauses = %lld.  Max live clauses = %lld\n",
	 (long long) added_clauses, (long long) deleted_clauses, (long long) max_live_clauses);
  printf("c verification time = %.2f secs\n", secs);
  return return_code;
}
