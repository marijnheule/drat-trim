#include "uuf-30-1.cnf.h"
#include "uuf-30-1.drat.h"

#define NULL 0
#define stdin NULL
#define RAND_MAX 0xffff

typedef unsigned size_t;
typedef int ssize_t;

int main (int argc, char** argv);

void __runtime_start() {
    char* argv[] = {"", "uuf-30-1.cnf", "uuf-30-1.drat"};
    main(3, argv);
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
    return c;
}

int long write(int fd, const void *buf, unsigned long count) {
    for (unsigned long i = 0; i < count; i++) {
        char c = ((const char*)buf)[i];
        putchar(c);
    }
    return count;
}

#endif

// TODO do not ignore the actual values
#define printf(fmt, ...) write(0, fmt, sizeof(fmt) + 1)

// TODO check the file
#define fprintf(file, fmt, ...) write(0, fmt, sizeof(fmt) + 1)

int fputc(int c, FILE* stream) {
    // TODO implement?
    return 0;
}

char *fgets(char *s, int size, FILE *stream) {
    // TODO implement
    return 0;
}

int rand() {
    // TODO
    return 3;
}

int abs(int x) {
    return x >= 0 ? x : -x;
}

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


/* Byte-wise swap two items of size SIZE. */
#define SWAP(a, b, size)						      \
  do									      \
    {									      \
      size_t __size = (size);						      \
      char *__a = (a), *__b = (b);					      \
      do								      \
	{								      \
	  char __tmp = *__a;						      \
	  *__a++ = *__b;						      \
	  *__b++ = __tmp;						      \
	} while (--__size > 0);						      \
    } while (0)
/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4
/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct
  {
    char *lo;
    char *hi;
  } stack_node;
/* The next 4 #defines implement a very fast in-line stack abstraction. */
/* The stack needs log (total_elements) entries (we could even subtract
   log(MAX_THRESH)).  Since total_elements has type size_t, we get as
   upper bound for log (total_elements):
   bits per byte (CHAR_BIT) * sizeof(size_t).  */
#define STACK_SIZE	(8 * sizeof (size_t))
#define PUSH(low, high)	((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define	POP(low, high)	((void) (--top, (low = top->lo), (high = top->hi)))
#define	STACK_NOT_EMPTY	(stack < top)
/* Order size using quicksort.  This implementation incorporates
   four optimizations discussed in Sedgewick:
   1. Non-recursive, using an explicit stack of pointer that store the
      next array partition to sort.  To save time, this maximum amount
      of space required to store an array of SIZE_MAX is allocated on the
      stack.  Assuming a 32-bit (64 bit) integer for size_t, this needs
      only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
      Pretty cheap, actually.
   2. Chose the pivot element using a median-of-three decision tree.
      This reduces the probability of selecting a bad pivot value and
      eliminates certain extraneous comparisons.
   3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
      insertion sort to order the MAX_THRESH items within each partition.
      This is a big win, since insertion sort is faster for small, mostly
      sorted array segments.
   4. The larger of the two sub-partitions is always pushed onto the
      stack first, with the algorithm then concentrating on the
      smaller partition.  This *guarantees* no more than log (total_elems)
      stack size is needed (actually O(1) in this case)!  */
