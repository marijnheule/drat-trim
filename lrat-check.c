/************************************************************************************[lrat-check.c]
Copyright (c) 2017-2023 Marijn Heule, Carnegie Mellon University
Last edit: July 2023

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
#include <limits.h>
#include <sys/time.h>

#define PRINT		0
#define DELETED		-1
#define CONFLICT	2
#define SUCCESS		1
#define FAILED		0
#define CNF		100
#define LRAT		200
#define CLRAT		300
#define BUCKET		8192
#define INIT		1024

void usage(char *name) {
  printf("Usage: %s FILE1.cnf FILE2.lrat [optional: FILE3.drat]\n", name);
  exit(0);
}

#ifdef LONGTYPE
  typedef long long ltype;
#else
  typedef int ltype;
#endif
typedef int mtype;

ltype added_clauses = 0;
ltype deleted_clauses = 0;
ltype live_clauses = 0;
ltype max_live_clauses = 0;

ltype *mask, *intro, now, lastIndex;

int maxBucket;

int *clsList, clsAlloc, clsLast;
int *table, tableSize;
long long int tableAlloc, maskAlloc;
int *litList, litCount, litAlloc;
int *inBucket;
int *topTable; long long int topAlloc;

int  getType   (int* list) { return list[1]; }
int  getIndex  (int* list) { return list[0]; }
int  getLength (int* list) { int i = 2; while (list[i]) i++; return i - 2; }
int* getHints  (int* list) { return list + getLength (list) + 2; }
int  getRATs   (int* list) { int c = 0; while (*list) if ((*list++) < 0) c++; return c; }

int convertLit (int lit)   { return (abs(lit) * 2) + (lit < 0); }
int printLit   (int lit)   { return (lit >> 1) * (-2 * (lit&1) + 1); }

inline int getClause (int index) {
  int bucket = index / BUCKET;
  int offset = index % BUCKET;
  return clsList[topTable[bucket]*BUCKET + offset]; }

inline void setClause (int index, int value) {
  int bucket = index / BUCKET;
  int offset = index % BUCKET;
  clsList[topTable[bucket]*BUCKET + offset] = value; }

void printClause (int* clause) {
  while (*clause) printf ("%i ", *clause++); printf ("0\n"); }

int checkRedundancy (int pivot, int start, int *hints, ltype thisMask, int print) {
  int res = abs(*hints++);
  assert (start <= res);

  if (print) printf ("c check redundancy res: %i pivot: %i start: %i\n", res, printLit(pivot), start);
  if (res != 0) { // if resolvent is non-empty
    while (start < res) { // check whether all clasues before the hint are tautologies
      if (getClause(start++) != DELETED) {
        int *clause = table + getClause(start-1);
        while (*clause) {
          int clit = convertLit (*clause++);
          if (clit == (pivot^1)) { printf ("c FAILED tautology %i\n", pivot); return FAILED; } } } }
    if (getClause(res) == DELETED) { printf ("c ERROR: using DELETED clause %i\n", res); return FAILED; };
    int flag = 0, *clause = table + getClause(res);
    while (*clause) { // find the pivot
      int clit = convertLit (*clause++);
      if (clit == (pivot^1)) { flag = 1; continue; }
      if (mask[clit  ] >= thisMask) continue;       // lit is falsified
      if (mask[clit^1] >= thisMask) return SUCCESS; // blocked
      mask[clit] = thisMask; }
    if (flag == 0) { printf ("c FAILED: pivot %i not found\n", pivot); return FAILED; } }

  while (*hints > 0) {
    if (print) {
      printf ("c hint %i\nc ", *hints);
      int* c = table + getClause(*hints); printClause (c); }
    if (getClause(*hints) == DELETED) { printf ("c ERROR: using DELETED hint clause %i\n", *hints); printf ("c %i\n", topTable[*hints/BUCKET]); return FAILED; };
    int unit = 0, *clause = table + getClause(*(hints++));
    while (*clause) {
      int clit = convertLit (*(clause++));
      if (mask[clit] >= thisMask) continue; // lit is falsified
      if (unit != 0) { printf ("c FAILED: multiple literals unassigned in hint %i: %i %i\n", hints[-1], unit%2?(-unit >> 1):(unit>>1), clit%2?(-clit >> 1):(clit>>1));
                       return FAILED; }
      unit = clit; }
    if (print) {
      if (unit != 0) printf ("c unit %i\n", printLit(unit));
      else           printf ("c detected conflict\n"); }
    if (unit == 0) return CONFLICT;  // detected conflict
    if (mask[unit^1] == thisMask) printf ("c WARNING hint already satisfied in lemma with index %lli\n", (long long) lastIndex);
    mask[unit^1] = thisMask; }

  if (res == 0) {
    if (print) printf ("c SUCCESS\n");
    return SUCCESS;
  }
  return FAILED; }

int checkClause (int* list, int size, int* hints, int print) {
  now++;
  int pivot = convertLit (list[0]);

  int RATs = getRATs (hints + 1); // the number of negated hints
  for (int i = 0; i < size; i++) { // assign all literals in the clause to false
    int clit = convertLit (list[i]);
    if (clit >= maskAlloc) { // in case we encountered a new literal
      int old = maskAlloc;  // need to set intro?
      maskAlloc = (clit * 3) >> 1;
      if (maskAlloc % 2) maskAlloc++;
      mask  = (ltype *) realloc (mask,  sizeof (ltype) * maskAlloc);
      intro = (ltype *) realloc (intro, sizeof (ltype) * maskAlloc);
      if (!mask || !intro) { printf ("c Memory allocation failure\n"); exit (1); }
      for (int j = old; j < maskAlloc; j++) mask[j] = intro[j] = 0; }
    mask [clit] = now + RATs; } // mark all literals in lemma with mask

  int res = checkRedundancy (pivot, 0, hints, now + RATs, print);

  if (res == CONFLICT) { return SUCCESS; }
  if (res == FAILED  ) { return FAILED;  }

  int *first = hints; first++; while (*first > 0) first++;
  int start = intro[pivot ^ 1];

  if (RATs == 0)      {
//    if (res == SUCCESS) return FAILED; // No RAT and no conflict is FAILED
    if (print) printf ("c start %i first %i\n", start, -first[0]);
    if (start != 0) return FAILED;
    return SUCCESS; }

  while (start < -first[0]) {  // check whether no clause before -first[0] has -pivot.
    if (getClause(start) != DELETED) {
      int *clause = table + getClause(start);
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
    if (checkRedundancy (pivot, start, hints, now, print) == FAILED) return FAILED;
    start = abs(*hints) + 1; }

  while (start <= clsLast) {
    if (getClause(start++) != DELETED) {
      int *clause = table + getClause(start-1);
      while (*clause) {
        int clit = convertLit (*clause++);
        if (clit == (pivot^1)) { printf("c FAILED: tautology %i\n", pivot); return FAILED; } } } }

  return SUCCESS; }

void addClause (int index, int* literals, int size, FILE* drat) {

//  printf ("c index %i\n", index);
  if (index >= topAlloc * BUCKET) {
    long long int old = topAlloc;
    topAlloc = (topAlloc * 3) >> 1;
    printf ("c topTable reallocation from %lli to %lli\n", old, topAlloc);
    topTable = (int*) realloc (topTable, sizeof(int) * topAlloc);
    if (!topTable) { printf ("c Memory allocation failure of topTable\n"); exit (1); }
    for (int j = old; j < topAlloc; j++) topTable[j] = -1; }

  int count = 0, bucket = topTable[index/BUCKET];
  if (bucket >= 0) count = inBucket[bucket];
  if (bucket == -1) {
//  if (count == 0 || bucket == -1) {
//    printf ("c count %i %i\n", count, topTable[index/BUCKET]);
    for (bucket = 0; bucket < maxBucket; bucket++)
      if (inBucket[bucket] == 0) { topTable[index/BUCKET] = bucket; break; }
//    printf ("c index %i will be in bucket %i\n", index, bucket);
  }

  if (bucket == maxBucket) {
    maxBucket = (maxBucket * 3) >> 1;
    printf ("c increasing the number of buckets from %i to %i\n", bucket, maxBucket);
    inBucket = (int*) realloc (inBucket, sizeof(int) * maxBucket);
    if (!inBucket) { printf ("c Memory allocation failure of inBucket\n"); exit (1); }
    for (int j = bucket; j < maxBucket; j++) inBucket[j] = 0;
    clsList = (int*) realloc (clsList, sizeof(int) * maxBucket * BUCKET);
    if (!clsList) { printf ("c Memory allocation failure of clsList\n"); exit (1); }
    for (int i = bucket * BUCKET; i < maxBucket * BUCKET; i++) clsList[i] = DELETED; // is this required?
  }

  topTable[index/BUCKET] = bucket;

  if (tableSize + size >= tableAlloc) {
//    printf ("c table realloc %lli\n", tableAlloc);
    tableAlloc = (tableAlloc * 3) >> 1;
    table = (int*) realloc (table, sizeof (int) * tableAlloc);
    if (!table) { printf ("c Memory allocation failure of table\n"); exit (1); }
  }

  setClause (index, tableSize);
  for (int i = 0; i < size; i++) {
    int clit = convertLit (literals[i]);
    if (intro[clit] == 0) intro[clit] = index;
    if (drat != NULL) fprintf (drat, "%i ", literals[i]);
    table[tableSize++] = literals[i]; }
  if (drat != NULL) fprintf (drat, "0\n");
  table[tableSize++] = 0;
  clsLast = index;

  bucket = topTable[index/BUCKET];
  inBucket[bucket]++;

  added_clauses++;
  live_clauses++;
  if (live_clauses > max_live_clauses)
      max_live_clauses = live_clauses;
}

void deleteClauses (int* list, FILE* drat) {
  while (*list) {
    int index = *list++;
    if (getClause(index) == DELETED) {
      printf ("c WARNING: clause %i is already deleted\n", index); }
    else {
      if (drat) {
        int* clause = table + getClause(index);
        fprintf (drat, "d ");
        while (*clause) fprintf (drat, "%i ", *clause++);
        fprintf (drat, "0\n"); }
      setClause (index, DELETED);
      int bucket = topTable[index/BUCKET];
      inBucket[bucket]--;
      if (inBucket[bucket] == 0) {
        topTable[index/BUCKET] = -1;
//        printf ("c bucket %i is empty\n", bucket);
      }
      deleted_clauses++;
      live_clauses--; }
  }
}

void compress (int index, int print) {
   int* newTable = table;
   int n = 0;
   for (int i = 0; i < topAlloc; i++) {
     if (topTable[i] == -1) continue;
//     int b = topTable[i];
//     printf ("c bucket: %i size: %i\n", b, inBucket[b]);
//     assert (inBucket[b]);
     for (int j = 0; j < BUCKET; j++) {
       int c = i*BUCKET+j;
       if (getClause(c) == DELETED) continue;
       int* clause = table + getClause(c);
       setClause (c, n);
       while (*clause != 0) { newTable[n++] = *clause++; }
       newTable[n++] = 0; }
   }

/*
   for (int i = 0; i < maxBucket; i++) {
     if (inBucket[i] == 0) continue;
     for (int j = 0; j < BUCKET; j++) {
       int c = i*BUCKET+j;
       if (getClause(c) == DELETED) continue;
       int* clause = table + getClause(c);
       setClause (c, n);
       while (*clause != 0) { newTable[n++] = *clause++; }
       newTable[n++] = 0; }
   }
*/
   if (print)
     printf ("c compress at index %i: tableSize reduced from %i to %i\n", index, tableSize, n);
   tableSize = n;
}

