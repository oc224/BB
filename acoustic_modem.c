#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 128
#define SERIAL_TIMEOUT 1000

a_modem modem;

int a_modem_open() {
	//open serial port, go to command mode, issue at (attention), then check response
	if (RS232_OpenComport(a_modem_dev_no, a_modem_serial_baudrate)) {
		printf("Acoustic modem, Fail to open.\n");
		return FAIL;
	}
// ensure command mode
	RS232_SendBuf(a_modem_dev_no, "+++\r", 4);
// detect modem response
	RS232_SendBuf(a_modem_dev_no, "at\r", 3);
	if (a_modem_wait_ack("OK", SERIAL_TIMEOUT) == FAIL) {
		printf("A_modem, fail to open, modem no response\n");
		return FAIL;
	}
	return SUCCESS;
}

inline void a_modem_close() {
	//close serial port
	RS232_CloseComport(a_modem_dev_no);
}

inline void a_modem_clear_io_buffer() {
	// Clear input/output buffer, may put a delay function before this
	RS232_Flush(a_modem_dev_no);
}

int a_modem_set_devel_configs() {
	// set modem the preferable config in devel stage.
	a_modem_clear_io_buffer();
	RS232_SendBuf(a_modem_dev_no, "+++\r", 4);
	RS232_SendBuf(a_modem_dev_no, "@P1EchoChar=Ena\r", 16);
	if (a_modem_wait_ack("p1echochar", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to set\n");
		return FAIL;
	}
	RS232_SendBuf(a_modem_dev_no, "@TxPower=1\r", 11);
	if (a_modem_wait_ack("txpower", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to set\n");
		return FAIL;
	}
	RS232_SendBuf(a_modem_dev_no, "cfg store\r", 10);
	if (a_modem_wait_ack("stored", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to store\n");
		return FAIL;
	}
	return SUCCESS;
}

int a_modem_set_deploy_configs() {
	// set modem the preferable config in deploy stage.
	a_modem_clear_io_buffer();
	RS232_SendBuf(a_modem_dev_no, "+++", 3);
	RS232_SendBuf(a_modem_dev_no, "@TxPower=8\r", 11);
	if (a_modem_wait_ack("txpower", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to set\n");
		return FAIL;
	}
	RS232_SendBuf(a_modem_dev_no, "cfg store\r", 10);
	if (a_modem_wait_ack("stored", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to store\n");
		return FAIL;
	}
	return SUCCESS;
}

int a_modem_play(char * filename) {
	//play a wavform, store the tx time
	char buf[BUFSIZE];
	char buf2[BUFSIZE];
	int n;
	a_modem_clear_io_buffer();
// play
	sprintf(buf, "play /ffs/%s\r", filename); //use strcat instead?
	if (!RS232_SendBuf(a_modem_dev_no, buf, strlen(buf))) {
		printf("Acoustic modem, send command error\n");
		return FAIL;
	}
	if (a_modem_wait_ack("buffering", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, fail to play waveform\n");
		return FAIL;
	}
// get time stamp
	n = a_modem_wait_info("tx", 5000, buf, BUFSIZE); //TODO play command have a variable buffer time
	if (n) {
		buf[n - 1] = 0; //remove carriage return
		printf("info : %s\n", buf);
		sprintf(buf2, "echo '%s,%s' >> TXLOG.TXT", filename, buf);
		system(buf2); //TODO store result using graceful way
		system("echo $(date -Ru) >> TXLOG.TXT");
		return SUCCESS;
	} else {
		printf("A_modem, fail to get TX time\n");
		return FAIL;
	}
}

int a_modem_record(int duration) {
// record waveform, store rx info as well.
	char buf[BUFSIZE], buf2[BUFSIZE];
	char logname[32];
	a_modem_clear_io_buffer();
	/*record on*/
	RS232_SendBuf(a_modem_dev_no, "record on\r", 10);
/*	if (a_modem_wait_ack("ok", SERIAL_TIMEOUT) == FAIL)
		fprintf(stderr, "A_modem, record msg (ok) missing\n");*/
/*	if (a_modem_wait_info("at", SERIAL_TIMEOUT, buf, BUFSIZE) == FAIL)
		fprintf(stderr, "A_modem, record msg (...recorder on...) missing\n");
	sprintf(buf2, "echo '%s' >> RXLOG.TXT", buf);
	system(buf2);*/

	usleep(duration * 1000); //TODO int overflow?
	/*record off*/
	RS232_SendBuf(a_modem_dev_no, "\r", 1);
	RS232_SendBuf(a_modem_dev_no, "record off\r", 11);

	/* get rx filename*/
/*	if (a_modem_wait_info(".wav", SERIAL_TIMEOUT, buf, BUFSIZE) == FAIL)
		fprintf(stderr, "A_modem, record msg (filename.wav) missing\n");
	sprintf(buf2, "echo '%s' >> RXLOG.TXT", buf);
	system(buf2);*/

/*	if (a_modem_wait_info("off at", SERIAL_TIMEOUT, buf, BUFSIZE) == FAIL)
		fprintf(stderr, "A_modem, record msg (...recorder off...) missing\n");
	sprintf(buf2, "echo '%s' >> RXLOG.TXT", buf);
	system(buf2);*/
/*	if (a_modem_wait_ack("ok", SERIAL_TIMEOUT) == FAIL)
		fprintf(stderr, "A_modem, record msg (ok) missing\n");*/
	/*get log name*/
	RS232_SendBuf(a_modem_dev_no, "\r", 1);
	if (a_modem_wait_info("log", SERIAL_TIMEOUT, buf, BUFSIZE) == FAIL)
		fprintf(stderr, "A_modem, record msg (filename.log) missing\n");
	sscanf(buf,"%*s log file %s",logname);
	printf("log name :%s\n",logname);
	sprintf(buf2, "echo '%s' >> RXLOG.TXT",logname+4);
	system(buf2);

	return SUCCESS;
}

int a_modem_sync_time_gps() {
	// sync modem time
	char buf[BUFSIZE];
	char buf2[BUFSIZE];
	RS232_SendBuf(a_modem_dev_no, "gpsd\r", 5);
	a_modem_close();
	sleep(1);
	system("gpspipe -r -n 20 |grep 'GPGGA' >> /dev/ttyUSB2");
	sleep(5);
	a_modem_open();
	a_modem_clear_io_buffer();
	// simple test
	RS232_SendBuf(a_modem_dev_no, "date\r", 5);
	a_modem_wait_info("2014", SERIAL_TIMEOUT, buf, BUFSIZE);
	sprintf(buf2, "echo '%s' >> AMODEM.TXT", buf);
	system(buf2);
	RS232_SendBuf(a_modem_dev_no, "date -store\r", 12);
	RS232_SendBuf(a_modem_dev_no, "@latituder\r", 11);
	RS232_SendBuf(a_modem_dev_no, "@longitude\r", 11);
	//TODO check
	return SUCCESS;
}

int a_modem_sync_clock_gps() {
	// sync clock source
	a_modem_clear_io_buffer();
	// Confirm clock source for the modem
	RS232_SendBuf(a_modem_dev_no, "@SyncPPS\r", 9);
	if (a_modem_wait_ack("4", SERIAL_TIMEOUT) == FAIL) {
		RS232_SendBuf(a_modem_dev_no, "@SyncPPS=4\r", 11);
		//RS232_SendBuf(a_)
		printf("A_modem, syncpps source is not gps, reset the source...\n");
		printf("sync..., this will take 30 seconds or more...\n");
		sleep(30);
	}
	// confirm sync
	RS232_SendBuf(a_modem_dev_no, "sync\r", 5);
	printf("modem sync to pps signal...\n");
	if (a_modem_is_clock_Sync(5, 6) == FAIL) {
		printf("A_modem, fail to sync\n");
		return FAIL;
	}
	return SUCCESS;
}

int a_modem_is_clock_Sync(int samp_interval, int N_retry) {
	//check if clock sync (regardless of clock source), samp_interval (sec)
	//TODO update a_modem strut as well
	int i;
	// input check
	a_modem_clear_io_buffer();
	if ((samp_interval <= 0) | (N_retry <= 0)) {
		printf("IS_SYNC, input error");
		return FAIL;
	}
	// check every X sec
	for (i = 0; i < N_retry; i++) {
		RS232_SendBuf(a_modem_dev_no, "sync\r", 5);
		if (a_modem_wait_ack("synchronized", SERIAL_TIMEOUT)) {
			return SUCCESS;
		}
		sleep(samp_interval);
	}
	printf("a_modem, sync time out\n");
	return FAIL;
}

int a_modem_ls() {
	// list files
	//TODO
	return 0;
}

inline int a_modem_wait_info(char *key_word, int timeout, char *info,
		int info_size) {
	//wait and get for the expecting info, timeout in milliseconds
	return RS232_wait_info(a_modem_dev_no, key_word, timeout, info, info_size);
}

inline int a_modem_wait_ack(char *ack_msg, int timeout) {
	//block until either ack_msg shows or timeout, timeout in milliseconds
	return RS232_wait_ack(a_modem_dev_no, ack_msg, timeout);
}

int a_modem_status() {
// get status (internal temp, pwr cond...) fill struct a_modem
	int return_state = SUCCESS;
	char buf[BUFSIZE];
	RS232_SendBuf(a_modem_dev_no, "atv\r", 4);
	if (a_modem_wait_info("dsp", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "DSP Bat = %f", &modem.dsp_bat);
	} else {
		fprintf(stderr, "A_modem, fail to get modem status(dsp bat)\n");
		return_state = FAIL;
	}
	if (a_modem_wait_info("temp", SERIAL_TIMEOUT, buf, BUFSIZE)) {
		sscanf(buf, "Board Temp = %f", &modem.board_temp);
	} else {
		fprintf(stderr, "A_modem, fail to get modem status (board temp)\n");
		return_state = FAIL;
	}
	RS232_SendBuf(a_modem_dev_no, "mdm_battery\r", 12);
	if (a_modem_wait_info("modem battery", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "Modem Battery = %f", &modem.mdm_bat);
	} else {
		fprintf(stderr, "A_modem, fail to get modem status(modem battery)\n");
		return_state = FAIL;
	}
	RS232_SendBuf(a_modem_dev_no, "rtc_battery\r", 12);
	if (a_modem_wait_info("rtc battery", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "RTC Battery = %f", &modem.rtc_bat);
	} else {
		fprintf(stderr, "A_modem, fail to get modem status(rtc battery)\n");
		return_state = FAIL;
	}
	return return_state;
}

void a_modem_status_show() {
	printf("Board Temp = %f\n", modem.board_temp);
	printf("Modem battery = %f\n", modem.mdm_bat);
	printf("Power = %f\n", modem.dsp_bat);
	printf("RTC battery = %f", modem.rtc_bat);
}

int a_modem_prob(a_network* status) {
	// get atxn atrn info
	return SUCCESS;
}

int a_modem_upload_file(const char *fname){

	return SUCCESS;
}
