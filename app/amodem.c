#define _GNU_SOURCE
#include "amodem.h"
#include "rs232.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define BUFSIZE 128

amodem modem;/*a struct that contains the status of modem or some useful information*/
amodem_msg msg_local;/*a list that contains latest msg from (local) modem*/
amodem_msg msg_remote;/*a list that contains latest msg from (remote) modem*/

static void amodem_readthread(void *arg);

static void amodem_readthread(void *arg){
char dump[BUFSIZE];
char *remote_msg;
int n;
while(1){
/*        if (modem.readthread_permit!=1){
	sleep(1);
	printf("closed..\n");
	continue;
	}*/
	usleep(10000);
	n=RS232_PollComport(modem.fd,dump,BUFSIZE);
        if (n<1) continue;
        //printf("%s",dump);
        /*store to input buffer*/
        dump[n-2]='\0';/*remove newline char*/

        /*return if text = user <>*/

        /*store to msg list (local & remote)*/
        if (strstr(dump,"DATA")==NULL)  amodem_msg_push(&msg_local,dump);//local
        else   {//remote
        remote_msg=strstr(dump,":")+1;
        amodem_msg_push(&msg_remote,remote_msg);
        /*if msg from remote , show it*/
        printf("Remote : %s\n",remote_msg);
        /*if go slave request, be slave*/
        }
}
}

int amodem_ffs_clear(){
/*clear ffs system, high usage ie. 98% of ffs system make the modem
behave strange
rm *.log file and *.wav file except *t*.wav file
*/
char fname[80];
amodem_puts("ls /ffs/ \r");
while (amodem_wait_local(NULL,TIMEOUT_SERIAL,fname,80)!=NULL){
if ((strstr(fname,"user")!=NULL)&&(strlen(fname)<7)&& (strstr(fname,"rom")!=NULL)&&(strstr(fname,"ini")!=NULL))continue;
//if (strstr(fname,"log")!=NULL) rm=1;
//if ((strstr(fname,"wav")!=NULL)&&(strstr(fname,"t")==NULL)) rm=1;

//confirm
printf("remove file : %s \n are you sure?\n",fname);
if (fgetc(stdin)!='y') continue;
//remove
amodem_puts("rm /ffs/");
amodem_puts(fname);
amodem_puts("\r");
usleep(100000);

fname[0]=0;
}
return SUCCESS;
}

void amodem_print(int msec){
char *string;
int i;
/*print all the text from serial port*/
for (i=0;i<msec;i++){
string=amodem_msg_pop(&msg_local);
if (string!=NULL) printf("%s\n",string);
usleep(1000);
}

}
void amodem_msg_show(amodem_msg * list){
	/*show msg list*/
	int i;
	printf("MSG LIST\n");
	printf("N_unread = %d, i = %d\n",list->N_unread,list->i);
	for (i=0;i<32;i++)printf("-");
	printf("\n");
	for (i=0;i<LIST_SIZE;i++)printf("%2d %s\n",i,list->text[(list->i+i)%LIST_SIZE]);
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
	if ((modem.tx_p=fopen(PATH_TX,"a"))==NULL){
	fprintf(stderr,"fail to open TX log\n");
	return FAIL;
	}
	if ((modem.rx_p=fopen(PATH_RX,"a"))==NULL){
	fprintf(stderr,"fail to open RX log\n");
	return FAIL;
	}
	/*com logger*/
	modem.com_logger=log_open(PATH_AMODEM);
	log_event(modem.com_logger,0,"amodem init");
	//open serial port
	amodem_open();
	amodem_mode_select('c',3);
	return SUCCESS;
}

int amodem_open() {
	//open serial port, go to command mode, issue at (attention), then check response
//	modem.readthread_permit=1;
	if ((modem.fd=RS232_OpenComport(amodem_dev_path, amodem_serial_baudrate))<1) {
		printf("Acoustic modem, Fail to open.\n");//error
		return FAIL;
	}
	return SUCCESS;
}

