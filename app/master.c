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
#define GUARD_HEAD 1
#define GUARD_TAIL 1

logger *t_log;
TASK task_select,task_pool[2];
NODE_MODE node_mode ;

void command_wait();
void wait_command_user();
void node_mode_swap(int mode);
int xcorr();
int atalk();
int task_push(int type,int isremote,char *arg,int slot);
void task_pop();
int command_exec();

int main()
{
//amodem init
amodem_init();
//log init
t_log=log_open(MASTER_LOGPATH);
log_show(t_log);
log_event(t_log,0,"master program start");

//task init
task_push(EMPTY,0," ",0);
task_push(EMPTY,0," ",1);

node_mode_swap(NMASTER);

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
sprintf(buf_log,"task %d",task_select.type);
log_event(t_log,0,buf_log);

// notify remote
if (task_select.isremote){
sprintf(remote,"REQ%d",task_select.type);
amodem_puts_remote(255,remote);}

//do task
switch (task_select.type)
{
case TALK:
//master_talk();
break;
case ATALK:
ctalk();
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
play(task_select.arg);
break;
case MSRECORD:
record(task_select.arg);
break;
case SYNCALL:
//master_sync();
break;
case HELP:
help();
break;
case UPLOAD:
upload(task_select.arg);
break;
case ANAL:
data_anal();
break;
case SEND_REMOTE:
amodem_puts_remote(255,task_select.arg);
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
amodem_puts_local(task_select.arg);
amodem_print(1000);
break;
default:
//fprintf(stderr, "ERROR: please input readable command (small letter)\n");
break;
}


}
int data_anal(DATA_COOK *dc){
//upload data xcorr...
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
//sscanf(task_select.arg,"%*s %s",fname);
sprintf(path_in,"%s/%s",PATH_RAW_DATA,fname);
strcpy(path_out,path_in);
strcpy(strstr(path_out,".wav"),".out");
printf("proc %s, output %s ...\n",fname,path_out);

wav2CIR(path_in,"T1_raw.wav",path_out,dc);

return SUCCESS;


}
int xcorr(){
char fname[40];
char path_out[120],path_in[120];
DATA_COOK dc = {.snr = 0.0, .avg = 0.0, .max = 0.0, .offset = 0.0, .i_offset = 0, .hh = 0, .mm = 0};
//input fname
sscanf(task_select.arg,"%*s %s",fname);
sprintf(path_in,"%s/%s.wav",PATH_RAW_DATA,fname);
strcpy(path_out,path_in);
strcpy(strstr(path_out,".wav"),".out");
printf("proc %s, output %s ...\n",fname,path_out);

wav2CIR(path_in,"T1_raw.wav",path_out,&dc);
return SUCCESS;
}

int atalk(){
char buf[BUFSIZE];
switch (node_mode){
case NMASTER:
/*play*/
amodem_play("t1.wav");
/*wait ack*/
amodem_wait_remote("ACK",REMOTE_TIMEOUT,NULL,0);
//sleep
sleep(DELAY_MODE_SELECT-1);
/*record*/
amodem_record(2000);
/*recv stamp*/
sleep(4*DELAY_MODE_SELECT);
//printf("wait info\n");
amodem_wait_remote("TX",REMOTE_TIMEOUT,buf,BUFSIZE);
printf("Remote TX @ %s\n",buf);
break;
case NSLAVE:
//sleep
sleep(DELAY_MODE_SELECT-1);
/*record*/
amodem_record(2000);
/*send ack*/
amodem_puts_remote(255,ACK);
/*play*/
amodem_play("t1.wav");
/*send stamp*/
amodem_puts_remote(255,modem.latest_tx_stamp);
break;
default:
break;
}
}

