#include "log.h"
#include "amodem.h"
#include "common.h"
#include "ms.h"
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
static pthread_mutex_t mtx_task = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_stdin = PTHREAD_MUTEX_INITIALIZER;

typedef enum {NMASTER,NSLAVE} NODE_MODE;
typedef char bool;


typedef enum{
	EMPTY,
        NONE,
        TALK,//recipricol transmission, ok
        ATALK,
        CONVERSATION,
        CONEND,
        QUICK,
        MSPLAY,//local modem play
        MSRECORD,//local modem record
        SYNCALL,//sync remote & local modem
        HELP,//show help msg
        UPLOAD,//upload local files
        SEND_REMOTE,//send msg to remote modems
        MSG_SHOW,//show msg & msg_remote
        STATUS,
        GPSLOG,
        RREBOOT,
	XCROSS
}cmd_type;

//command info
typedef struct{
cmd_type type;
bool isremote;
char *arg;
}CMD;

//info (status, info)
typedef struct{
NODE_MODE mode;
CMD cmd;
}node_task;

node_task task;

int wait_command_master(){
	/*
	input remote
	output to CMD field
	*/
        int ret=FAIL;
        static int cnt=1;
        char buf[BUFSIZE];

        if (amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT)==NULL)return FAIL;
        printf("remote:%s \n",buf);

        /*decode*/
        if (sscanf(buf,"%d",&ret) < 1)
                ret = FAIL;

        /*return*/
        if (ret!=FAIL){
                cnt++;
	pthread_mutex_lock(&mtx_task);
        task.cmd.type=ret;
	free(task.cmd.arg);
	task.cmd.arg=strdup(buf);
	task.cmd.isremote=0;
	pthread_mutex_unlock(&mtx_task);}
        return ret;
}

static void wait_command_user()
{
	/*
	input from console
	output to CMD field
	*/

	int cnt=1;
	char arg0[BUFSHORT];//first word in the line
	char buf[BUFSIZE];//line console input
	int type=EMPTY;
	int isremote=0;
	while (1){
	usleep(10000);
	/*console prompt*/
	if (task.cmd.type!=EMPTY) continue;
	pthread_mutex_lock(&mtx_stdin);
	printf("%s%d>:","master", cnt);
	buf[0]=0;arg0[0]=0;
	fgets(buf, BUFSIZE, stdin);
	pthread_mutex_unlock(&mtx_stdin);

	// task fill I
	type=EMPTY;
	isremote=0;

	//decode (special)
	sscanf(buf,"%s",arg0);
	if (strcmp(arg0,"+++")==0){
	pthread_mutex_lock(&mtx_task);
	task.mode=NMASTER;
	pthread_mutex_unlock(&mtx_task);
	printf("go to master mode\n");
	continue;}
	else if (strcmp(arg0,"---")==0){
	pthread_mutex_lock(&mtx_task);
	task.mode=NSLAVE;
	pthread_mutex_unlock(&mtx_task);
	printf("go to slave mode\n");
	continue;}
	else if (strcmp(arg0,"quit")==0){
	printf("quit...\n");
	amodem_end();
	exit(0);
	}

	//decode (normal)
	if (task.mode!=NMASTER){
	printf("This node is not in master mode, unable to give command\n");
	continue;}

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
	pthread_mutex_lock(&mtx_task);
	cnt++;
	task.cmd.type=type;
	task.cmd.isremote=isremote;
	free(task.cmd.arg);
	task.cmd.arg=strdup(buf);
	pthread_mutex_unlock(&mtx_task);
	}
}
}

int atalk(){
char buf[BUFSIZE];
switch (task.mode){
case NMASTER:
/*play*/
amodem_play("t1.wav");
/*wait ack*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*record*/
amodem_record(1000);
/*recv stamp*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
break;
case NSLAVE:
/*record*/
amodem_record(1000);
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

int xcross(){
char fname[40];
char output[40];
sscanf(task.cmd.arg,"%*s %s",fname);
strcpy(output,fname);
strcpy(strstr(output,".wav"),".out");
printf("proc %s, output %s ...\n",fname,output);

wav2CIR(fname,"T1_raw.wav",output);
return SUCCESS;
}

logger *t_log;

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
	pthread_mutex_lock(&mtx_task);
	task.mode=NMASTER;
	task.cmd.arg=strdup(" ");
	task.cmd.type=EMPTY;
	task.cmd.isremote=0;
	pthread_mutex_unlock(&mtx_task);

	//console thread
        pthread_attr_init(&attr);
        pthread_create(&t_read,&attr,(void *)wait_command_user,NULL);

	while (1)
	{
		usleep(100000);
		//wait command
		if (task.cmd.type==EMPTY) continue;
		//log
		sprintf(buf_log,"task %d",task.cmd.type);
		log_event(t_log,0,buf_log);

		// notify remote
		if (task.cmd.isremote){
		sprintf(remote,"REQ%d",task.cmd.type);
		amodem_puts_remote(remote);}

		pthread_mutex_lock(&mtx_stdin);
		switch (task.cmd.type)
		{
		case TALK://ok
		master_talk();
		break;
		case ATALK://ok
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
		case MSPLAY://ok
		play(task.cmd.arg);
		break;
		case MSRECORD://ok
		record(task.cmd.arg);
		break;
		case SYNCALL://ok
		master_sync();
		break;
		case HELP:
		help();
		break;
		case UPLOAD:
		upload(task.cmd.arg);
		break;
		case SEND_REMOTE://ok
		amodem_puts_remote(task.cmd.arg);
		break;
		case MSG_SHOW://ok
		amodem_msg_show(&msg_local);
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
			fprintf(stderr, "ERROR: please input readable command (small letter)\n");
			break;
		}
		pthread_mutex_unlock(&mtx_stdin);

		//update task
		pthread_mutex_lock(&mtx_task);
		task.cmd.type=EMPTY;
		free(task.cmd.arg);
		task.cmd.arg=strdup(" ");
		pthread_mutex_unlock(&mtx_task);
	}

amodem_close();
return 0;
}
