#include "uuf-30-1.cnf.h"
#include "uuf-30-1.drat.h"

#define NULL 0
#define stdin NULL
#define RAND_MAX 0xffff

typedef unsigned size_t;
typedef int ssize_t;

int main (int argc, char** argv);

void __runtime_start() {
    main(0, 0);
}

void assert(int c)
{
    if (!c) exit(1);
}

#define MEMORY_SIZE 0x40000000
static unsigned char memory[MEMORY_SIZE];
static unsigned char* bump_ptr = memory;

void* malloc(unsigned size) {
    unsigned aligned_size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1) + sizeof(unsigned);

    if ((bump_ptr + aligned_size) > (memory + MEMORY_SIZE)) {
        // TODO better to panic right away.
        // Out of memory
        return NULL;
    }

    // Allocate memory from the bump pointer
    void* allocated_mem = bump_ptr;
    bump_ptr += aligned_size;

    *((unsigned*) allocated_mem) = size;
    return (void*)((char*)allocated_mem + sizeof(unsigned));
}

void free(void* ptr) {}

void* memcpy(void* destination, const void* source, unsigned num) {
    unsigned char* dest = (unsigned char*)destination;
    const unsigned char* src = (const unsigned char*)source;

    if ((((unsigned)dest & 3) == 0) && (((unsigned)src & 3) == 0) && (num >= 4)) {
        unsigned* dest_word = (unsigned*)dest;
        const unsigned* src_word = (const unsigned*)src;
        unsigned num_words = num / sizeof(unsigned);

        // Copy whole words
        for (unsigned i = 0; i < num_words; i++) {
            dest_word[i] = src_word[i];
        }

        // Calculate the remaining bytes to copy
        unsigned remaining_bytes = num - (num_words * 4);
        dest += num_words * 4;
        src += num_words * 4;

        // Copy the remaining bytes
        for (unsigned i = 0; i < remaining_bytes; i++) {
            dest[i] = src[i];
        }
    } else {
        // Copy byte by byte
        for (unsigned i = 0; i < num; i++) {
            dest[i] = src[i];
        }
    }

    return destination;
}

void* realloc(void* ptr, unsigned size) {
    if (ptr == NULL) {
        return malloc(size);
    }

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    void* new_ptr = malloc(size);
    if (new_ptr != NULL) {
        // Copy the data from the old memory block to the new memory block
        unsigned old_size = *(unsigned*)((char*) ptr - sizeof(unsigned));
        unsigned copy_size = (old_size < size) ? old_size : size;
        memcpy(new_ptr, ptr, copy_size);

        // Free the old memory block
        free(ptr);
    }

    return new_ptr;
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

#ifdef RISCV_SIM
extern int long write(int fd, const void *buf, unsigned long count);
#else
int putchar(int c) {
    register unsigned value __asm__("a0") = (unsigned)c;
    __asm__ volatile("ebreak" : : "r"(value));
}

int long write(int fd, const void *buf, unsigned long count) {
    for (unsigned long i = 0; i < count; i++) {
        char c = ((const char*)buf)[i];
        putchar(c);
    }
    while(1){}
}

#endif

// TODO do not ignore the actual values
#define printf(fmt, ...) write(0, fmt, sizeof(fmt) + 1)

// TODO check the file
#define fprintf(file, fmt, ...) write(0, fmt, sizeof(fmt) + 1)

int atoi(const char* s) {
    while (isspace(*s))
        s++;

    if (*s == 0) return 0;

    int negative = 0;
    if (*s == '-')
    {
        negative = 1;
        s++;
    }

    if (*s < '0' || *s > '9') return 0;

    // TODO overflow
    int x = 0;
    while (*s >= '0' && *s <= '9')
    {
        x = x * 10 + (*s - '0');
        s++;
    }
    if (negative) x = -x;
    return x;
}

void exit(int status) {
    // TODO take status into account
    // but drat-trim does actually not always properly report the status.
    while (1) {}
}

// These are not really needed
int remove(const char* path) {
    exit(1);
}