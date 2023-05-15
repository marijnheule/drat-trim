#include "uuf-30-1.cnf.h"
#include "uuf-30-1.drat.h"

#define NULL 0
#define EOF -1
#define stdin 1
#define RAND_MAX 0xffff

struct FILE {
  unsigned char* data;
  unsigned pos;
  unsigned len;
};

struct FILE* fopen_arr(unsigned char data[], unsigned len) {
  struct FILE* f = malloc(sizeof(struct FILE));
  f->data = data;
  f->pos = 0;
  f->len = len;
  return f;
}

int getc_unlocked(struct FILE* f) {
  if (f->pos >= f->len) {
    return EOF;
  } else {
    int c = f->data[f->pos++];
    return c;
  }
}

