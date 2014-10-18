COMPILER=arm-linux-gnueabihf-
CC=$(COMPILER)gcc
AR=$(COMPILER)ar
#CC=arm-linux-gnueabi-gcc
#CC=arm-linux-gnueabihf-gcc-4.8
CFLAGS= -march=armv7-a -mcpu=cortex-a8  -Wall -mfpu=neon -ftree-vectorize -ffast-math
TARGET1=charlie
TARGET2=dylan
TARGET3=master
LIBNE10=-I ./include/ -L ./lib -lNE10 -lm -mfpu=neon

all:master
unicon.o:unicon.c
	$(CC) $(CFLAGS) -c unicon.c
unicon:unicon.o rs232.o acoustic_modem.o log.o
	$(CC) $(CFLAGS) -o ./bin/unicon unicon.o rs232.o acoustic_modem.o log.o -pthread
rs232.o:rs232.c
	$(CC) $(CFLAGS) -c rs232.c
amodem.o:amodem.c
	$(CC) $(CFLAGS) -c amodem.c -pthread
amodem_io.o:amodem_io.c
	$(CC) $(CFLAGS) -c amodem_io.c
libamodem.a:amodem.o amodem_io.o rs232.o
	$(AR) rcs ./lib/libamodem.a amodem.o amodem_io.o rs232.o
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
exp:scheduler.o exp.o acoustic_modem.o rs232.o system.o log.o
	$(CC) $(CFLAGS) -o ./bin/exp scheduler.o exp.o acoustic_modem.o rs232.o system.o log.o -lrt -pthread
data_upload.o:data_upload.c
	$(CC) $(CFLAGS) -c data_upload.c
data_upload:data_upload.o acoustic_modem.o rs232.o system.o log.o
	$(CC) $(CFLAGS) -o ./bin/data_upload data_upload.o acoustic_modem.o rs232.o system.o  log.o -pthread
connect.o:connect.c
	$(CC) $(CFLAGS) -c connect.c
connect:connect.o wireless_modem.o rs232.o system.o connect.o
	$(CC) $(CFLAGS) -o ./bin/connect wireless_modem.o rs232.o system.o connect.o
slave:slave.o rs232.o acoustic_modem.o system.o ms.o scheduler.o log.o
	$(CC) $(CFLAGS) -o ./bin/slave slave.o rs232.o acoustic_modem.o system.o ms.o scheduler.o log.o -lrt -pthread
slave.o:slave.c
	$(CC) $(CFLAGS) -c slave.c
master:master.o libamodem.a system.o ms.o scheduler.o log.o signal.o wav.o
	$(CC) $(CFLAGS) -o ./bin/master master.o ./lib/libamodem.a system.o ms.o scheduler.o log.o signal.o wav.o -lrt -pthread $(LIBNE10)
ms.o:ms.c
	$(CC) $(CFLAGS) -c ms.c
master.o:master.c
	$(CC) $(CLFAGS) -c master.c $(LIBNE10)
gps.o:gps.c
	$(CC) $(CFLAGS) -c gps.c
log.o:log.c log.h
	$(CC) $(CFLAGS) -c log.c
test:test_sync test_gps
test_sync:rs232.o acoustic_modem.o sync_test.o
	$(CC) $(CFLAGS) -o ./bin/test_sync rs232.o acoustic_modem.o sync_test.o
sync_test.o: sync_test.c
	$(CC) $(CFLAGS) -c sync_test.c
gps_test.o:gps_test.c
	$(CC) $(CFLAGS) -c gps_test.c
test_gps:gps.o gps_test.o
	$(CC) -o ./bin/test_gps gps.o gps_test.o
rs232_test.o:rs232_test.c
	$(CC) $(CFLAGS) -c rs232_test.c
a_modem_test.o:a_modem_test.c
	$(CC) $(CFLAGS) -c a_modem_test.c 
test_amodem:a_modem_test.o acoustic_modem.o rs232.o system.o
	$(CC) $(CFLAGS) -o ./bin/test_amode a_modem_test.o rs232.o acoustic_modem.o system.o
test_rs232:rs232_test.o rs232.o system.o
	$(CC) $(CFLAGS) -o ./bin/test_rs232 rs232_test.o rs232.o system.o
signal.o:signal.c
	$(CC) -c signal.c $(LIBNE10)
wav.o:wav.c
	$(CC) -c wav.c $(LIBNE10)

clean:
	rm -f *.o
	rm ./bin/*
	
deploy:
	rsync -avz ./bin/ root@$(TARGET1):~/bin/.
	rsync -avz ./bin/ root@$(TARGET2):~/bin/.
	rsync -avz ./bin/ root@$(TARGET3):~/bin/.
