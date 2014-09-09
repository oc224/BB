CC=arm-linux-gnueabi-gcc
#CC=arm-linux-gnueabihf-gcc-4.8
CFLAGS= -march=armv7-a -mcpu=cortex-a8  -Wall

all:data_upload exp unicon master slave
unicon.o:unicon.c
	$(CC) $(CFLAGS) -c unicon.c
unicon:unicon.o rs232.o acoustic_modem.o log.o
	$(CC) $(CFLAGS) -o ./bin/unicon unicon.o rs232.o acoustic_modem.o log.o -pthread
rs232.o:rs232.c
	$(CC) $(CFLAGS) -c rs232.c
acoustic_modem.o:acoustic_modem.c
	$(CC) $(CFLAGS) -c acoustic_modem.c -pthread
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
master:master.o acoustic_modem.o rs232.o system.o ms.o scheduler.o log.o
	$(CC) $(CFLAGS) -o ./bin/master master.o acoustic_modem.o rs232.o system.o ms.o scheduler.o log.o -lrt -pthread
ms.o:ms.c
	$(CC) $(CFLAGS) -c ms.c
master.o:master.c
	$(CC) $(CLFAGS) -c master.c
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

clean:
	rm -f *.o
	rm ./bin/*
	
deploy:
	#cp ./bin/* ./home_fs/bin/.
	#cp ./config/*txt ./home_fs/config/.
	#cp ./script/* ./home_fs/script/.
	rsync -avz ./bin/ root@charlie:~/bin/.
	#rsync -avz ./home_fs/ root@dylan:~/.