inline void amodem_close() {
	//close serial port
//	modem.readthread_permit=0;
	RS232_CloseComport(modem.fd);
}

int amodem_cfg_set(const char *fname) {
	// set modem the preferable config in devel stage.
	FILE *fp;
	char buf[BUFSIZE];
	amodem_puts("+++\r");
	if ((fp=fopen(fname,"r"))==NULL){
	fprintf(stderr,"fail to open %s\n",CFG_DEVEL);
	return FAIL;
	}

	while (fgets(buf,BUFSIZE,fp)>0){
		if (buf[0]=='#')continue;
		amodem_puts(buf);
		amodem_puts("\r");
		amodem_print(TIMEOUT_SERIAL);
	}
	fclose(fp);

	/*store cfg*/
	amodem_puts("cfg store\r");
	if (amodem_wait_local_ack("stored", TIMEOUT_SERIAL) == NULL) {
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
		printf("amodem, send command error\n");
		return FAIL;
	}
	if (amodem_wait_local_ack("buffering", TIMEOUT_SERIAL) == NULL) {
		fprintf(stderr, "amodem, fail to play waveform\n");
		return FAIL;
	}
	// get time stamp
	//TODO buffer time is random
	if (amodem_wait_local("tx",TIMEOUT_SERIAL, buf, BUFSIZE)!=NULL) {
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

int amodem_mode_select(char mode,int N_retry){
int i;
//if in command mode
if (mode=='c'){
amodem_puts("at\r");
if (amodem_wait_local_ack("ok",TIMEOUT_SERIAL)!=NULL) return SUCCESS;}

for (i=0;i<N_retry;i++){
sleep(DELAY_BEFORE_MODE_SWAP);

switch (mode){
case 'o'://online mode
amodem_puts("ato\r");
sleep(DELAY_ONLINE);
if (amodem_wait_local_ack("connect",TIMEOUT_SERIAL)!=NULL) {
sleep(DELAY_AFTER_MODE_SWAP);
return SUCCESS;}
break;

case 'c'://command mode
amodem_puts("+++");
sleep(DELAY_COMMAND);
amodem_puts("at\r");
if (amodem_wait_local_ack("ok",TIMEOUT_SERIAL)!=NULL) {
sleep(DELAY_AFTER_MODE_SWAP);
return SUCCESS;}
break;
default:
fprintf(stderr,"error %s\n",__func__);
return FAIL;
}
}

return FAIL;
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
	amodem_puts("\r");
	amodem_puts( "record off\r\r");


	/*get log name*/
	if (amodem_wait_local("log", 4*TIMEOUT_SERIAL, buf, BUFSIZE)!=NULL){
	sscanf(buf,"%*s log file %s",logname);
	printf("local log file : %s\n",logname+4);
	strcpy(modem.latest_rx_fname,logname+4);
	fprintf(modem.rx_p,"%s\n",logname+4);
	fprintf(modem.rx_p,"sync %d\n",amodem_is_clock_Sync(2));
	fprintf(modem.rx_p,"bb : %s\n",ctime(&bb_stamp));
	fflush(modem.rx_p);
	return SUCCESS;
	}else{
	fprintf(stderr, "%s, record msg (filename.log) missing\n",__func__);
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
	// simple test
	amodem_puts("date\r");
	amodem_wait_local(NULL, TIMEOUT_SERIAL, buf, BUFSIZE);
	// dump
	sprintf(buf2, "echo '%s' >> %s", buf,PATH_AMODEM);
	system(buf2);//SYSTEM DUMP
	amodem_puts("date -store\r");
	//TODO check
	return SUCCESS;
}

int amodem_sync_clock_gps(int sec) {
	// sync modem clock source
	// Confirm clock source for the modem
	amodem_puts("@SyncPPS\r");
	if (amodem_wait_local_ack("2", TIMEOUT_SERIAL) !=NULL) {
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
		if ((new_msg=amodem_msg_pop(&msg_local))!=NULL) {
			if (strcasestr(new_msg,"eA")!=NULL) {
				new_msg=amodem_msg_pop(&msg_local);
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
	for (i = 0; i < sec/(TIMEOUT_SERIAL/1000); i++) {
		amodem_puts("sync\r");
		if (amodem_wait_local_ack("synchronized", TIMEOUT_SERIAL)!=NULL) {
			modem.sync_state=SYNC;
			return SUCCESS;
		}
//		sleep(1);
	}
	modem.sync_state=NOT_SYNC;
	printf("amodem, sync time out\n");
	return FAIL;
}

int amodem_status() {
// get status (internal temp, pwr cond...) fill struct amodem
	int return_state = SUCCESS;
	char buf[BUFSIZE];
	//log
	log_event(modem.com_logger,0,"amodem status:");
	amodem_puts("atv\r");
	if (amodem_wait_local("dsp", TIMEOUT_SERIAL, buf, BUFSIZE) ) {
		sscanf(buf, "DSP Bat = %f", &modem.dsp_bat);
		fprintf(modem.com_logger->fp,"dsp : %f\n",modem.dsp_bat);

	} else {
		fprintf(stderr, "amodem, fail to get modem status(dsp bat)\n");
		return_state = FAIL;
	}
	if (amodem_wait_local("temp", TIMEOUT_SERIAL, buf, BUFSIZE)) {
		sscanf(buf, "Board Temp = %f", &modem.board_temp);
		fprintf(modem.com_logger->fp,"board temp : %f\n",modem.board_temp);
	} else {
		fprintf(stderr, "amodem, fail to get modem status (board temp)\n");
		return_state = FAIL;
	}

	amodem_puts("mdm_battery\r");
	if (amodem_wait_local("modem battery", TIMEOUT_SERIAL, buf, BUFSIZE) ) {
		sscanf(buf, "Modem Battery = %f", &modem.mdm_bat);
		fprintf(modem.com_logger->fp,"mdm bat : %f\n",modem.mdm_bat);

	} else {
		fprintf(stderr, "amodem, fail to get modem status(modem battery)\n");
		return_state = FAIL;
	}

	amodem_puts("rtc_battery\r");
	if (amodem_wait_local("rtc battery", TIMEOUT_SERIAL, buf, BUFSIZE) ) {
		sscanf(buf, "RTC Battery = %f", &modem.rtc_bat);
		fprintf(modem.com_logger->fp,"rtc bat : %f\n",modem.rtc_bat);
	} else {
		fprintf(stderr, "amodem, fail to get modem status(rtc battery)\n");
		return_state = FAIL;
	}
	fflush(modem.com_logger->fp);
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
	//log
	sprintf(buf,"try to upload file %s",fname);
	log_event(modem.com_logger,1,buf);
	// copy
	sprintf(buf,"cp /sd/%s /ffs/%s\r",fname,fname);
	amodem_puts(buf);

	if (amodem_wait_local(NULL,TIMEOUT_COPY,buf,BUFSIZE)==NULL){
	fprintf(stderr,"cp no response\n");
	return FAIL;
	}
	if (strstr(buf,"ok")==NULL){
	printf("copy done \n");
	}else{
	fprintf(stderr,"cp error\n");
	return FAIL;}
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
	// delete old files//TODO hot,
	amodem_puts("\r");
	sprintf(buf,"rm /ffs/%s\r",fname);
	amodem_puts(buf);
	sprintf(buf,"rm /sd/%s\r",fname);
	amodem_puts(buf);
	// check if abnormal file size TODO
	// move file TODO
	return SUCCESS;
}

int amodem_end(){
log_event(modem.com_logger,0,"amodem stop");
fclose(modem.tx_p);
fclose(modem.rx_p);
log_close(modem.com_logger);
amodem_puts("record off\r");//make sure modem is not recording...
amodem_close();
return SUCCESS;
}
