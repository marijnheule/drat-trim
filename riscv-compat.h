#include "uuf-30-1.cnf.h"
#include "uuf-30-1.drat.h"

#define NULL 0
#define EOF -1
#define stdin 1
#define RAND_MAX 0xffff

struct FILE_s {
  unsigned char* data;
  unsigned pos;
  unsigned len;
};

typedef struct FILE_s FILE;

int strcmp(const char* a, const char* b) {
    while (*a == *b) {
        if (*a == 0) return 0;
        a++;
        b++;
    }
    return *a - *b;
}

FILE* fopen_arr(unsigned char data[], unsigned len) {
  struct FILE_s* f = malloc(sizeof(struct FILE_s));
  f->data = data;
  f->pos = 0;
  f->len = len;
  return f;
}

FILE *fopen (const char *filename, const char* mode) {
    if (strcmp(filename, "uuf-30-1.cnf") == 0) {
        return fopen_arr(examples_uuf_30_1_cnf, examples_uuf_30_1_cnf_len);
    } else if (strcmp(filename, "uuf-30-1.drat") == 0) {
        return fopen_arr(examples_uuf_30_1_drat, examples_uuf_30_1_drat_len);
    } else {
        return 0;
    }
}

int getc_unlocked(FILE* f) {
  if (f->pos >= f->len) {
    return EOF;
  } else {
    int c = f->data[f->pos++];
    return c;
  }
}

void assert(int c) {
    if (!c) exit(1);
}