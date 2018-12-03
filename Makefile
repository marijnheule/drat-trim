drat-trim: main.c drat-trim.o
	gcc main.c -O2 -o drat-trim drat-trim.o

drat-trim.o: drat-trim.c drat-trim.h
	gcc drat-trim.c -O2 -c -o drat-trim.o

drat-trim.a: drat-trim.o
	ar rc $@ $^

clean:
	rm -f drat-trim drat-trim.o drat-trim.a
