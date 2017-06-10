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

#define ADD		1
#define DEL		0
#define SUCCESS		1
#define FAILED		0
#define CNF		100
#define LRAT		200
#define CLRAT		300

long long *mask, now;

int *clsList, clsAlloc;
int *table, tableSize, tableAlloc, maskSize, maskAlloc;

int  getType   (int* list) { return list[0]; }
int  getIndex  (int* list) { return list[1]; }
int  getLength (int* list) { int i; for (i = 2; list[i] != 0; i++); return i - 2; }
int* getHints  (int* list) { return list + getLength (list) + 1; }
int  getRATs   (int* list) { int c = 0; while (*list) if ((*list++) < 0) c++; return c; }

int convertLit (int lit)   { return (abs(lit) * 2) + (lit < 0); }

int checkRedundancy (int *hints, int thisMask) {
  int res = abs(*hints++);

  if (res > 0) {
    int unit = 0, *clause = table + clsList[res];
    while (*clause) {
      clit = convertLit (*clause++);
      if (mask[clit] >= thisMask) continue; // lit is falsified
      if (unit != 0) return FAILED;
      unit = clit; }
    if (unit == 0) return SUCCESS;
    mask[clit ^ 1] = thisMask; }

  while (*hints > 0) {
    int unit = 0, *clause = table + clsList[*hint++];
    while (*clause) {
      clit = convertLit (*clause++);
      if (mask[clit] >= now) continue; // lit is falsified
      if (unit != 0) return FAILED;
      unit = clit; }
    if (unit == 0) return SUCCESS;
    mask[clit ^ 1] = thisMask; } }

  if (res == 0) return SUCCESS;
  return FAILED; }

int checkClause (int* list, int size, int* hints) {
  now++;
  int i, pivot = convertLit (list[0]);
  int RATs = getRATs (hints);
  for (i = 0; i < size; i++) {
    int clit = convertLit (list[i]);
    if (clit > maskSize) {
      maskAlloc = (clit * 3) >> 1;
      mask = (long long *) realloc (mask, sizeof(long long) * maskAlloc); }
    mask [clit] = now + RATs; }

  int res = checkRedundancy (hints, now + RATs);
  if (res  == FAILED) return FAILED;
  if (RATs == 0)      return SUCCESS;

  hints++;
  while (*hints) {
    while (*hint > 0) hints++;
    now++;
    if (checkRedundancy (hints, now) == FAILED) return FAILED; }

  return SUCCESS; }

void addClause (int index, int* literals, int size) {
  if (index >= clsAlloc) {
    clsAlloc = clsAlloc (index * 3) >> 1;
    clsList = (int*) realloc (clsList, sizeof(int) * clsAlloc); }

  if (tableSize + size >= tableAlloc) {
    tableAlloc = (tableAlloc * 3) >> 1;
    table = (int*) realloc (table, sizeof (int) * tableAlloc); }

  clsList[index] = tableSize;
  int i; for (i = 0; i < size; i++) table[tableSize++] = literals[i]; }

void deleteClauses (int* list) {
  while (*list) {
    int index = *list++;
    if (talbe[index] == 0) {
      printf ("c WARNING: clause %i is already deleted\n", index); }
    table[index] = 0; }

void parseLine (FILE* file, int *list, int mode) {

  if (mode == CNF) {

  }
  else {


  }
}

int main (int argc, char** argv) {
  now = 1;
  FILE *cnf, *proof;

  cnf   = fopen (argv[1], "r");

  int nVar, nCls;
  fscanf (cnf, " p cnf %i %i ", &nVar, &nCls);

  clsAlloc = nCls * 2;
  clsList  = (int*) malloc (sizeof(int) * clsAlloc);

  maxVar = nVar;
  maskAlloc = maxVar * 4;
  mask = (int *) malloc (sizeof(int) * maskAlloc);

  tableSize  = 0;
  tableAlloc = nCls * 2;
  table = (int *) malloc (sizeof(int) * tableAlloc);

  proof = fopen (argv[2], "r");
  int mode = CLRAT;
  while (1) {
    int flag = parseLine (proof, list, mode);
    if (flag == FAILED) break;

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
        printClause (list + 2, length);
        printf("c NOT VERIFIED\n");
        exit (0); }

      if (lenght == 0) {
        printf ("c VERIFIED\n");
        exit (1); }
    }
    else {
      printf ("c failed type\n");
      exit (0);
    }
  }
}