int btalk(){
char buf[BUFSIZE];
switch (node_mode){
case NMASTER:
//inform
amodem_puts_local("atr210\r");
sleep(9);
amodem_wait_ack(&msg_local,"Response",2000);
/*play*/
amodem_play("t1.wav");
/*wait ack*/
amodem_wait_ack(&msg_local,"210",6000);
//
sleep(9);
/*record*/
amodem_record(2000);
sleep(2);
/*recv stamp*/
amodem_wait_remote(":",REMOTE_TIMEOUT,buf,BUFSIZE);
printf("Remote TX @ %s\n",buf);
break;
case NSLAVE:
/*wait ack*/
amodem_wait_ack(&msg_local,"210",6000);
sleep(9);
/*record*/
amodem_record(2000);
//inform
amodem_puts_local("atr210\r");
sleep(9);
amodem_wait_ack(&msg_local,"Response",2000);
/*play*/
amodem_play("t1.wav");
/*send stamp*/
amodem_puts_remote(255,modem.latest_tx_stamp+8);
break;
default:
break;
}
}

int ctalk(){
DATA_COOK dc = {.snr = 0.0, .avg = 0.0, .max = 0.0, .offset = 0.0, .i_offset = 0, .hh = 0, .mm = 0};
char buf[BUFSIZE];
char *dc_send=(char *)malloc(sizeof(DATA_COOK)+1);
switch (node_mode){
case NMASTER:
//inform remotes
amodem_puts_local("atr3\r");
sleep(11);
//play
amodem_play("t1.wav");
sleep(1);
//recv dc
amodem_puts_local("@verbose=1\r");
sleep(1);
amodem_mode_select('o',3);
sleep(60);
amodem_wait_msg(&msg_local,NULL,10000,dc_send,sizeof(DATA_COOK)+1);
amodem_mode_select('c',3);
amodem_puts_local("@verbose=3\r");
DATA_COOK_show((DATA_COOK*)dc_send);
break;
case NSLAVE:
//wait
sleep(9-GUARD_HEAD);
//record
amodem_record(2000+GUARD_TAIL*1000);
//
sleep(1);
//anal
data_anal(&dc);
//send dc back
amodem_mode_select('o',3);
memcpy(dc_send,&dc,sizeof(DATA_COOK));
dc_send[sizeof(DATA_COOK)]=0;
amodem_puts_local(dc_send);
sleep(2);
amodem_mode_select('c',3);
DATA_COOK_show((DATA_COOK *)dc_send);
break;
default:
break;
}
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
	if (strcmp(arg0,"++++")==0){
	node_mode_swap(NMASTER);
	return;}
	else if (strcmp(arg0,"----")==0){
	node_mode_swap(NSLAVE);
	return ;}
	else if (strcmp(arg0,"exit")==0){
	printf("exit...\n");
	amodem_end();
	exit(0);
	}

	//decode (normal)
	if (node_mode!=NMASTER){
	printf("This node is not in master mode, unable to give command\n");
	return ;}

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
	task_push(type,isremote,buf,1);
	}


}

void node_mode_swap(int mode){//13ok
switch (mode){
case NMASTER:
node_mode=NMASTER;
printf("go to master mode\n");
break;
case NSLAVE:
node_mode=NSLAVE;
printf("go to slave mode\n");
break;
default:
break;
}

}

int task_push(int type,int isremote,char *arg,int slot){//16ok
if (task_pool[slot].type!=EMPTY) return FAIL;
strcpy(task_pool[slot].arg,arg);
task_pool[slot].type=type;
task_pool[slot].isremote=isremote;
return SUCCESS;
}

void task_pop(){//16ok
if (task_pool[0].type!=EMPTY) {
task_select.type=task_pool[0].type;
task_select.isremote=task_pool[0].isremote;
strcpy(task_select.arg,task_pool[0].arg);
task_pool[0].type=EMPTY;
}else if (task_pool[1].type!=EMPTY) {
task_select.type=task_pool[1].type;
task_select.isremote=task_pool[1].isremote;
strcpy(task_select.arg,task_pool[1].arg);
task_pool[1].type=EMPTY;
}else {
task_select.type=EMPTY;
}
//printf("task no.%d\n",task_select.type);
};

void command_wait(){
while(1){
task_pop();
if (task_select.type != EMPTY) //task exist
break;

if (node_mode==NSLAVE){
usleep(100000);
continue;}else wait_command_user();

}
}
