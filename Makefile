drat-trim: main.c drat-trim.o
	gcc main.c -O2 -o drat-trim drat-trim.o

drat-trim.o: drat-trim.c drat-trim.h
	gcc -fPIC drat-trim.c -O2 -c -o drat-trim.o

drat-trim.a: drat-trim.o
	ar rc $@ $^

install: drat-trim.a drat-trim.h drat-trim
	mkdir -p install/lib
	mkdir -p install/include
	mkdir -p install/bin
	cp drat-trim.a install/lib
	cp drat-trim.h install/include
	cp drat-trim install/bin

clean:
	rm -rf drat-trim drat-trim.o drat-trim.a install


