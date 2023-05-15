#include "uuf-30-1.cnf.h"
#include "uuf-30-1.drat.h"

#define NULL 0
#define stdin NULL
#define RAND_MAX 0xffff


void assert(int c)
{
    if (!c) exit(1);
}

struct FILE_s {
  unsigned char* data;
  unsigned pos;
  unsigned len;
};

typedef struct FILE_s FILE;

#define EOF (-1)

int strcmp(const char* a, const char* b)
{
    while (*a == *b) {
        if (*a == 0) return 0;
        a++;
        b++;
    }
    return *a - *b;
}

FILE* fopen_arr(unsigned char data[], unsigned len)
{
  struct FILE_s* f = malloc(sizeof(struct FILE_s));
  f->data = data;
  f->pos = 0;
  f->len = len;
  return f;
}

FILE *fopen (const char *filename, const char* mode)
{
    if (strcmp(filename, "uuf-30-1.cnf") == 0) {
        return fopen_arr(examples_uuf_30_1_cnf, examples_uuf_30_1_cnf_len);
    } else if (strcmp(filename, "uuf-30-1.drat") == 0) {
        return fopen_arr(examples_uuf_30_1_drat, examples_uuf_30_1_drat_len);
    } else {
        return 0;
    }
}

int fclose(FILE* f)
{
    return 0;
}

int is_eof(FILE* f)
{
    return f->pos >= f->len;
}

char peek(FILE* f)
{
  if (is_eof(f)) {
    return EOF;
  } else {
    return f->data[f->pos];
  }
}

void advance(FILE* f)
{
    if (f->pos < f->len)
        f->pos++;
}

int getc_unlocked(FILE* f)
{
    char c = peek(f);
    advance(f);
    return c;
}

int isspace(int c)
{
    return c == ' ' || c == '\r' || c == '\n' || c == '\f' || c == '\t' || c == '\v';
}

void skip_whitespace(FILE* f)
{
    while (isspace(peek(f)))
        advance(f);
}

int fscanf_long(FILE* f, long* a)
{
    *a = 0;
    skip_whitespace(f);

    if (is_eof(f)) return EOF;

    int negative = 0;
    int c = peek(f);

    if (c == '-')
    {
        negative = 1;
        advance(f);
        c = peek(f);
    }

    if (c < '0' || c > '9') return 0;

    // TODO overflow
    long x = 0;
    while (c >= '0' && c <= '9' && c != EOF)
    {
        advance(f);
        x = x * 10 + (c - '0');
        c = peek(f);
    }
    if (negative) x = -x;
    *a = x;
    skip_whitespace(f);
    return 1;
}

int fscanf_int(FILE* f, int* a)
{
    long b;
    int ret = fscanf_long(f, &b);
    // TODO overflow
    *a = b;
    return ret;
}


int fscanf_4(FILE* f, const char* format, int* a)
{
    assert(strcmp(format, " %i ") == 0);
    return fscanf_int(f, a);
}

int fscanf_1(FILE* f, const char* format, int* a, long* b)
{
    assert(strcmp(format, "p cnf %i %li \n") == 0);
    if (peek(f) == 'p') {
        advance(f);
        skip_whitespace(f);
        if (peek(f) == 'c') {
            advance(f);
            if (peek(f) == 'n') {
                advance(f);
                if (peek(f) == 'f') {
                    advance(f);

                    skip_whitespace(f);
                    int s1 = fscanf_int(f, a);
                    if (s1 == EOF || s1 == 0) return s1;
                    int s2 = fscanf_long(f, b);
                    if (s2 == EOF) return s2;
                    return s1 + s2;
                }
            }
        }
    }
    return 0;
}

int fscanf_2(FILE* f, const char* format)
{
    assert(strcmp(format, "%*s\n") == 0);
    while (1)
    {
        int c = getc_unlocked(f);
        if (c == '\n') return 0;
        if (c == EOF) return EOF;
    }
}

int fscanf_3(FILE* f, const char* format, int* a)
{
    assert(strcmp(format, " d  %i ") == 0);
    skip_whitespace(f);
    if (peek(f) == 'd') {
        advance(f);

        skip_whitespace(f);
        return fscanf_int(f, a);
    }
    return 0;
}

extern int long write(int fd, const void *buf, unsigned long count);

// TODO do not ignore the actual values
#define printf(fmt, ...) write(0, fmt, sizeof(fmt) + 1)
