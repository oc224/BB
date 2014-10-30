#include "log.h"
#include "amodem.h"
#include "common.h"
#include "ms.h"
#include "master.h"
#include "signal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define BUFSIZE 128
#define BUFSHORT 20
#define MASTER_LOGPATH "/root/log/master_log.txt"
#define GUARD_HEAD 0
#define GUARD_TAIL 0
#define ADDR_BROADCAST 255

logger *t_log;
TASK task;

struct SETTING{
float guard_time_head;
float guard_time_tail;
int no_mseq;
int local_addr;
}set;

void command_wait();
int command_exec();
void wait_command_user();
int xcorr();
int atalk();

int main()
{
//amodem init
amodem_init();
//log init
t_log=log_open(MASTER_LOGPATH);
log_show(t_log);
log_event(t_log,0,"master program start");



while (1)
{
	//GATHER COMMAND
	command_wait();

	//Command exec
	command_exec();
}

return 0;
}

int command_exec(){
char remote[20];
char buf_log[BUFSIZE];
//log
sprintf(buf_log,"task %d",task.type);
log_event(t_log,0,buf_log);

// notify remote
if (task.isremote){
sprintf(remote,"REQ%d",task.type);
amodem_puts_remote(ADDR_BROADCAST,remote);}

//do task
switch (task.type)
{
case TALK:
//master_talk();
break;
case ATALK:
ctalk_master();
break;
case CONVERSATION:
//master_con();
break;
case CONEND:
//master_conend();
break;
case QUICK:
//master_quick();
break;
case MSPLAY:
play(task.arg);
break;
case MSRECORD:
record(task.arg);
break;
case SYNCALL:
//master_sync();
break;
case HELP:
help();
break;
case UPLOAD:
upload(task.arg);
break;
case ANAL:
data_anal();
break;
case SEND_REMOTE:
amodem_puts_remote(ADDR_BROADCAST,task.arg);
break;
case MSG_SHOW://ok
printf("local msg\n");
amodem_msg_show(&msg_local);
printf("remote msg\n");
amodem_msg_show(&msg_remote);
break;
case STATUS:
break;
case GPSLOG:
//system("gpspipe -r -n 12 | grep GPGGA >> /home/root/log/gpslog.txt");
break;
case RREBOOT:
//master_rreboot();
break;
case XCORR:
xcorr();
break;
case NONE:
amodem_puts_local(task.arg);
amodem_print(700);
break;
default:
//fprintf(stderr, "ERROR: please input readable command (small letter)\n");
break;
}


}

int data_anal(DATA_COOK *dc){
//upload data xcorr...
char buf[BUFSIZE];
char fname[40];
//char fname[40];
char path_out[120],path_in[120];

//input (default fname)
strcpy(fname,modem.latest_rx_fname);
//upload log
amodem_upload_file(fname);
//upload wav
strcpy(strstr(fname,"log"),"wav");
amodem_upload_file(fname);

//input fname
//sscanf(task.arg,"%*s %s",fname);
sprintf(path_in,"%s/%s",PATH_RAW_DATA,fname);
strcpy(path_out,path_in);
strcpy(strstr(path_out,".wav"),".out");
printf("proc %s, output %s ...\n",fname,path_out);

//enter tx wav
fprintf(stdout,"tx wav:>");
fgets(fname,40,stdin);
fname[strlen(fname)-1]=0;
sprintf(buf,"/root/tx/%s",fname);

wav2CIR(path_in,buf,path_out,dc);

return SUCCESS;
}

int xcorr(){
char buf[BUFSIZE];
char fname[40];
char path_out[120],path_in[120];
DATA_COOK dc = {.snr = 0.0, .avg = 0.0, .max = 0.0, .offset = 0.0, .i_offset = 0, .hh = 0, .mm = 0};
//input fname
sscanf(task.arg,"%*s %s",fname);
sprintf(path_in,"%s/%s.wav",PATH_RAW_DATA,fname);
strcpy(path_out,path_in);
strcpy(strstr(path_out,".wav"),".out");
printf("proc %s, output %s ...\n",fname,path_out);

//enter tx wav
fprintf(stdout,"tx wav:>");
fgets(fname,40,stdin);
fname[strlen(fname)-1]=0;
sprintf(buf,"/root/tx/%s",fname);

wav2CIR(path_in,buf,path_out,&dc);
return SUCCESS;
}

