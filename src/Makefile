CC=arm-linux-gnueabi-gcc
CFLAGS=-I/home/martin/fftw/include -L/home/martin/fftw/lib
all: test
test:signal.o wav.o
	$(CC) -o test signal.o wav.o -lfftw3 -lm $(CFLAGS)
signal.o:signal.c
	$(CC) -c signal.c -lfftw3 -lm $(CFLAGS)
wav.o:wav.c
	$(CC) -c wav.c -lfftw3 -lm $(CFLAGS)
run:
	./test
	sudo cp res ~/v_share/.
clean:
	rm *.o
deploy:
	scp test root@Charlie:~/bin
	scp *.wav root@Charlie:~/.
