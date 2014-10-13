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

logger *t_log;
TASK task_select,task_pool[2];
NODE_MODE node_mode ;

void command_wait();
void wait_command_user();
void node_mode_swap(int mode);
int xcross();
int atalk();
int task_push(int type,int isremote,char *arg,int slot);
void task_pop();

int main()
{
	char remote[20];
	char buf_log[BUFSIZE];
        pthread_attr_t attr;
        pthread_t t_read;

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

		//Do the tasks
		//log
		sprintf(buf_log,"task %d",task_select.type);
		log_event(t_log,0,buf_log);

		// notify remote
		if (task_select.isremote){
		sprintf(remote,"REQ%d",task_select.type);
		amodem_puts_remote(remote);}
		//do task
		switch (task_select.type)
		{
		case TALK:
		master_talk();
		break;
		case ATALK:
		atalk();
		break;
		case CONVERSATION:
		master_con();
		break;
		case CONEND:
		master_conend();
		break;
		case QUICK:
		master_quick();
		break;
		case MSPLAY:
		play(task_select.arg);
		break;
		case MSRECORD:
		record(task_select.arg);
		break;
		case SYNCALL:
		master_sync();
		break;
		case HELP:
		help();
		break;
		case UPLOAD:
		upload(task_select.arg);
		break;
		case SEND_REMOTE:
		amodem_puts_remote(task_select.arg);
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
		system("gpspipe -r -n 12 | grep GPGGA >> /home/root/log/gpslog.txt");
		break;
		case RREBOOT:
		master_rreboot();
		break;
		case XCROSS:
		xcross();
		break;
		case NONE:
		//amodem_puts(task.cmd.arg);
		//amodem_print(1000);
		break;
		default:
		//fprintf(stderr, "ERROR: please input readable command (small letter)\n");
		break;
		}

	}

return 0;
}
int xcross(){
char fname[40];
char output[40];
sscanf(task_select.arg,"%*s %s",fname);
strcpy(output,fname);
strcpy(strstr(output,".wav"),".out");
printf("proc %s, output %s ...\n",fname,output);

wav2CIR(fname,"T1_raw.wav",output);
return SUCCESS;
}
int atalk(){
char buf[BUFSIZE];
switch (node_mode){
case NMASTER:
/*play*/
amodem_play("t1.wav");
/*wait ack*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*record*/
amodem_record(2000);
/*recv stamp*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
break;
case NSLAVE:
/*record*/
amodem_record(2000);
/*send ack*/
amodem_puts_remote(ACK);
/*play*/
amodem_play("t1.wav");
/*send stamp*/
amodem_puts_remote(modem.latest_tx_stamp+8);
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
	int type=EMPTY;
	int isremote=0;
	/*console prompt*/
	printf("%s%d>:","master", cnt);
	buf[0]=0;arg0[0]=0;
	fgets(buf, BUFSIZE, stdin);

	// task fill I
	type=EMPTY;
	isremote=0;

	//decode (special)
	sscanf(buf,"%s",arg0);
	if (strcmp(arg0,"++++")==0){
	node_mode_swap(NMASTER);
	return;}
	else if (strcmp(arg0,"----")==0){
	node_mode_swap(NSLAVE);
	return ;}
	else if (strcmp(arg0,"quit")==0){
	printf("quit...\n");
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
		isremote=1;
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
	}else if (strcmp("xcross",arg0)==0){
		type=XCROSS;
		isremote=0;
	}else{
		type=NONE;
		isremote=0;
	}
	/*return*/
	if (type>NONE){
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

int task_push(int type,int isremote,char *arg,int slot){//13ok
if (task_pool[slot].type!=EMPTY) return FAIL;
strcpy(task_pool[slot].arg,arg);
task_pool[slot].type=type;
task_pool[slot].isremote=isremote;
return SUCCESS;
}

void task_pop(){//13ok
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
printf("task no.%d\n",task_select.type);
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