int ctalk_master(){
DATA_COOK dc = {.snr = 0.0, .avg = 0.0, .max = 0.0, .offset = 0.0, .i_offset = 0, .hh = 0, .mm = 0};
char buf[BUFSIZE];
char *dc_send=(char *)malloc(sizeof(DATA_COOK)+1);
//inform remotes
amodem_puts_local("atr3\r");
sleep(11);
//play
amodem_play("t1.wav");
sleep(10);
//record
amodem_record(GUARD_HEAD+2000+GUARD_TAIL*1000);
//anal
data_anal(&dc);
return 0;
}

int ctalk_slave(){
DATA_COOK dc = {.snr = 0.0, .avg = 0.0, .max = 0.0, .offset = 0.0, .i_offset = 0, .hh = 0, .mm = 0};
char buf[BUFSIZE];
char *dc_send=(char *)malloc(sizeof(DATA_COOK)+1);
//wait
sleep(9-GUARD_HEAD);
//record
amodem_record(GUARD_HEAD+2000+GUARD_TAIL*1000);
//
sleep(10-GUARD_HEAD-2-GUARD_TAIL);
//
amodem_play("t1.wav");
//anal send back
data_anal(&dc);
sprintf(buf,"SNR = %4.1f, RX %d:%d:%f\n",dc.snr,dc.hh,dc.mm,dc.ss+dc.offset);
amodem_mode_select('o',3);
amodem_puts_local(buf);
sleep(2);
amodem_puts_local(modem.latest_tx_stamp);
sleep(2);
amodem_mode_select('c',3);
}

void wait_command_user()
{
	/*
	input from console
	output to task field
	*/

	static int cnt=1;
	char arg0[BUFSHORT];//first word in the line
	char buf[BUFSIZE];//line console input
	int type;
	int isremote=0;
	/*console prompt*/
	printf("%s%d>:","master", cnt);
	buf[0]=0;arg0[0]=0;
	fgets(buf, BUFSIZE, stdin);
	sscanf(buf,"%s",arg0);

	// task fill I
	type=EMPTY;
	isremote=0;

	//decode (special)
	if (strcmp(arg0,"exit")==0){
	printf("exit...\n");
	amodem_end();
	exit(0);
	}


	if (strcmp(arg0,"talk")==0){
		type=TALK;
		isremote=1;
	}else if (strcmp(arg0,"atalk")==0){
		type=ATALK;
		isremote=0;
	}else if (strcmp(arg0,"con")==0){
		type=CONVERSATION;
		isremote=1;
	}else if (strcmp(arg0,"conend")==0){
		type=CONEND;
		isremote=1;
	}else if(strcmp(arg0,"play")==0){
		type=MSPLAY;
		isremote=0;
	}else if (strcmp(arg0,"record")==0){
		type=MSRECORD;
		isremote=0;
	}else if (strcmp("sync",arg0)==0){
		type=SYNCALL;
		isremote=1;
	}else if (strcmp("upload",arg0)==0){
		type=UPLOAD;
		isremote=0;
	}else if (strcmp("anal",arg0)==0){
		type=ANAL;
		isremote=0;
	}else if (strcmp("quick",arg0)==0){
		type=QUICK;
		isremote=1;
	}else if (strcmp("status",arg0)==0){
		type=STATUS;
		isremote=1;
	}else if (strcmp("sr",arg0)==0){
		type=SEND_REMOTE;
		isremote=0;
	}else if (strcmp("showmsg",arg0)==0){
		type=MSG_SHOW;
		isremote=0;
	}else if (strcmp("help",arg0)==0){
		type=HELP;
		isremote=0;
	}else if (strcmp("gpslog",arg0)==0){
		type=GPSLOG;
		isremote=0;
	}else if (strcmp("rreboot",arg0)==0){
		type=RREBOOT;
		isremote=1;
	}else if (strcmp("xcorr",arg0)==0){
		type=XCORR;
		isremote=0;
	}else{
		type=NONE;
		isremote=0;
	}
	/*return*/
	if (type>EMPTY){
	cnt++;
	task.type=type;
	task.isremote=isremote;
	}


}

void command_wait(){
wait_command_user();
//or wait_master
}
