CC=gcc

all: test
test:signal.o wav.o
	$(CC) -o test signal.o wav.o -lfftw3 -lm
signal.o:signal.c
	$(CC) -c signal.c -lfftw3 -lm
wav.o:wav.c
	$(CC) -c wav.c -lfftw3 -lm