static void addLit (int lit) {
  if (litCount >= litAlloc) {
    litAlloc = (litAlloc * 3) >> 1;
    litList = (int*) realloc (litList, sizeof (int) * litAlloc);
    if (!litList) { printf ("c Memory allocation failure of litList\n"); exit (1); }
  }
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
//    printf ("%lli ", index);
//    assert (index > 0);
    if (tmp == 0 || tmp == EOF) return 0;
    addLit ((int) index);

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
  if (argc < 2)
     usage(argv[0]);
  if (argc == 2)
    printf ("c expecting proof in stdin\n");

  struct timeval start_time, finish_time;
  int found_error = 0; // encountered an error?
  int found_empty_clause = 0; // encountered derivation of empty clause?
  gettimeofday(&start_time, NULL);
  now = 0, clsLast = 0;

  int nVar = 0, nCls = 0;
  char ignore[1024];
  FILE* cnf   = fopen (argv[1], "r");
  if (!cnf) {
      printf("Couldn't open file '%s'\n", argv[1]);
      exit(1); }

  for (;;) {
    int tmp = fscanf (cnf, " p cnf %i %i ", &nVar, &nCls);
    if (tmp == 2) break;
    fgets (ignore, sizeof (ignore), cnf);
    int j; for (j = 0; j < 1024; j++) { if (ignore[j] == '\n') break; }
    if (j == 1024) {
      printf ("c ERROR: comment longer than 1024 characters: %s\n", ignore); exit (0); } }

  if (nVar <= 0) nVar = 1;

  topAlloc = INIT;
  topTable = (int*) malloc (sizeof(int) * topAlloc);
  for (int i = 0; i < topAlloc; i++) topTable[i] = -1;

  maxBucket = topAlloc;
  inBucket = (int*) malloc (sizeof(int) * maxBucket);
  for (int i = 0; i < maxBucket; i++) inBucket[i] = 0;
  clsList  = (int*) malloc (sizeof(int) * maxBucket * BUCKET);
  for (int i = 0; i < maxBucket * BUCKET; i++) clsList[i] = DELETED;

  tableSize  = 0;
  tableAlloc = (long long int) nCls * 2;
  table = (int *) malloc (sizeof(int) * tableAlloc);

  litAlloc = nVar * 10;
  litList = (int*) malloc (sizeof (int) * litAlloc);

  maskAlloc = 20 * nVar;
  mask  = (ltype*) malloc (sizeof(ltype) * maskAlloc);
  intro = (ltype*) malloc (sizeof(ltype) * maskAlloc);
  for (int i = 0; i < maskAlloc; i++) mask[i] = intro[i] = 0;

  int index = 1;
  while (1) {
    int size = parseLine (cnf, CNF, index);
    if (size == 0) break;
    addClause (index++, litList, size, NULL); }
  fclose (cnf);

  printf ("c parsed a formula with %i variables and %i clauses\n", nVar, nCls);

  FILE* proof = stdin;
  if (argc > 2)
    proof = fopen (argv[2], "r");
  if (!proof) {
    printf("c Couldn't open file '%s'\n", argv[2]);
    exit(1); }

  FILE* drat = NULL;
  if (argc > 3) {
    drat = fopen (argv[3], "w");
    if (!drat) {
      printf("c Couldn't open file '%s'\n", argv[3]);
      exit(1); } }

  int print = PRINT;
  int mode = LRAT;
  int line = 0;
  ltype del = 0;
  while (1) {
    if (live_clauses < 5 * (deleted_clauses - del)) {
      compress (line, print);
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

      if (checkClause (litList + 2, length, hints, print) == SUCCESS) {
        addClause (line, litList + 2, length, drat); }
      else {
        printf("c failed while checking clause: "); printClause (litList + 2);
        checkClause (litList + 2, length, hints, 1);
        found_error = 1;
        break;
      }
      if (length == 0) {
        found_empty_clause = 1; }
    }
    else {
      printf ("c failed type\n");
      found_error = 1;
      break;
    }
  }

  int return_code;
  if (found_empty_clause && !found_error) {
    printf ("c VERIFIED\n");
    return_code = 0;
  } else {
//    if (!found_error) {
//    }
    printf("c WARNING: Verification incomplete. Last line checked = %d\n", line);
    printf ("c NOT VERIFIED\n");
    return_code = 1;
  }

  printf ("c allocated %i %lli %i\n", maxBucket, tableAlloc, litAlloc);

  gettimeofday(&finish_time, NULL);
  double secs = (finish_time.tv_sec + 1e-6 * finish_time.tv_usec) -
      (start_time.tv_sec + 1e-6 * start_time.tv_usec);
  printf("c Added clauses = %lld.  Deleted clauses = %lld.  Max live clauses = %lld\n",
	 (long long) added_clauses, (long long) deleted_clauses, (long long) max_live_clauses);
  printf("c verification time = %.2f secs\n", secs);
  return return_code;
}