void
qsort (void *const pbase, size_t total_elems, size_t size,
	    int (*cmp)(const void *, const void *))
{
  char *base_ptr = (char *) pbase;
  const size_t max_thresh = MAX_THRESH * size;
  if (total_elems == 0)
    /* Avoid lossage with unsigned arithmetic below.  */
    return;
  if (total_elems > MAX_THRESH)
    {
      char *lo = base_ptr;
      char *hi = &lo[size * (total_elems - 1)];
      stack_node stack[STACK_SIZE];
      stack_node *top = stack;
      PUSH (NULL, NULL);
      while (STACK_NOT_EMPTY)
        {
          char *left_ptr;
          char *right_ptr;
	  /* Select median value from among LO, MID, and HI. Rearrange
	     LO and HI so the three values are sorted. This lowers the
	     probability of picking a pathological pivot value and
	     skips a comparison for both the LEFT_PTR and RIGHT_PTR in
	     the while loops. */
	  char *mid = lo + size * ((hi - lo) / size >> 1);
	  if ((*cmp) ((void *) mid, (void *) lo) < 0)
	    SWAP (mid, lo, size);
	  if ((*cmp) ((void *) hi, (void *) mid) < 0)
	    SWAP (mid, hi, size);
	  else
	    goto jump_over;
	  if ((*cmp) ((void *) mid, (void *) lo) < 0)
	    SWAP (mid, lo, size);
	jump_over:;
	  left_ptr  = lo + size;
	  right_ptr = hi - size;
	  /* Here's the famous ``collapse the walls'' section of quicksort.
	     Gotta like those tight inner loops!  They are the main reason
	     that this algorithm runs much faster than others. */
	  do
	    {
	      while ((*cmp) ((void *) left_ptr, (void *) mid) < 0)
		left_ptr += size;
	      while ((*cmp) ((void *) mid, (void *) right_ptr) < 0)
		right_ptr -= size;
	      if (left_ptr < right_ptr)
		{
		  SWAP (left_ptr, right_ptr, size);
		  if (mid == left_ptr)
		    mid = right_ptr;
		  else if (mid == right_ptr)
		    mid = left_ptr;
		  left_ptr += size;
		  right_ptr -= size;
		}
	      else if (left_ptr == right_ptr)
		{
		  left_ptr += size;
		  right_ptr -= size;
		  break;
		}
	    }
	  while (left_ptr <= right_ptr);
          /* Set up pointers for next iteration.  First determine whether
             left and right partitions are below the threshold size.  If so,
             ignore one or both.  Otherwise, push the larger partition's
             bounds on the stack and continue sorting the smaller one. */
          if ((size_t) (right_ptr - lo) <= max_thresh)
            {
              if ((size_t) (hi - left_ptr) <= max_thresh)
		/* Ignore both small partitions. */
                POP (lo, hi);
              else
		/* Ignore small left partition. */
                lo = left_ptr;
            }
          else if ((size_t) (hi - left_ptr) <= max_thresh)
	    /* Ignore small right partition. */
            hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr))
            {
	      /* Push larger left partition indices. */
              PUSH (lo, right_ptr);
              lo = left_ptr;
            }
          else
            {
	      /* Push larger right partition indices. */
              PUSH (left_ptr, hi);
              hi = right_ptr;
            }
        }
    }
  /* Once the BASE_PTR array is partially sorted by quicksort the rest
     is completely sorted using insertion sort, since this is efficient
     for partitions below MAX_THRESH size. BASE_PTR points to the beginning
     of the array to sort, and END_PTR points at the very last element in
     the array (*not* one beyond it!). */
#define min(x, y) ((x) < (y) ? (x) : (y))
  {
    char *const end_ptr = &base_ptr[size * (total_elems - 1)];
    char *tmp_ptr = base_ptr;
    char *thresh = min(end_ptr, base_ptr + max_thresh);
    char *run_ptr;
    /* Find smallest element in first threshold and place it at the
       array's beginning.  This is the smallest array element,
       and the operation speeds up insertion sort's inner loop. */
    for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
      if ((*cmp) ((void *) run_ptr, (void *) tmp_ptr) < 0)
        tmp_ptr = run_ptr;
    if (tmp_ptr != base_ptr)
      SWAP (tmp_ptr, base_ptr, size);
    /* Insertion sort, running from left-hand-side up to right-hand-side.  */
    run_ptr = base_ptr + size;
    while ((run_ptr += size) <= end_ptr)
      {
	tmp_ptr = run_ptr - size;
	while ((*cmp) ((void *) run_ptr, (void *) tmp_ptr) < 0)
	  tmp_ptr -= size;
	tmp_ptr += size;
        if (tmp_ptr != run_ptr)
          {
            char *trav;
	    trav = run_ptr + size;
	    while (--trav >= run_ptr)
              {
                char c = *trav;
                char *hi, *lo;
                for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                  *hi = *lo;
                *hi = c;
              }
          }
      }
  }
}
