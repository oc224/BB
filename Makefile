CC=arm-linux-gnueabi-gcc
#CC=arm-linux-gnueabihf-gcc-4.8
CFLAGS= -march=armv7-a -mcpu=cortex-a8  -Wall -mfloat-abi=soft

all:a_modem_test rs232_test data_upload master  gps_test exp unicon charlie dylan
unicon.o:unicon.c
	$(CC) $(CFLAGS) -c unicon.c
unicon:unicon.o rs232.o acoustic_modem.o
	$(CC) $(CFLAGS) -o ./bin/unicon unicon.o rs232.o acoustic_modem.o
rs232.o:rs232.c
	$(CC) $(CFLAGS) -c rs232.c
acoustic_modem.o:acoustic_modem.c
	$(CC) $(CFLAGS) -c acoustic_modem.c 
rs232_test.o:rs232_test.c
	$(CC) $(CFLAGS) -c rs232_test.c
a_modem_test.o:a_modem_test.c
	$(CC) $(CFLAGS) -c a_modem_test.c 
a_modem_test:a_modem_test.o acoustic_modem.o rs232.o system.o
	$(CC) $(CFLAGS) -o ./bin/a_modem_test a_modem_test.o rs232.o acoustic_modem.o system.o
rs232_test:rs232_test.o rs232.o system.o
	$(CC) $(CFLAGS) -o ./bin/rs232_test rs232_test.o rs232.o system.o
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
data_upload:data_upload.o acoustic_modem.o rs232.o system.o
	$(CC) $(CFLAGS) -o ./bin/data_upload data_upload.o acoustic_modem.o rs232.o system.o
connect.o:connect.c
	$(CC) $(CFLAGS) -c connect.c
connect:connect.o wireless_modem.o rs232.o system.o connect.o
	$(CC) $(CFLAGS) -o ./bin/connect wireless_modem.o rs232.o system.o connect.o
charlie:charlie.o rs232.o acoustic_modem.o system.o ms.o
	$(CC) $(CFLAGS) -o ./bin/charlie charlie.o rs232.o acoustic_modem.o system.o ms.o
charlie.o:charlie.c
	$(CC) $(CFLAGS) -c charlie.c
dylan:dylan.o rs232.o acoustic_modem.o system.o ms.o
	$(CC) $(CFLAGS) -o ./bin/dylan dylan.o rs232.o acoustic_modem.o system.o ms.o
dylan.o:dylan.c
	$(CC) $(CFLAGS) -c dylan.c
slave:slave.o rs232.o acoustic_modem.o system.o ms.o
	$(CC) $(CFLAGS) -o ./bin/slave slave.o rs232.o acoustic_modem.o system.o ms.o
slave.o:slave.c
	$(CC) $(CFLAGS) -c slave.c
master:master.o acoustic_modem.o rs232.o system.o ms.o
	$(CC) $(CFLAGS) -o ./bin/master master.o acoustic_modem.o rs232.o system.o ms.o
ms.o:ms.c
	$(CC) $(CFLAGS) -c ms.c
master.o:master.c
	$(CC) $(CLFAGS) -c master.c
gps_test:gps.o gps_test.o
	$(CC) -o ./bin/gps_test gps.o gps_test.o
gps.o:gps.c
	$(CC) $(CFLAGS) -c gps.c
gps_test.o:gps_test.c
	$(CC) $(CFLAGS) -c gps_test.c

test:sync_test
sync_test.o:sync_test.c
	$(CC) $(CFLAGS) -c sync_test.c
sync_test:rs232.o acoustic_modem.o sync_test.o
	$(CC) $(CFLAGS) -o ./bin/sync_test rs232.o acoustic_modem.o sync_test.o

clean:
	rm -f *.o
	rm ./bin/*
	
deploy:
	cp ./bin/* ./home_fs/bin/.
	cp ./config/*txt ./home_fs/config/.
	cp ./script/* ./home_fs/script/.
	rsync -avz ./home_fs/ root@charlie:~/.
	rsync -avz ./home_fs/ root@dylan:~/.
sync_test.o: sync_test.c
	$(CC) $(CFLAGS) -c sync_test.c
sync_test: rs232.o acoustic_modem.o sync_test.o
	$(CC) $(CFLAGS) -o ./bin/sync_test rs232.o acoustic_modem.o sync_test.o
	
