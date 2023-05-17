FLAGS = -DLONGTYPE

all: drat-trim drat-trim-riscv drat-trim-riscv-native #lrat-check compress decompress gapless

drat-trim: drat-trim.c
	gcc drat-trim.c -std=c99 -g -o drat-trim

drat-trim-riscv: drat-trim.c riscv-compat.h
	clang-11 -DRISCV drat-trim.c -std=c99 -O2 -o drat-trim.S --target=riscv32 -S

drat-trim-riscv-native: drat-trim.c riscv-compat.h
	gcc -DRISCV_SIM drat-trim.c -std=c99 -g -o drat-trim-riscv-native

lrat-check: lrat-check.c
	gcc lrat-check.c -std=c99 $(FLAGS) -O2 -o lrat-check

compress: compress.c
	gcc compress.c -std=c99 $(FLAGS) -O2 -o compress

decompress: decompress.c
	gcc decompress.c -std=c99 $(FLAGS) -O2 -o decompress

gapless: gapless.c
	gcc gapless.c -std=c99 -O2 -o gapless

test: drat-trim-riscv-native
	./drat-trim-riscv-native uuf-30-1.cnf uuf-30-1.drat

clean:
	rm drat-trim lrat-check compress decompress gapless
