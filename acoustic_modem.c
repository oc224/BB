#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 256
#define SERIAL_TIMEOUT 2000
#define TX_LOG "echo $(date -Ru) >> /home/root/log/TXLOG.TXT"
#define TX_PATH "/home/root/log/TXLOG.TXT"
#define RX_LOG "echo $(date -Ru) >> /home/root/log/RXLOG.TXT"
#define RX_PATH "/home/root/log/RXLOG.TXT"
#define GPSPIPE "gpspipe -r -n 20 |grep 'GPGGA' >> /dev/ttyUSB2"
#define ONLINE_COMMAND 2
#define WAIT_INTVAL 100000
#define N_ITER_DIV (WAIT_INTVAL/1000)


a_modem modem;
a_modem_msg msg;


int a_modem_msg_add(char *msg_str){
	if (msg.i<(LIST_SIZE-1))msg.i++;
	else	msg.i=0;
	free(msg.text[msg.i]);
	msg.text[msg.i]=strdup(msg_str);
	//printf("%d\n",msg.i);
	//a_modem_msg_show();
	return SUCCESS;
}
void a_modem_msg_show(){
	int i;
	for (i=0;i<LIST_SIZE;i++)printf("%d %s\n",i,msg.text[i]);
}

int a_modem_init(){
	int i;
	msg.i=0;
	for (i=0;i<LIST_SIZE;i++)msg.text[i]=strdup(" ");
	return SUCCESS;
}
int a_modem_open() {
	//open serial port, go to command mode, issue at (attention), then check response
	if (RS232_OpenComport(a_modem_dev_no, a_modem_serial_baudrate)) {
		printf("Acoustic modem, Fail to open.\n");
		return FAIL;
	}
// ensure command mode
	a_modem_puts("+++\r");
	//RS232_SendBuf(a_modem_dev_no, "+++\r", 4);
/*// detect modem response
	RS232_SendBuf(a_modem_dev_no, "at\r", 3);
	if (a_modem_wait_ack("OK", SERIAL_TIMEOUT) == FAIL) {
		printf("A_modem, fail to open, modem no response\n");
		return FAIL;
	}*/
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
	a_modem_puts("+++\r");
	a_modem_puts("@P1EchoChar=Ena\r");
	//RS232_SendBuf(a_modem_dev_no, , 4);
	//RS232_SendBuf(a_modem_dev_no, "@P1EchoChar=Ena\r", 16);
	if (a_modem_wait_ack("p1echochar", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to set\n");
		return FAIL;
	}
	a_modem_puts("@TxPower=1\r");
	//RS232_SendBuf(a_modem_dev_no, , 11);
	if (a_modem_wait_ack("txpower", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to set\n");
		return FAIL;
	}
	a_modem_puts("cfg store\r");
	//RS232_SendBuf(a_modem_dev_no, , 10);
	if (a_modem_wait_ack("stored", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to store\n");
		return FAIL;
	}
	return SUCCESS;
}

int a_modem_set_deploy_configs() {
	// set modem the preferable config in deploy stage.
	a_modem_clear_io_buffer();
	//RS232_SendBuf(a_modem_dev_no, "+++", 3);
	a_modem_puts("+++\r");
	a_modem_puts("@TxPower=8\r");
	if (a_modem_wait_ack("txpower", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to set\n");
		return FAIL;
	}
	a_modem_puts( "cfg store\r");
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
	//a_modem_clear_io_buffer();
// play
	a_modem_puts("\r");
	sprintf(buf, "play /ffs/%s\r", filename); //use strcat instead?
	if (a_modem_puts(buf)==FAIL) {
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
		sprintf(buf2, "echo '%s,%s' >> %s", filename, buf,TX_PATH);
		system(buf2); //TODO store result using graceful way
		system(TX_LOG);
		return SUCCESS;
	} else {
		printf("A_modem, fail to get TX time\n");
		return FAIL;
	}
}

inline int a_modem_puts(const char*msg){
	// write a line to serial port
	//return RS232_SendBuf(a_modem_dev_no,msg,strlen(msg));
	return RS232_SendBuf(a_modem_dev_no,msg,strlen(msg));
	 //RS232_SendBuf(a_modem_dev_no,"\r",1);
}

inline int a_modem_gets(char* buf,int size){
	// read a line from serial port
	char dump[BUFSIZE];
	int n;
	n=RS232_PollComport(a_modem_dev_no,dump,BUFSIZE);

	//printf("debug gets :%d %s\n",n,dump);
	if (n>3){
		dump[n]=0;
		strcpy(buf,dump);
		if (strstr(dump,"user")==NULL){
		dump[n-1]=0;
		a_modem_msg_add(dump);
		}
	}
	return n;
}

int a_modem_msg_send(const char*msg){
	// write msg acoustically to remotes modem
	a_modem_puts("ato\r");
	if (a_modem_wait_ack("connect",SERIAL_TIMEOUT)==FAIL){
		printf("fail to enter online mode\n");
		return FAIL;
	}
	a_modem_puts(msg);
	if (a_modem_wait_ack("forwarding",SERIAL_TIMEOUT)==FAIL){
		printf("fail to forward msg\n");
		return FAIL;
	}
	sleep(ONLINE_COMMAND);

	a_modem_puts("+++\r");//TODO simple way to confirm we are in command mode
	usleep(300000);
	return SUCCESS;
}

int a_modem_play_smart(char * filename,int mili_sec) {
	//play a wavform, store the tx time
	char buf[BUFSIZE];
	char buf2[BUFSIZE];
	int n;
	//a_modem_clear_io_buffer();
	// tell remote modem to record on
	sprintf(buf,"%d %d",REC_PLY,mili_sec);
	if (a_modem_msg_send(buf)==FAIL){
		return FAIL;
	}

// play
	sprintf(buf, "play /ffs/%s\r", filename); //use strcat instead?
	a_modem_puts(buf);

	if (a_modem_wait_ack("buffering", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, fail to play waveform\n");
		return FAIL;
	}
// get time stamp, store it, send it.
	n = a_modem_wait_info("tx", 5000, buf, BUFSIZE); //TODO play command have a variable buffer time
	if (n) {
		buf[n - 1] = 0; //remove carriage return
		printf("info : %s\n", buf);
		sprintf(buf2,"%d %s",TX,buf);
		a_modem_msg_send(buf2);
		sprintf(buf2, "echo '%s,%s' >> %s", filename, buf,TX_PATH);
		system(buf2); //TODO store result using graceful way
		system(TX_LOG);
		return SUCCESS;
	} else {
		printf("A_modem, fail to get TX time\n");
		return FAIL;
	}
}

int a_modem_slave(){
	char buf[BUFSIZE];
	char buf2[BUFSIZE];
	a_modem_command this_command;
	int dur;
	while(1){
		if (a_modem_wait_info("DATA",SERIAL_TIMEOUT,buf,BUFSIZE)==FAIL)continue;
		//if (sscanf(buf,"DATA(%*d):%s",buf2)<1)continue;
		//printf("debug : %s\n",buf+11);
		sscanf(buf+11,"%d %s",&this_command,buf2);
		switch (this_command){
		case REC:
			dur=atoi(buf2);
			printf("rec %d\n",dur);
			a_modem_record(dur);
			break;
		case TX:
			printf("tx %s\n",buf2);
			break;
		case REC_PLY:
			dur=atoi(buf2);
			printf("rec %d\n",dur);
			a_modem_record(dur);//TODO perhaps sleep a little
			sleep(1);
			a_modem_play_smart("lfm_data_t3_l1.wav",500);
			a_modem_msg_show();
			break;
		default:
			printf("unknown command\n");
			break;

		}
	}
	return SUCCESS;
}

int a_modem_record(int duration) {
// record waveform, store rx info as well.
	char buf[BUFSIZE], buf2[BUFSIZE];
	char logname[32];
	//a_modem_clear_io_buffer();
	/*record on*/
	printf("%s\n",buf);
	a_modem_puts("\r");
	a_modem_puts("record on\r");
	//RS232_SendBuf(a_modem_dev_no, "record on\r", 10);

/*	if (a_modem_wait_ack("ok", SERIAL_TIMEOUT) == FAIL)
		fprintf(stderr, "A_modem, record msg (ok) missing\n");*/
/*	if (a_modem_wait_info("at", SERIAL_TIMEOUT, buf, BUFSIZE) == FAIL)
		fprintf(stderr, "A_modem, record msg (...recorder on...) missing\n");
	sprintf(buf2, "echo '%s' >> RXLOG.TXT", buf);
	system(buf2);*/

	usleep(duration * 1000); //TODO int overflow?
	/*record off*/
	a_modem_puts("\r");
	a_modem_puts( "record off\r");
	//RS232_SendBuf(a_modem_dev_no, "\r", 1);
	//RS232_SendBuf(a_modem_dev_no,, 11);

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
	//RS232_SendBuf(a_modem_dev_no, "\r", 1);
	a_modem_puts("\r");
	if (a_modem_wait_info("log", 4*SERIAL_TIMEOUT, buf, BUFSIZE) == FAIL)
		fprintf(stderr, "A_modem, record msg (filename.log) missing\n");
	sscanf(buf,"%*s log file %s",logname);
	printf("log name :%s\n",logname);
	sprintf(buf2, "echo '%s' >> %s",logname+4,RX_PATH);
	system(RX_LOG);
	system(buf2);
	return SUCCESS;
}

int a_modem_sync_time_gps() {
	// sync modem time
	char buf[BUFSIZE];
	char buf2[BUFSIZE];
	a_modem_puts("gpsd\r");
	//RS232_SendBuf(a_modem_dev_no, "gpsd\r", 5);
	a_modem_close();
	sleep(1);
	system(GPSPIPE);
	sleep(5);
	a_modem_open();
	a_modem_clear_io_buffer();
	// simple test
	a_modem_puts("date\r");
	//RS232_SendBuf(a_modem_dev_no, "date\r", 5);
	a_modem_wait_info("2014", SERIAL_TIMEOUT, buf, BUFSIZE);
	sprintf(buf2, "echo '%s' >> AMODEM.TXT", buf);
	system(buf2);
	a_modem_puts("date -store");
	//RS232_SendBuf(a_modem_dev_no, "date -store\r", 12);
	//RS232_SendBuf(a_modem_dev_no, "@latituder\r", 11);
	//RS232_SendBuf(a_modem_dev_no, "@longitude\r", 11);
	//TODO check
	return SUCCESS;
}

int a_modem_sync_clock_gps() {
	// sync clock source
	a_modem_clear_io_buffer();
	// Confirm clock source for the modem
	a_modem_puts("@SyncPPS\r");
	//RS232_SendBuf(a_modem_dev_no, "@SyncPPS\r", 9);
	if (a_modem_wait_ack("4", SERIAL_TIMEOUT) == FAIL) {
		a_modem_puts("@SyncPPS=4\r");
		//RS232_SendBuf(a_modem_dev_no, "@SyncPPS=4\r", 11);
		//RS232_SendBuf(a_)
		printf("A_modem, syncpps source is not gps, reset the source...\n");
		printf("sync..., this will take 30 seconds or more...\n");
		sleep(30);
	}
	// confirm sync
	a_modem_puts("sync\r");
	//RS232_SendBuf(a_modem_dev_no, "sync\r", 5);
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
		a_modem_puts("sync\r");
		//RS232_SendBuf(a_modem_dev_no, "sync\r", 5);
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
	int n;
		char *buf=(char*)malloc(sizeof(char)*info_size);
		int delay=0;
		int Niter=timeout/N_ITER_DIV;
		while(delay<Niter) {//before timeout
			n=a_modem_gets(buf,info_size);
			if (n<1) {
				delay++;
			} else {
				buf[n]=0;
				if (n<info_size) {
					if (strcasestr(buf,key_word)) {
						strcpy(info,buf); //copy msg
						return n;
					}
				} else {
					printf("info_size to small\n");
					return FAIL;
				}
			}
			usleep(WAIT_INTVAL);
		}
	// timeout
		printf("info timeout\n");
		return FAIL;
}

inline int a_modem_wait_ack(char *ack_msg, int timeout) {
	//block until either ack_msg shows or timeout, timeout in milliseconds
	int n;
	char buf[BUFSIZE];
	int delay=0;
	int Niter=timeout/N_ITER_DIV;
	while(delay<Niter) {
		n=a_modem_gets(buf,BUFSIZE);
		if (n<1) { //input not ready
			delay++;
		} else { // input ready
			buf[n]=0;
			if (strcasestr(buf,ack_msg))return SUCCESS;
		}
		usleep(WAIT_INTVAL);
	}
	printf("ack timeout\n");
	return FAIL;
}

int a_modem_status() {
// get status (internal temp, pwr cond...) fill struct a_modem
	int return_state = SUCCESS;
	char buf[BUFSIZE];
	a_modem_puts("atv\r");
	//RS232_SendBuf(a_modem_dev_no, , 4);
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
	a_modem_puts("mdm_battery\r");
	//RS232_SendBuf(a_modem_dev_no, , 12);
	if (a_modem_wait_info("modem battery", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "Modem Battery = %f", &modem.mdm_bat);
	} else {
		fprintf(stderr, "A_modem, fail to get modem status(modem battery)\n");
		return_state = FAIL;
	}
	a_modem_puts("rtc_battery\r");
	//RS232_SendBuf(a_modem_dev_no, , 12);
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
	//upload a file in modem /sd/ directory
	char buf[BUFSIZE];
	int ret;
	int n_file;
	memset(buf,0,BUFSIZE);
	//check for existence
	sprintf(buf,"ls -l /sd/%s\r",fname);
	RS232_SendBuf(a_modem_dev_no,buf,strlen(buf));
	if (a_modem_wait_info("total",SERIAL_TIMEOUT,buf,BUFSIZE)==FAIL){
		fprintf(stderr,"modem timeout\n");
		return FAIL;
	}
	sscanf(buf,"total of %d file",&n_file);//TODO mem fix
	if (n_file==0){
		printf("file not exist\n");
		return FAIL;
	}
	// copy
	a_modem_clear_io_buffer();
	sprintf(buf,"cp /sd/%s /ffs/%s\r",fname,fname);
	RS232_SendBuf(a_modem_dev_no,buf,strlen(buf));
	/*if (a_modem_wait_ack("error",SERIAL_TIMEOUT)==SUCCESS){
		printf("fail to copy file from /ffs to /sd\n");
		return FAIL;
	}*/
	// wait for the copy
	if (a_modem_wait_ack("ok",60000)==FAIL){//TODO estimate time, proper value
		printf("copy file time out\r");
		return FAIL;
	}
	// issue ymodem send (sb)
	sprintf(buf,"sb /ffs/%s\r",fname);
	RS232_SendBuf(a_modem_dev_no,buf,strlen(buf));
	a_modem_close();
	// rb
	sprintf(buf,"rb -vv >%s<%s",a_modem_dev_path,a_modem_dev_path);
	ret=system(buf);
	printf("rb return %d\n",ret);
	//open
	a_modem_open();
	if (ret!=0){
		printf("fail to use y modem protocol\n");
		return FAIL;
	}
	// delete old files
	RS232_SendBuf(a_modem_dev_no,"\r",1);
	sprintf(buf,"rm /ffs/%s\r",fname);
	RS232_SendBuf(a_modem_dev_no,buf,strlen(buf));
	sprintf(buf,"rm /sd/%s\r",fname);
	RS232_SendBuf(a_modem_dev_no,buf,strlen(buf));
	return SUCCESS;
}

int a_modem_master(){
return 0;
}
