#define _GNU_SOURCE
#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#define BUFSIZE 100/*default size for buffer*/
#define SERIAL_TIMEOUT 2000/*default timeout for reading modem*/
#define AMODEM_PATH "/home/root/log/AMODEM.TXT"
#define GPSPIPE "gpspipe -r -n 20 |grep 'GPGGA' >> /dev/ttyUSB2" /*feed modem gps GPGGA setence.*/
#define TX_PATH "/home/root/log/TXLOG.TXT"
#define RX_PATH "/home/root/log/RXLOG.TXT"
#define SYCN_TIMEOUT 15
#define COPY_TIMEOUT 60000
a_modem modem;/*a struct that contains the status of modem or some useful information*/
a_modem_msg msg;/*a list that contains latest msg from (local) modem*/
a_modem_msg msg_remote;/*a list that contains latest msg from (remote) modem*/


int a_modem_ffs_clear(){
/*clear ffs system, high usage ie. 98% of ffs system make the modem
behave strange
rm *.log file and *.wav file except *t*.wav file
*/
char fname[80];
int rm;
a_modem_puts("ls /ffs/ \r");
while (a_modem_wait_info(NULL,SERIAL_TIMEOUT,fname,80)==SUCCESS){
printf("remove file : %s\n",fname);
rm=0;
if ((strstr(fname,"user")!=NULL)&&(strlen(fname)<7))continue;
if (strstr(fname,"log")!=NULL) rm=1;
if ((strstr(fname,"wav")!=NULL)&&(strstr(fname,"t")==NULL)) rm=1;
if (rm){
a_modem_puts("rm /ffs/");
a_modem_puts(fname);
a_modem_puts("\r");}
usleep(100000);
fname[0]=0;
}
return SUCCESS;
}

void a_modem_print(int msec){
/*print all the text from serial port*/
int Niter=msec/N_ITER_DIV,delay=0;
char buf[BUFSIZE];

while(delay<Niter){
if (a_modem_gets(buf,BUFSIZE)==FAIL){
delay++;
usleep(WAIT_INTVAL);
}else printf("%s\n",buf);
}
}

int a_modem_msg_add(a_modem_msg *msg_list ,char *msg_str){
	/*add string to msg list*/
	/*move to current index (msg.i)*/
	if (msg_list->i<(LIST_SIZE-1))
	msg_list->i++;
	else
	msg_list->i=0;

	if (msg_list->N_unread<LIST_SIZE) msg_list->N_unread++;
	free(msg_list->text[msg_list->i]);
	msg_list->text[msg_list->i]=strdup(msg_str);
	return SUCCESS;
}

int a_modem_wait_remote(char *buf,int bufsize,int msec){
/*read msg from remote list and store in buf until timeout (miliseconds)*/
int delay=0,Niter=msec/N_ITER_DIV;

while(delay<Niter){//before timeout
if (msg_remote.N_unread>0){/*read the oldest msg from remote msg list*/
if (buf!=NULL)strcpy(buf,MSG_PULL(msg_remote));
msg_remote.N_unread--;
return SUCCESS;
}

a_modem_gets(NULL,0);
usleep(WAIT_INTVAL);
delay++;
}

printf("wait remote timeout\n");
return FAIL;
}

void a_modem_msg_show(a_modem_msg * list){
	/*show msg list*/
	int i;
	printf("MSG LIST\n");
	for (i=0;i<32;i++)printf("-");
	printf("\n");
	for (i=0;i<LIST_SIZE;i++)printf("%2d %s\n",i,list->text[i]);
	printf("\n");
}

int a_modem_init(){
	int i;

	/*init the struct variable*/
	/*init msg list (remote & local)*/
	msg.i=0;
	msg.N_unread=0;
	msg_remote.i=0;
	msg_remote.N_unread=0;
	msg.N_unread=0;
	for (i=0;i<LIST_SIZE;i++){
	msg.text[i]=strdup(" ");
	msg_remote.text[i]=strdup("");
	}
	/*init modem*/
	modem.latest_tx_stamp[0]=0;
	modem.latest_rx_fname[0]=0;
	modem.def_tx_wav[0]=0;
	modem.sync_state=NOT_SYNC;
	modem.board_temp=0;
	modem.dsp_bat=0;
	modem.mdm_bat=0;
	modem.rtc_bat=0;
	/*TX / RX log*/
	if ((modem.tx_p=fopen(TX_PATH,"a"))==NULL){
	fprintf(stderr,"fail to open TX log\n");
	return FAIL;
	}
	if ((modem.rx_p=fopen(RX_PATH,"a"))==NULL){
	fprintf(stderr,"fail to open RX log\n");
	return FAIL;
	}
	return SUCCESS;
}

