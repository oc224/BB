#define _GNU_SOURCE
#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define BUFSIZE 100/*default size for buffer*/
#define SERIAL_TIMEOUT 2000/*default timeout for reading modem*/
#define AMODEM_PATH "/home/root/log/AMODEM.TXT"
#define GPSPIPE "gpspipe -r -n 20 |grep 'GPGGA' >> /dev/ttyUSB2" /*feed modem gps GPGGA setence.*/
#define TX_PATH "/home/root/log/TXLOG.TXT"
#define RX_PATH "/home/root/log/RXLOG.TXT"
#define SYCN_TIMEOUT 15
#define COPY_TIMEOUT 60000
#define ATODELAY 500000
amodem modem;/*a struct that contains the status of modem or some useful information*/
amodem_msg msg_local;/*a list that contains latest msg from (local) modem*/
amodem_msg msg_remote;/*a list that contains latest msg from (remote) modem*/

static void amodem_readthread(void *arg);

int amodem_ffs_clear(){
/*clear ffs system, high usage ie. 98% of ffs system make the modem
behave strange
rm *.log file and *.wav file except *t*.wav file
*/
char fname[80];
int rm;
amodem_puts("ls /ffs/ \r");
while (amodem_wait_info(NULL,SERIAL_TIMEOUT,fname,80)==SUCCESS){
printf("remove file : %s\n",fname);
rm=0;
if ((strstr(fname,"user")!=NULL)&&(strlen(fname)<7))continue;
if (strstr(fname,"log")!=NULL) rm=1;
if ((strstr(fname,"wav")!=NULL)&&(strstr(fname,"t")==NULL)) rm=1;
if (rm){
amodem_puts("rm /ffs/");
amodem_puts(fname);
amodem_puts("\r");}
usleep(100000);
fname[0]=0;
}
return SUCCESS;
}
int amodem_msg_send(const char *str){

return SUCCESS;
}

void amodem_print(int msec){
/*print all the text from serial port*/
int Niter=msec/N_ITER_DIV,delay=0;
char* new_msg;

while(delay<Niter){
if ((new_msg=amodem_msg_pull(&msg_local))!=NULL) printf("%s\n",new_msg);
delay++;
usleep(WAIT_INTVAL);
}
}

