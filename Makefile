CC=arm-linux-gnueabi-gcc
CFLAGS= -march=armv7-a -mcpu=cortex-a8 -mfloat-abi=soft

all:connect
connect:connect.o rs232.o
	$(CC) $(CFLAGS) -o connect connect.o rs232.o
connect.o:connect.c
	$(CC) $(CFLAGS) -c connect.c
rs232.o:rs232.c
	$(CC) $(CFLAGS) -c rs232.c
clean:
	rm -f *.o connect
run:program
	./connect
deploy:program
	scp ./connect root@charlie:~/.
