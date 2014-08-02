CC=arm-linux-gnueabi-gcc
#CC=arm-linux-gnueabihf-gcc-4.8
CFLAGS= -march=armv7-a -mcpu=cortex-a8  -Wall -mfloat-abi=soft

all:a_modem_test rs232_test w_test exp data_upload connect a_slave
rs232.o:rs232.c
	$(CC) $(CFLAGS) -c rs232.c
acoustic_modem.o:acoustic_modem.c acoustic_modem.h
	$(CC) $(CFLAGS) -c acoustic_modem.c
rs232_test.o:rs232_test.c
	$(CC) $(CFLAGS) -c rs232_test.c
a_modem_test.o:a_modem_test.c
	$(CC) $(CFLAGS) -c a_modem_test.c 
a_modem_test:a_modem_test.o acoustic_modem.o rs232.o
	$(CC) $(CFLAGS) -o ./bin/a_modem_test a_modem_test.o rs232.o \
	acoustic_modem.o
rs232_test:rs232_test.o rs232.o
	$(CC) $(CFLAGS) -o ./bin/rs232_test rs232_test.o rs232.o
wireless_modem.o:wireless_modem.c wireless_modem.h
	$(CC) $(CFLAGS) -c wireless_modem.c rs232.o
w_test.o:w_test.c
	$(CC) $(CFLAGS) -c w_test.c
w_test:w_test.o wireless_modem.o rs232.o
	$(CC) $(CFLAGS) -o ./bin/w_test w_test.o wireless_modem.o rs232.o
system.o:system.c
	$(CC) $(CFLAGS) -c system.c
scheduler.o:scheduler.c
	$(CC) $(CFLAGS) -c -lrt scheduler.c 
exp.o:exp.c
	$(CC) $(CFLAGS) -c -lrt exp.c 
exp:scheduler.o exp.o acoustic_modem.o rs232.o system.o
	$(CC) $(CFLAGS) -o ./bin/exp scheduler.o exp.o acoustic_modem.o rs232.o system.o -lrt
data_upload.o:data_upload.c
	$(CC) $(CFLAGS) -c data_upload.c
data_upload:data_upload.o acoustic_modem.o rs232.o
	$(CC) $(CFLAGS) -o ./bin/data_upload data_upload.o acoustic_modem.o rs232.o
connect.o:connect.c
	$(CC) $(CFLAGS) -c connect.c
connect:connect.o wireless_modem.o rs232.o system.o connect.o
	$(CC) $(CFLAGS) -o ./bin/connect wireless_modem.o rs232.o system.o connect.o
a_slave:a_modem_slave.o rs232.o acoustic_modem.o
	$(CC) $(CFLAGS) -o ./bin/a_slave a_modem_slave.o rs232.o acoustic_modem.o
clean:
	rm -f *.o 
run:
	
deploy:
	scp ./bin/* root@charlie:~/bin/.
	scp ./bin/* root@dylan:~/bin/.
	#scp -r ./config/* root@charlie:~/config/.
	#scp -r ./config/* root@dylan:~/config/.
