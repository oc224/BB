CC=arm-linux-gnueabi-gcc

all: test
test:signal.o wav.o
	$(CC) -o test signal.o wav.o -lfftw3 -lm -L
signal.o:signal.c
	$(CC) -c signal.c -lfftw3 -lm
wav.o:wav.c
	$(CC) -c wav.c -lfftw3 -lm
run:
	./test
	sudo cp res ~/v_share/.
clean:
	rm *.o
