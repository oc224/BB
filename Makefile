CC=arm-linux-gnueabi-gcc
CFLAGS= -march=armv7-a -mcpu=cortex-a8 -mfloat-abi=soft -Wall

all:a_modem_test rs232_test
rs232.o:rs232.c
	$(CC) $(CFLAGS) -c rs232.c
acoustic_modem.o:acoustic_modem.c acoustic_modem.h
	$(CC) $(CFLAGS) -c acoustic_modem.c
rs232_test.o:rs232_test.c
	$(CC) $(CFLAGS) -c rs232_test.c
a_modem_test.o:a_modem_test.c
	$(CC) $(CFLAGS) -c a_modem_test.c 
a_modem_test:a_modem_test.o acoustic_modem.o rs232.o
	$(CC) $(CFLAGS) -o a_modem_test a_modem_test.o rs232.o \
	acoustic_modem.o
rs232_test:rs232_test.o rs232.o
	$(CC) $(CFLAGS) -o rs232_test rs232_test.o rs232.o

clean:
	rm -f *.o 
run:
	
deploy:
	mv a_modem_test bin/.
	mv rs232_test bin/.
	scp ./bin/* root@charlie:~/.
