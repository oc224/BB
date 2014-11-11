#define _GNU_SOURCE
#include "master.h"
#include "amodem.h"
#include "rs232.h"
#include "common.h"
#include "gps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define BUFSIZE 128
#define DEFAULT_TXNAME "mseq10_T1_l1"

amodem modem={.board_temp = 0, .dsp_bat = 0, .mdm_bat = 0, .rtc_bat = 0};/*a struct that contains the status of modem or some useful information*/
amodem_msg msg_local={.N_unread = 0, .i = 0};/*a list that contains latest msg from (local) modem*/
amodem_msg msg_remote={.N_unread = 0, .i = 0};/*a list that contains latest msg from (remote) modem*/


static void amodem_readthread(void *arg){
char dump[BUFSIZE];
char *remote_msg;
int n;
int type;
	char *pNL;
while(1){

	usleep(10000);
	pthread_mutex_lock(&modem.readthread);
	n=RS232_PollComport(modem.fd,dump,BUFSIZE);
	pthread_mutex_unlock(&modem.readthread);
        if (n<1) continue;
        /*store to input buffer*/
	if ((pNL=strstr(dump,"\r"))!=NULL) *pNL = '\0';
	if ((pNL=strstr(dump,"\n"))!=NULL) *pNL = '\0';
//        dump[n-2]='\0';/*remove newline char*/

	// packet for address signal
	if (strstr(dump,"$Packet ")!=NULL){
	sscanf(dump,"%*s %*s %*s %d",&type);
	printf("recv command %d\n",type);
	task_push(&task_recv_master,type,0," ");
	continue;}

        /*return if text = user <>*/

        //store to msg list (local & remote)*/
        if (strstr(dump,"DATA")==NULL)  amodem_msg_push(&msg_local,dump);//local
        else   {//remote

        remote_msg=strstr(dump,":")+1;
        //if msg from remote , show it
        printf("Remote : %s\n",remote_msg);

        amodem_msg_push(&msg_remote,remote_msg);


        //if go slave request, be slave*/
	remote_msg=strstr(dump,"REQ");
	if (remote_msg!=NULL){
	remote_msg+=3;
	sscanf(remote_msg,"%d",&type);
	printf("recv command %d\n",type);
	task_push(&task_recv_master,type,0," ");
	continue;
        }

}

}
}

/*
int amodem_ffs_clear(){
//clear ffs system, high usage ie. 98% of ffs system make the modem
//behave strange
//rm *.log file and *.wav file except *t*.wav file
char fname[80];
amodem_puts_local("ls /ffs/ \r");
while (amodem_wait_local(NULL,TIMEOUT_SERIAL,fname,80)!=NULL){
if ((strstr(fname,"user")!=NULL)&&(strlen(fname)<7)&& (strstr(fname,"rom")!=NULL)&&(strstr(fname,"ini")!=NULL))continue;
//if (strstr(fname,"log")!=NULL) rm=1;
//if ((strstr(fname,"wav")!=NULL)&&(strstr(fname,"t")==NULL)) rm=1;

//confirm
printf("remove file : %s \n are you sure?\n",fname);
if (fgetc(stdin)!='y') continue;
//remove
amodem_puts_local("rm /ffs/");
amodem_puts_local(fname);
amodem_puts_local("\r");
usleep(100000);

fname[0]=0;
}
return SUCCESS;
}*/


void amodem_print(int msec){
//print all the text from serial port
char *string;
int i;
for (i=0;i<msec;i++){
string=amodem_msg_pop(&msg_local);
if (string!=NULL) printf("%s\n",string);
usleep(1000);}
}

void amodem_msg_show(amodem_msg * list){
	/*show msg list*/
	int i;
	printf("MSG LIST\n");
	printf("N_unread = %d, i = %d\n",list->N_unread,list->i);
	for (i=0;i<32;i++)printf("-");
	printf("\n");
	for (i=0;i<LIST_SIZE;i++) printf("%2d %s\n",i,list->text[i]);
	printf("\n");
}

