all: drat-trim lrat-check

drat-trim: drat-trim.c
	gcc drat-trim.c -O2 -o drat-trim

lrat-check: lrat-check.c
	gcc lrat-check.c -O2 -o lrat-check

clean:
	rm drat-trim lrat-check