int a_modem_open() {
	//open serial port, go to command mode, issue at (attention), then check response
	if ((modem.fd=RS232_OpenComport(a_modem_dev_path, a_modem_serial_baudrate))) {
		printf("Acoustic modem, Fail to open.\n");//error
		return FAIL;
	}
	// ensure command mode
	a_modem_puts("+++\r");
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

int a_modem_cfg_set(const char *fname) {
	// set modem the preferable config in devel stage.
	FILE *fp;
	char buf[BUFSIZE];
	a_modem_clear_io_buffer();
	a_modem_puts("+++\r");
	if ((fp=fopen(fname,"r"))==NULL){
	fprintf(stderr,"fail to open %s\n",CFG_DEVEL);
	return FAIL;
	}

	while (fgets(buf,BUFSIZE,fp)>0){
		if (buf[0]=='#')continue;
		a_modem_puts(buf);
		a_modem_puts("\r");
		a_modem_print(SERIAL_TIMEOUT);
	}
	fclose(fp);

	/*store cfg*/
	a_modem_puts("cfg store\r");
	if (a_modem_wait_ack("stored", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, cfg fail to store\n");
		return FAIL;
	}
	return SUCCESS;
}

int a_modem_play(char * filename) {
	//play a wavform, store the tx time
	char buf[BUFSIZE];
	modem.latest_tx_stamp[0]=0;
	time_t bb_stamp;
	time(&bb_stamp);
	// play
	a_modem_puts("\r");
	sprintf(buf, "play /ffs/%s\r", filename); 
	if (a_modem_puts(buf)==FAIL) {
		printf("Acoustic modem, send command error\n");
		return FAIL;
	}
	if (a_modem_wait_ack("buffering", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "A_modem, fail to play waveform\n");
		return FAIL;
	}
	// get time stamp
	//TODO buffer time is random
	if (a_modem_wait_info("tx",WAIT_TXTIME, buf, BUFSIZE)==SUCCESS) {
		printf("local tx time : %s\n", buf);
		strcpy(modem.latest_tx_stamp,buf);
		fprintf(modem.tx_p,"%s,%s\n",filename,buf);
		fprintf(modem.tx_p,"sync %d\n",a_modem_is_clock_Sync(2));
		fprintf(modem.tx_p,"BB:%s\n",ctime(&bb_stamp));
		fflush(modem.tx_p);
		return SUCCESS;
	} else {
		printf("A_modem, fail to get TX time\n");
		return FAIL;
	}
}

inline int a_modem_puts(const char*msg){
	// write a line to serial port
	return RS232_SendBuf(modem.fd,msg,strlen(msg));
}

int a_modem_gets(char* buf,int size){
	/* read a line of text from serial port
	Input
		*buf can be char array ready for store the string 
		or NULL
		*size the array size of buf
	Output
	 	*buf the text read will be stored at buf
		*the the text read will be copy to msg or msg_remote
		*the newline char is removed.
	 	*return FAIL or n char read*/
	char dump[BUFSIZE];
	int n;

	if (buf!=NULL)buf[0]=0;/*make sure input buffer clear if this function fail*/
	n=RS232_PollComport(modem.fd,dump,BUFSIZE);
	if (n<1){
		return FAIL;
	}
	/*store to input buffer*/
	dump[n-1]=0;/*remove newline char*/
	if (buf!=NULL)strcpy(buf,dump);

	/*return if text = user <>*/
	/*if ((strstr(dump,"user")!=NULL)&(strlen(dump)<11)){
	printf("debug : prompt\n");
	return n;
	}*/
	/*store to msg list (local & remote)*/
	dump[n-1]=0;/*remove newline char*/
	if (strstr(dump,"DATA")==NULL)	a_modem_msg_add(&msg,dump);
	else{
	//sscanf(dump,"DATA(%*d):%s",buf2);//TODO
	a_modem_msg_add(&msg_remote,strstr(dump,":")+1);}

	return n;
}

int a_modem_msg_send(const char*msg){
	/* write msg acoustically to remote modems*/
	a_modem_puts("ato\r");
	if (a_modem_wait_ack("connect",2*SERIAL_TIMEOUT)==FAIL){
		printf("fail to enter online mode\n");
		return FAIL;
	}
	a_modem_puts(msg);
	if (a_modem_wait_ack("forwarding",2*SERIAL_TIMEOUT)==FAIL){
		printf("fail to forward msg\n");
		return FAIL;
	}
	usleep(ONLINE_COMMAND);

	a_modem_puts("+++\r");
	usleep(COMMAND_DELAY);
	return SUCCESS;
}

int a_modem_record(int duration) {
/* record waveform, store rx info as well.
duration in mili seconds
*/
	char logname[32];
	time_t bb_stamp;

	modem.latest_rx_fname[0]=0;
	time(&bb_stamp);
	/*record on*/
	a_modem_puts("\r");
	a_modem_puts("record on\r");

	/*recording*/
	usleep(duration * 1000); //TODO int overflow?

	/*record off*/
	a_modem_puts( "record off\r\r");
*/


	/*get log name*/
	if (a_modem_wait_info("log", 4*SERIAL_TIMEOUT, buf, BUFSIZE)==SUCCESS){
	sscanf(buf,"%*s log file %s",logname);
	printf("local log file : %s\n",logname+4);
	strcpy(modem.latest_rx_fname,logname+4);
	fprintf(modem.rx_p,"%s\n",logname+4);
	fprintf(modem.rx_p,"sync %d\n",a_modem_is_clock_Sync(2));
	fprintf(modem.rx_p,"bb : %s\n",ctime(&bb_stamp));
	fflush(modem.rx_p);
	return SUCCESS;
	}else{
	fprintf(stderr, "A_modem, record msg (filename.log) missing\n");
	return FAIL;
	}
}

int a_modem_sync_time_gps() {
	/* sync modem date&time with gps NMEA msg (GPGGA)*/
	char buf[BUFSIZE];
	char buf2[BUFSIZE];
	a_modem_puts("gpsd\r");
	a_modem_close();
	sleep(1);
	system(GPSPIPE);
	//sleep(GPSPIPE_TIME);
	a_modem_open();
	a_modem_clear_io_buffer();
	// simple test
	a_modem_puts("date\r");
	a_modem_wait_info(NULL, SERIAL_TIMEOUT, buf, BUFSIZE);
	// dump
	sprintf(buf2, "echo '%s' >> %s", buf,AMODEM_PATH);
	system(buf2);//SYSTEM DUMP
	a_modem_puts("date -store\r");
	//TODO check
	return SUCCESS;
}

int a_modem_sync_clock_gps(int sec) {
	// sync modem clock source
	a_modem_clear_io_buffer();
	// Confirm clock source for the modem
	a_modem_puts("@SyncPPS\r");
	if (a_modem_wait_ack("2", SERIAL_TIMEOUT) == SUCCESS) {
		a_modem_puts("@SyncPPS=4\r");
		printf("A_modem, syncpps source is not gps, reset the source...\n");
		printf("sync..., this will take 30 seconds or more...\n");
		//sleep(30);
	}
	// confirm sync
	a_modem_puts("sync\r");
	printf("modem sync to pps signal...\n");
	if (a_modem_is_clock_Sync(sec) == FAIL) {
		printf("A_modem, fail to sync\n");
		return FAIL;
	}
	printf("local modem sync!\n");
	return SUCCESS;
}

int a_modem_sync_status() {
	// to check if modem is sync
	char buf[BUFSIZE];
	int n, delay = 0, Niter=2000/N_ITER_DIV;;

	a_modem_puts("sync\r");

	while(delay<Niter) { //before timeout
		n=a_modem_gets(buf,BUFSIZE);
		if (n<1)
			delay++;
		else {
			if (n<BUFSIZE) {
				if (strcasestr(buf,"eA")) {
					a_modem_gets(buf,BUFSIZE);
					printf("sync status is: 	%s",buf);
					return n;
				}
			} else {
				printf("can get the info of sync\n");
				return FAIL;
			}
		}
		usleep(WAIT_INTVAL);
	}
	// timeout
	printf("info timeout\n");
	return FAIL;
}

int a_modem_is_clock_Sync(int sec) {
	//check if clock sync (regardless of clock source) until synchronized or timeout
	int i;
	// check every X sec
	for (i = 0; i < sec/(SERIAL_TIMEOUT/1000); i++) {
		a_modem_puts("sync\r");
		if (a_modem_wait_ack("synchronized", SERIAL_TIMEOUT)) {
			modem.sync_state=SYNC;
			return SUCCESS;
		}
//		sleep(1);
	}
	modem.sync_state=NOT_SYNC;
	printf("a_modem, sync time out\n");
	return FAIL;
}

int a_modem_wait_ack(char *keyword,int timeout){
return a_modem_wait_info(keyword,timeout,NULL,0);
}

int a_modem_wait_info(char *key_word, int timeout, char *info,
		int info_size) {
	/*block until key_word prompt or timout(miliseconds), if key_word prompt, 
	store that line contained key_word to info, the output info is a string
	(end with a NULL char)
	*/
	char buf[BUFSIZE];
	int delay=0;
	int Niter=timeout/N_ITER_DIV;
	if (info!=NULL)info[0]=0;/*make sure input buffer clear when this funtion fail*/
	msg.N_unread=0;
	while(delay<Niter) {//before timeout
		a_modem_gets(buf,BUFSIZE);
		if (msg.N_unread<1) {
			delay++;
		} else {//got new msg
			//new msg match
			if (key_word==NULL){
			if (info!=NULL) strncpy(info,msg.text[msg.i],info_size);
			return SUCCESS;
			}
			if (strcasestr(msg.text[msg.i],key_word)!=NULL) {
			if (info!=NULL) strncpy(info,msg.text[msg.i],info_size);
			return SUCCESS;
			}
		}
		usleep(WAIT_INTVAL);
		delay++;
	}
	// timeout
	printf("info timeout\n");
	return FAIL;
}


int a_modem_status() {
// get status (internal temp, pwr cond...) fill struct a_modem
	int return_state = SUCCESS;
	char buf[BUFSIZE];
	a_modem_puts("atv\r");
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
	if (a_modem_wait_info("modem battery", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "Modem Battery = %f", &modem.mdm_bat);
	} else {
		fprintf(stderr, "A_modem, fail to get modem status(modem battery)\n");
		return_state = FAIL;
	}

	a_modem_puts("rtc_battery\r");
	if (a_modem_wait_info("rtc battery", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "RTC Battery = %f", &modem.rtc_bat);
	} else {
		fprintf(stderr, "A_modem, fail to get modem status(rtc battery)\n");
		return_state = FAIL;
	}
	return return_state;
}

void a_modem_status_show() {
	//show modem status stored in a_modem struct
	printf("Board Temp = %f\n", modem.board_temp);
	printf("Modem battery = %f\n", modem.mdm_bat);
	printf("Power = %f\n", modem.dsp_bat);
	printf("RTC battery = %f\n", modem.rtc_bat);
	switch (modem.sync_state){
	case SYNC:
		printf("sync.\n");
	break;
	case NOT_SYNC:
		printf("not sync\n");
	break;
	default:
		printf("unknown state\n");
	break;
	}
}

int a_modem_upload_file(const char *fname){
	/*upload a file in modem /sd/ directory to current path
	then remove the file*/
	char buf[BUFSIZE];
	int ret;
	// copy
	a_modem_clear_io_buffer();
	sprintf(buf,"cp /sd/%s /ffs/%s\r",fname,fname);
	a_modem_puts(buf);
	a_modem_wait_info(NULL,SERIAL_TIMEOUT,NULL,BUFSIZE);
	if (a_modem_wait_info(NULL,COPY_TIMEOUT,buf,BUFSIZE)==FAIL){
	fprintf(stderr,"cp no response\n");
	return FAIL;
	}
	if (strstr(buf,"ok")==NULL){
	printf("copy done \n");
	}else{
	fprintf(stderr,"cp error\n");
	return FAIL;}
	/*if (strcasestr(buf,"error")!=NULL){
	fprintf(stderr,"cp error\n");
	return FAIL;
	}
	if (strcasestr(buf,"ok")!=NULL){
	fprintf(stdout,"cp done\n");
	}else{
	fprintf(stderr,"cp error\n");
	return FAIL;
	}*/
	/*sleep(2);
	buf[0]=0;
	a_modem_gets(buf,BUFSIZE);
	if (strcasestr(buf,"error")!=NULL){
		fprintf(stderr,"fail to copy files in a modem\n");
		return FAIL;
	}
	// wait for the copy
	if (a_modem_wait_ack("ok",60000)==FAIL){//TODO estimate time, proper value
		printf("copy file time out\r");
		return FAIL;
	}
	*/
	// issue ymodem send (sb)
	sprintf(buf,"sb /ffs/%s\r",fname);
	a_modem_puts(buf);
	a_modem_close();
	// rb
	sprintf(buf,"rb -vv >%s<%s",a_modem_dev_path,a_modem_dev_path);
	ret=system(buf);//SET PWD TO BE SPECIFIC PATH
	printf("rb return %d\n",ret);
	//open
	a_modem_open();
	if (ret!=0){
		printf("fail to use y modem protocol\n");
		return FAIL;
	}
	// delete old files
	a_modem_puts("\r");
	sprintf(buf,"rm /ffs/%s\r",fname);
	a_modem_puts(buf);
	sprintf(buf,"rm /sd/%s\r",fname);
	a_modem_puts(buf);
	//move the file
	sprintf(buf,"mv ./%s /home/root/data/.",fname);
	system(buf);
	return SUCCESS;
}

int a_modem_end(){
fclose(modem.tx_p);
fclose(modem.rx_p);
a_modem_puts("record off\r");//make sure modem is not recording...
a_modem_close();
return SUCCESS;
}