int amodem_msg_add(amodem_msg *msg_list ,char *msg_str){
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

int amodem_wait_remote(char *buf, int bufsize, int msec) {
return amodem_wait_msg(&msg_remote,NULL,msec,buf,bufsize);
}

void amodem_msg_show(amodem_msg * list){
	/*show msg list*/
	int i;
	printf("MSG LIST\n");
	for (i=0;i<32;i++)printf("-");
	printf("\n");
	for (i=0;i<LIST_SIZE;i++)printf("%2d %s\n",i,list->text[i]);
	printf("\n");
}

int amodem_init(){
	int i;
	pthread_attr_t attr;
	pthread_t t_read;
	/*init the struct variable*/
	/*init msg list (remote & local)*/
	msg_local.i=0;
	msg_local.N_unread=0;
	msg_remote.i=0;
	msg_remote.N_unread=0;
	for (i=0;i<LIST_SIZE;i++){
	msg_local.text[i]=strdup("");
	msg_remote.text[i]=strdup("");
	}
	// open modem read thread
	pthread_attr_init(&attr);
	pthread_create(&t_read,&attr,(void *)amodem_readthread,NULL);
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

static void amodem_readthread(void *arg){
char dump[BUFSIZE];
int n;

while(1){

        n=RS232_PollComport(modem.fd,dump,BUFSIZE);
        if (n<1) continue;
        /*store to input buffer*/
        dump[n-1]=0;/*remove newline char*/

        /*return if text = user <>*/

        /*store to msg list (local & remote)*/
        if (strstr(dump,"DATA")==NULL)  amodem_msg_add(&msg_local,dump);
        else   amodem_msg_add(&msg_remote,strstr(dump,":")+1);

	/*if msg from remote , show it*/

	/*if go slave request, be slave*/

}
}

int amodem_open() {
	//open serial port, go to command mode, issue at (attention), then check response
	if ((modem.fd=RS232_OpenComport(amodem_dev_path, amodem_serial_baudrate))) {
		printf("Acoustic modem, Fail to open.\n");//error
		return FAIL;
	}
	// ensure command mode
	amodem_puts("+++\r");
	return SUCCESS;
}

inline void amodem_close() {
	//close serial port
	RS232_CloseComport(amodem_dev_no);
}

inline void amodem_clear_io_buffer() {
	// Clear input/output buffer, may put a delay function before this
	RS232_Flush(amodem_dev_no);
}

int amodem_cfg_set(const char *fname) {
	// set modem the preferable config in devel stage.
	FILE *fp;
	char buf[BUFSIZE];
	amodem_clear_io_buffer();
	amodem_puts("+++\r");
	if ((fp=fopen(fname,"r"))==NULL){
	fprintf(stderr,"fail to open %s\n",CFG_DEVEL);
	return FAIL;
	}

	while (fgets(buf,BUFSIZE,fp)>0){
		if (buf[0]=='#')continue;
		amodem_puts(buf);
		amodem_puts("\r");
		amodem_print(SERIAL_TIMEOUT);
	}
	fclose(fp);

	/*store cfg*/
	amodem_puts("cfg store\r");
	if (amodem_wait_ack("stored", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "amodem, cfg fail to store\n");
		return FAIL;
	}
	return SUCCESS;
}

int amodem_play(char * filename) {
	//play a wavform, store the tx time
	char buf[BUFSIZE];
	modem.latest_tx_stamp[0]=0;
	time_t bb_stamp;
	time(&bb_stamp);
	// play
	amodem_puts("\r");
	sprintf(buf, "play /ffs/%s\r", filename); 
	if (amodem_puts(buf)==FAIL) {
		printf("Acoustic modem, send command error\n");
		return FAIL;
	}
	if (amodem_wait_ack("buffering", SERIAL_TIMEOUT) == FAIL) {
		fprintf(stderr, "amodem, fail to play waveform\n");
		return FAIL;
	}
	// get time stamp
	//TODO buffer time is random
	if (amodem_wait_info("tx",WAIT_TXTIME, buf, BUFSIZE)==SUCCESS) {
		printf("local tx time : %s\n", buf);
		strcpy(modem.latest_tx_stamp,buf);
		fprintf(modem.tx_p,"%s,%s\n",filename,buf);
		fprintf(modem.tx_p,"sync %d\n",amodem_is_clock_Sync(2));
		fprintf(modem.tx_p,"BB:%s\n",ctime(&bb_stamp));
		fflush(modem.tx_p);
		return SUCCESS;
	} else {
		printf("amodem, fail to get TX time\n");
		return FAIL;
	}
}

inline int amodem_puts(const char*msg){
	// write a line to serial port
	msg_local.N_unread=0;
	msg_remote.N_unread=0;
	return RS232_SendBuf(modem.fd,msg,strlen(msg));
}

int amodem_mode_select(char mode){
//usleep()
switch (mode){
case 'o':
amodem_puts("ato\r");
if (amodem_wait_ack("connect",SERIAL_TIMEOUT)==FAIL)
break;
case 'c':
amodem_puts("+++");
//usleep()
amodem_puts("at\r");
if (amodem_wait_ack("ok",SERIAL_TIMEOUT)==FAIL)
break;
}


	/* write msg acoustically to remote modems*/
	amodem_puts("ato\r");
	if (amodem_wait_ack("connect",2*SERIAL_TIMEOUT)==FAIL){
		fprintf(stderr,"msg_send, fail to enter online mode\n");
		return FAIL;
	}
	usleep(ATODELAY);
	if (amodem_wait_ack("forwarding",2*SERIAL_TIMEOUT)==FAIL){
		fprintf(stderr,"msg_send, fail to forward msg\n");
		return FAIL;
	}
	usleep(ONLINE_COMMAND);

	amodem_puts("+++\r");
	usleep(COMMAND_DELAY);
	return SUCCESS;
}

int amodem_record(int duration) {
/* record waveform, store rx info as well.
duration in mili seconds
*/
	char logname[32];
	char buf[BUFSIZE];
	time_t bb_stamp;

	modem.latest_rx_fname[0]=0;
	time(&bb_stamp);
	/*record on*/
	amodem_puts("\r");
	amodem_puts("record on\r");

	/*recording*/
	usleep(duration * 1000); //TODO int overflow?

	/*record off*/
	amodem_puts( "record off\r\r");


	/*get log name*/
	if (amodem_wait_info("log", 4*SERIAL_TIMEOUT, buf, BUFSIZE)==SUCCESS){
	sscanf(buf,"%*s log file %s",logname);
	printf("local log file : %s\n",logname+4);
	strcpy(modem.latest_rx_fname,logname+4);
	fprintf(modem.rx_p,"%s\n",logname+4);
	fprintf(modem.rx_p,"sync %d\n",amodem_is_clock_Sync(2));
	fprintf(modem.rx_p,"bb : %s\n",ctime(&bb_stamp));
	fflush(modem.rx_p);
	return SUCCESS;
	}else{
	fprintf(stderr, "amodem, record msg (filename.log) missing\n");
	return FAIL;
	}
}

int amodem_sync_time_gps() {
	/* sync modem date&time with gps NMEA msg (GPGGA)*/
	char buf[BUFSIZE];
	char buf2[BUFSIZE];
	amodem_puts("gpsd\r");
	amodem_close();
	sleep(1);
	system(GPSPIPE);
	//sleep(GPSPIPE_TIME);
	amodem_open();
	amodem_clear_io_buffer();
	// simple test
	amodem_puts("date\r");
	amodem_wait_info(NULL, SERIAL_TIMEOUT, buf, BUFSIZE);
	// dump
	sprintf(buf2, "echo '%s' >> %s", buf,AMODEM_PATH);
	system(buf2);//SYSTEM DUMP
	amodem_puts("date -store\r");
	//TODO check
	return SUCCESS;
}

int amodem_sync_clock_gps(int sec) {
	// sync modem clock source
	amodem_clear_io_buffer();
	// Confirm clock source for the modem
	amodem_puts("@SyncPPS\r");
	if (amodem_wait_ack("2", SERIAL_TIMEOUT) == SUCCESS) {
		amodem_puts("@SyncPPS=4\r");
		printf("amodem, syncpps source is not gps, reset the source...\n");
		printf("sync..., this will take 30 seconds or more...\n");
		//sleep(30);
	}
	// confirm sync
	amodem_puts("sync\r");
	printf("modem sync to pps signal...\n");
	if (amodem_is_clock_Sync(sec) == FAIL) {
		printf("amodem, fail to sync\n");
		return FAIL;
	}
	printf("local modem sync!\n");
	return SUCCESS;
}

int amodem_sync_status() {
	// to check if modem is sync
	char* new_msg;
	int delay = 0, Niter=2000/N_ITER_DIV;;

	amodem_puts("sync\r");

	while(delay<Niter) { //before timeout
		if ((new_msg=amodem_msg_pull(&msg_local))!=NULL) {
			if (strcasestr(new_msg,"eA")) {
				new_msg=amodem_msg_pull(&msg_local);
				printf("sync status is: 	%s",new_msg);
				return SUCCESS;
			} else {
				printf("can get the info of sync\n");
				return FAIL;
			}
		}
		delay++;
		usleep(WAIT_INTVAL);
	}
	// timeout
	printf("info timeout\n");
	return FAIL;
}

int amodem_is_clock_Sync(int sec) {
	//check if clock sync (regardless of clock source) until synchronized or timeout
	int i;
	// check every X sec
	for (i = 0; i < sec/(SERIAL_TIMEOUT/1000); i++) {
		amodem_puts("sync\r");
		if (amodem_wait_ack("synchronized", SERIAL_TIMEOUT)) {
			modem.sync_state=SYNC;
			return SUCCESS;
		}
//		sleep(1);
	}
	modem.sync_state=NOT_SYNC;
	printf("amodem, sync time out\n");
	return FAIL;
}

int amodem_wait_ack(char *keyword,int timeout){
return amodem_wait_info(keyword,timeout,NULL,0);
}

char* amodem_msg_pull(amodem_msg* msg){
/*point to oldest unread msg*/
char* ret;
if (msg->N_unread==0) ret=NULL;
ret = msg->text[(msg->i+LIST_SIZE+msg->N_unread-1)%LIST_SIZE];
msg->N_unread--;
return ret;
}

int amodem_wait_info(char *key_word, int timeout, char *info,
		int info_size) {
return amodem_wait_msg(&msg_local,key_word,timeout,info,info_size);
}


int amodem_wait_msg(amodem_msg *msg,char *key_word, int timeout, char *info,
		int info_size) {
	/*block until key_word prompt or timout(miliseconds), if key_word prompt, 
	store that line contained key_word to info, the output info is a string
	(end with a NULL char)
	*/

	char *new_msg;
	int delay=0;
	int Niter=timeout/N_ITER_DIV;
	int is_copy=(info!=NULL);
	int is_nullkeyword=(key_word==NULL);
	int ret=FAIL;

	if (is_copy) info[0]=0;/*make sure input buffer clear when this funtion fail*/

	while(delay<Niter) {//before timeout
		if ((new_msg=amodem_msg_pull(msg))!=NULL) {//got new msg
			//new msg match
			if ((is_nullkeyword)||(strcasestr(new_msg,key_word)!=NULL)) {
			ret=SUCCESS;
			break;	}
		}
		usleep(WAIT_INTVAL);
		delay++;
	}
	if ((ret==SUCCESS)&&(is_copy)) strncpy(info,new_msg,info_size);
	return ret;
}


int amodem_status() {
// get status (internal temp, pwr cond...) fill struct amodem
	int return_state = SUCCESS;
	char buf[BUFSIZE];
	amodem_puts("atv\r");
	if (amodem_wait_info("dsp", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "DSP Bat = %f", &modem.dsp_bat);
	} else {
		fprintf(stderr, "amodem, fail to get modem status(dsp bat)\n");
		return_state = FAIL;
	}
	if (amodem_wait_info("temp", SERIAL_TIMEOUT, buf, BUFSIZE)) {
		sscanf(buf, "Board Temp = %f", &modem.board_temp);
	} else {
		fprintf(stderr, "amodem, fail to get modem status (board temp)\n");
		return_state = FAIL;
	}

	amodem_puts("mdm_battery\r");
	if (amodem_wait_info("modem battery", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "Modem Battery = %f", &modem.mdm_bat);
	} else {
		fprintf(stderr, "amodem, fail to get modem status(modem battery)\n");
		return_state = FAIL;
	}

	amodem_puts("rtc_battery\r");
	if (amodem_wait_info("rtc battery", SERIAL_TIMEOUT, buf, BUFSIZE) ) {
		sscanf(buf, "RTC Battery = %f", &modem.rtc_bat);
	} else {
		fprintf(stderr, "amodem, fail to get modem status(rtc battery)\n");
		return_state = FAIL;
	}
	return return_state;
}

void amodem_status_show() {
	//show modem status stored in amodem struct
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

int amodem_upload_file(const char *fname){
	/*upload a file in modem /sd/ directory to current path
	then remove the file*/
	char buf[BUFSIZE];
	int ret;
	// copy
	amodem_clear_io_buffer();
	sprintf(buf,"cp /sd/%s /ffs/%s\r",fname,fname);
	amodem_puts(buf);

	if (amodem_wait_info(NULL,COPY_TIMEOUT,buf,BUFSIZE)==FAIL){
	fprintf(stderr,"cp no response\n");
	return FAIL;
	}
	if (strstr(buf,"ok")==NULL){
	printf("copy done \n");
	}else{
	fprintf(stderr,"cp error\n");
	return FAIL;}
	/*if (strcasestr(buf,"error")!=NULL){
	return FAIL;
	}
	if (strcasestr(buf,"ok")!=NULL){
	fprintf(stdout,"cp done\n");
	}else{
	fprintf(stderr,"cp error\n");
	return FAIL;
	}*/
	// issue ymodem send (sb)
	sprintf(buf,"sb /ffs/%s\r",fname);
	amodem_puts(buf);
	amodem_close();
	// rb
	sprintf(buf,"rb -vv >%s<%s",amodem_dev_path,amodem_dev_path);
	ret=system(buf);//SET PWD TO BE SPECIFIC PATH
	printf("rb return %d\n",ret);
	//open
	amodem_open();
	if (ret!=0){
		printf("fail to use y modem protocol\n");
		return FAIL;
	}
	// delete old files
	amodem_puts("\r");
	sprintf(buf,"rm /ffs/%s\r",fname);
	amodem_puts(buf);
	sprintf(buf,"rm /sd/%s\r",fname);
	amodem_puts(buf);
	system(buf);
	return SUCCESS;
}

int amodem_end(){
fclose(modem.tx_p);
fclose(modem.rx_p);
amodem_puts("record off\r");//make sure modem is not recording...
amodem_close();
return SUCCESS;
}
