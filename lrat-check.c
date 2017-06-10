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

int now;
int nVar, nCls, *cls;
int *table, tableSize, tableAlloc;
int *mask, maskSize, maskAlloc;

int convertLit (lit) {

}

int checkClause (int* list, int size, int* hints) {
  int i;
  int pivot = convertLit (list[0]);
  for (i = 0; i < size; i++) {
    int clit = convertLit (list[i]);
    if (clit > maskSize) {
      maskAlloc = (clit * 3) >> 1;
      mask = (int *) realloc (mask, sizeof(int) * maskAlloc); }
    mask [clit] = now; }

  return SUCCESS;
}

void addClause (int index, int* literals, int size) {
  if (tableSize + size >= tableAlloc) {
    tableAlloc = (tableAlloc * 3) >> 1;
    table = (int*) realloc (table, sizeof (int) * tableAlloc); }

  int i;
  for (i = 0; i < size; i++) table[cls[index] + i] = literals[i]; }

void deleteClauses (int *list) {
  while (*list) {
    int index = *list++;
    if (talbe[index] == 0) {
      printf ("c WARNING: clause %i is already deleted\n", index); }
    table[index] = 0; }

void parseLine (FILE* file, int *literalsplushints, int mode) {

  if (mode == CNF) {

  }
  else {


  }
}

int getType (int* list) {
  return list[0]; }

int getIndex (int* list) {
  return list[1]; }

int getLength (int* list) {
  int i; for (i = 2; list[i] != 0; i++);
  return i - 2; }

int* getHints (int* list) {
  return list + getLength (list) + 1; }

int main (int argc, char** argv) {
  FILE *cnf, *proof;

  cnf   = fopen (argv[1], "r");

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
      deleteClauses (list + 2);
    }
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