int amodem_init(char* dev){
	pthread_attr_t attr;
	pthread_t t_read;

	/*init modem*/
	strcpy(modem.dev_path,dev);
	modem.latest_tx_stamp[0]=0;
	modem.latest_rx_fname[0]=0;
	strcpy(modem.def_tx_wav,DEFAULT_TXNAME);
	modem.sync_state=NOT_SYNC;
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
	// open modem read thread
	pthread_attr_init(&attr);
	pthread_create(&t_read,&attr,(void *)amodem_readthread,NULL);
	// command mode
	amodem_mode_select('c',3);
	return SUCCESS;
}

int amodem_open() {
	printf("open %s\n",modem.dev_path);
	//open serial port, go to command mode, issue at (attention), then check response
	if ((modem.fd=RS232_OpenComport(modem.dev_path, amodem_serial_baudrate))<1) {
		printf("%s, Fail to open.\n",__func__);//error
		return FAIL;
	}
	return SUCCESS;
}

inline void amodem_close() {
	//close serial port
	RS232_CloseComport(modem.fd);
}

int amodem_cfg_set(const char *fname) {
	// set modem the preferable config in devel stage.
	FILE *fp;
	char buf[BUFSIZE];
	amodem_puts_local("+++\r");
	if ((fp=fopen(fname,"r"))==NULL){
	fprintf(stderr,"fail to open %s\n",CFG_DEVEL);
	return FAIL;
	}

	while (fgets(buf,BUFSIZE,fp)>0){
		if (buf[0]=='#')continue;
		amodem_puts_local(buf);
		amodem_puts_local("\r");
		amodem_print(TIMEOUT_SERIAL);
	}
	fclose(fp);

	/*store cfg*/
	amodem_puts_local("cfg store\r");
	if (amodem_wait_ack(&msg_local,"stored", TIMEOUT_SERIAL) == FAIL) {
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
	amodem_puts_local("\r");
	sprintf(buf, "play /ffs/%s.wav\r", filename); 
	if (amodem_puts_local(buf)==FAIL) {
		printf("amodem, send command error\n");
		return FAIL;
	}
	if (amodem_wait_ack(&msg_local,"buffering", TIMEOUT_SERIAL) == FAIL) {
		fprintf(stderr, "amodem, fail to play waveform\n");
		return FAIL;
	}
	// get time stamp
	//TODO buffer time is random
	if (amodem_wait_local("tx",TIMEOUT_SERIAL, buf, BUFSIZE)!=NULL) {
		printf("local tx time : %s\n", buf);
		strcpy(modem.latest_tx_stamp,buf);
		fprintf(modem.tx_p,"%s,%s\n",filename,buf);
		//fprintf(modem.tx_p,"sync %d\n",amodem_is_clock_Sync(2));
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
for (i=0;i<N_retry;i++){

sleep(DELAY_BEFORE_MODE_SWAP);

switch (mode){
case 'o'://online mode
amodem_puts_local("ato\r");
sleep(DELAY_MODE_TRANS);
if (amodem_wait_ack(&msg_local,"CONNECT",TIMEOUT_SERIAL)!=FAIL) {
sleep(DELAY_AFTER_MODE_SWAP);
return SUCCESS;}
break;

case 'c'://command mode
amodem_puts_local("+++");
usleep(10000);
amodem_puts_local("\r");
sleep(DELAY_MODE_TRANS);
amodem_puts_local("at\r");
if (amodem_wait_ack(&msg_local,"ok",TIMEOUT_SERIAL)!=FAIL) {
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
	amodem_puts_local("\r");
	amodem_puts_local("record on\r");

	/*recording*/
	usleep(duration * 1000); //TODO int overflow?

	/*record off*/
	amodem_puts_local("\r");
	amodem_puts_local( "record off\r\r");


	/*get log name*/
	if (amodem_wait_local("log", 4*TIMEOUT_SERIAL, buf, BUFSIZE)!=NULL){
	sscanf(buf,"%*s log file %s",logname);
	printf("local log file : %s\n",logname+4);
	strcpy(modem.latest_rx_fname,logname+4);
	fprintf(modem.rx_p,"%s\n",logname+4);
	//fprintf(modem.rx_p,"sync %d\n",amodem_is_clock_Sync(2));
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
	//prepare
	amodem_puts_local("gpsd\r");
	amodem_close();
	sleep(1);
	//dump gps msg
	system(GPSPIPE);
	// simple test
	amodem_open();
	amodem_puts_local("date\r");
	amodem_wait_ack(&msg_local,"not store",TIMEOUT_SERIAL);
	// store
	amodem_puts_local("date -store\r");
	return SUCCESS;
}

int amodem_sync_clock_gps(int sec) {
	// sync modem clock source
	// Confirm clock source for the modem
	amodem_puts_local("@SyncPPS\r");
	if (amodem_wait_ack(&msg_local,"2", TIMEOUT_SERIAL) !=FAIL) {
		printf("warning, %s,syncpps source is not gps, reset the source...\n",__func__);
		//amodem_puts_local("@SyncPPS=4\r");
		//printf("sync..., this will take 30 seconds or more...\n");
		//sleep(30);
	}
	// confirm sync
	printf("modem sync to pps signal...\n");
	if (amodem_sync_status(sec)==SYNC_FALSE) {
		printf("amodem, fail to sync\n");
		return SYNC_FALSE;
	}
	printf("local modem sync!\n");
	return SYNC_FALSE;
}

int amodem_sync_status(int sec) {
	//check if clock sync (regardless of clock source) until synchronized or timeout
	// to check if modem is sync
	int delay = 0;


	while(delay<sec) { //before timeout
		amodem_puts_local("sync\r");
		if (amodem_wait_ack(&msg_local,"synchronized", TIMEOUT_SERIAL)!=FAIL) {
			modem.sync_state=SYNC;
			return SUCCESS;
		}

		delay++;
		sleep(1);
	}
	// timeout
	modem.sync_state=NOT_SYNC;
	printf("%s, sync time out\n",__func__);
	return FAIL;
}

int amodem_status() {
// get status (internal temp, pwr cond...) fill struct amodem
	int return_state = SUCCESS;
	char buf[BUFSIZE];
	//log
	log_event(modem.com_logger,0,"amodem status:");
	amodem_puts_local("atv\r");
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

	amodem_puts_local("mdm_battery\r");
	if (amodem_wait_local("modem battery", TIMEOUT_SERIAL, buf, BUFSIZE) ) {
		sscanf(buf, "Modem Battery = %f", &modem.mdm_bat);
		fprintf(modem.com_logger->fp,"mdm bat : %f\n",modem.mdm_bat);

	} else {
		fprintf(stderr, "amodem, fail to get modem status(modem battery)\n");
		return_state = FAIL;
	}

	amodem_puts_local("rtc_battery\r");
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
	amodem_puts_local(buf);
	// copy check
	printf("copying\n");
	if (amodem_wait_ack(&msg_local,"ok",TIMEOUT_COPY)==FAIL){
	fprintf(stderr,"cp error\n");
	return FAIL;
	}
	//printf("copy done \n");
	sleep(2);

	// issue ymodem send (sb)
	sprintf(buf,"sb /ffs/%s\r",fname);
	amodem_puts_local(buf);
	amodem_close();
	pthread_mutex_lock(&modem.readthread);
	// rb
	sprintf(buf,"rb -vv >%s<%s",modem.dev_path,modem.dev_path);
	ret=system(buf);//SET PWD TO BE SPECIFIC PATH
	pthread_mutex_unlock(&modem.readthread);
	//printf("rb return %d\n",ret);

	//open
	amodem_open();

	//rm file in ffs
	sprintf(buf,"rm /ffs/%s\r",fname);
	amodem_puts_local(buf);
	if (amodem_wait_ack(&msg_local,"Ok",TIMEOUT_SERIAL)==FAIL) fprintf(stderr,"fail to remove file in /ffs/\n");

	if (ret!=0){
		fprintf(stderr,"%s, rb error\n",__func__);
		return FAIL;
	}

	// mv
	sprintf(buf,"mv ./%s %s",fname,PATH_RAW_DATA);
	system(buf);


	return SUCCESS;
}

int amodem_end(){
log_event(modem.com_logger,0,"amodem stop");
fclose(modem.tx_p);
fclose(modem.rx_p);
log_close(modem.com_logger);
amodem_puts_local("record off\r");//make sure modem is not recording...
amodem_close();
return SUCCESS;
}
