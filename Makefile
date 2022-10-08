FLAGS = -DLONGTYPE

all: drat-trim lrat-check compress decompress gapless

drat-trim: drat-trim.c
	gcc drat-trim.c -std=c99 -O2 -o drat-trim

lrat-check: lrat-check.c
	gcc lrat-check.c -std=c99 $(FLAGS) -O2 -o lrat-check

compress: compress.c
	gcc compress.c -std=c99 $(FLAGS) -O2 -o compress

decompress: decompress.c
	gcc decompress.c -std=c99 $(FLAGS) -O2 -o decompress

gapless: gapless.c
	gcc gapless.c -std=c99 -O2 -o gapless

clean:
	rm drat-trim lrat-check compress decompress gapless
