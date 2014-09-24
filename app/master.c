#include "log.h"
#include "amodem.h"
#include "common.h"
#include "ms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BUFSIZE 128
#define BUFSHORT 20
#define MASTER_LOGPATH "/root/log/master_log.txt"

typedef enum {NMASTER,NSLAVE} NODE_MODE;
typedef char bool;


typedef enum{
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
        WAIT_REMOTE,//wait remote msg
        CLEAR_FFS,//clear local ffs
        STATUS,
        GPSLOG,
        RREBOOT
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
        if (ret!=FAIL)
                cnt++;
        task.cmd.type=ret;
	free(task.cmd.arg);
	task.cmd.arg=strdup(buf);
	task.cmd.isremote=0;
        return ret;
}

int wait_command_user()
{
	/*
	input from console
	output to CMD field
	*/

	static int cnt=1;
	char arg0[BUFSHORT];//first word in the line
	char buf[BUFSIZE];//line console input
	buf[0]=0;

	/*console prompt*/
	printf("%s%d>:","master", cnt);
	fgets(buf, BUFSIZE, stdin);

	// task fill I
	task.cmd.type=NONE;
	task.cmd.isremote=0;
	free(task.cmd.arg);
	task.cmd.arg=strdup(buf);

	/*decode*/
	sscanf(buf,"%s",arg0);
	//printf("debug %s\n",arg0);
	if (strcmp(arg0,"talk")==0){
		task.cmd.type=TALK;
		task.cmd.isremote=1;
	}else if (strcmp(arg0,"atalk")==0){
		task.cmd.type=ATALK;
		task.cmd.isremote=1;
	}else if (strcmp(arg0,"con")==0){
		task.cmd.type=CONVERSATION;
		task.cmd.isremote=1;
	}else if (strcmp(arg0,"conend")==0){
		task.cmd.type=CONEND;
		task.cmd.isremote=1;
	}else if(strcmp(arg0,"play")==0){
		task.cmd.type=MSPLAY;
		task.cmd.isremote=0;
	}else if (strcmp(arg0,"record")==0){
		task.cmd.type=MSRECORD;
		task.cmd.isremote=0;
	}else if (strcmp("sync",arg0)==0){
		task.cmd.type=SYNCALL;
		task.cmd.isremote=1;
	}else if (strcmp("upload",arg0)==0){
		task.cmd.type=UPLOAD;
		task.cmd.isremote=0;
	}else if (strcmp("quick",arg0)==0){
		task.cmd.type=QUICK;
		task.cmd.isremote=1;
	}else if (strcmp("status",arg0)==0){
		task.cmd.type=STATUS;
		task.cmd.isremote=1;
	}else if (strcmp("sr",arg0)==0){
		task.cmd.type=SEND_REMOTE;
		task.cmd.isremote=0;
	}else if (strcmp("wr",arg0)==0){
		task.cmd.type=WAIT_REMOTE;
		task.cmd.isremote=0;
	}else if (strcmp("showmsg",arg0)==0){
		task.cmd.type=MSG_SHOW;
		task.cmd.isremote=0;
	}else if (strcmp("clearffs",arg0)==0){
		task.cmd.type=CLEAR_FFS;
		task.cmd.isremote=0;
	}else if (strcmp("help",arg0)==0){
		task.cmd.type=HELP;
		task.cmd.isremote=0;
	}else if (strcmp("gpslog",arg0)==0){
		task.cmd.type=GPSLOG;
		task.cmd.isremote=0;
	}else if (strcmp("rreboot",arg0)==0){
		task.cmd.type=RREBOOT;
		task.cmd.isremote=1;
	}else{
		task.cmd.type=NONE;
		task.cmd.isremote=0;
	}
	/*return*/
	if (task.cmd.type>0)
		cnt++;
	return task.cmd.type;
}

int wait_command(){
printf("msg_remote.N_unread=%d\n",msg_remote.N_unread);
if (msg_remote.N_unread>0){
task.mode=NSLAVE;
}

switch (task.mode){
case NMASTER:
return wait_command_user();
break;
case NSLAVE:
return wait_command_master();
break;
default:
sleep(1);
return FAIL;
break;
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

logger *t_log;

void u_interrupt(){
unsigned char user;
printf("M,S,Q ? \n");
user=fgetc(stdin);
switch (user){
case 'M':
task.mode=NMASTER;
break;
case 'S':
task.mode=NSLAVE;
break;
case 'Q':
exit(0);
break;

}
//printf("user = %c\n",user);

}
int main()
{
	char remote[20];
	char buf_log[BUFSIZE];
	/*init cfg...*/
	system_cfg_read();
//	system_cfg_show();
//	printf("NODE NAME : %s\n ",t_node.name);
	amodem_init();
//	amodem_open();
//	scheduler_init();
//	scheduler_read("/root/config/schedule.txt");
//	scheduler_task_show();
	t_log=log_open(MASTER_LOGPATH);
	log_show(t_log);
	log_event(t_log,0,"master program start");

	//task init
	task.mode=NMASTER;
	task.cmd.arg=strdup(" ");
	task.cmd.type=NONE;
	task.cmd.isremote=0;

	//signal
	signal(SIGINT,u_interrupt);
	while (1)
	{
		//wait command
		if (wait_command()==FAIL)continue;

		//log
		printf("command %d\n",task.cmd.type);
		sprintf(buf_log,"task %d",task.cmd.type);
		log_event(t_log,0,buf_log);

		// notify remote
		if (task.cmd.isremote){
		sprintf(remote,"REQ%d",task.cmd.type);
		amodem_puts_remote(remote);}

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
		break;
		case MSG_SHOW://ok
		amodem_msg_show(&msg_local);
		amodem_msg_show(&msg_remote);
		break;
		case WAIT_REMOTE://ok
		wait_remote(task.cmd.arg);
		break;
		case CLEAR_FFS:
		amodem_ffs_clear();
		break;
		case STATUS:
		break;
		case GPSLOG:
		system("gpspipe -r -n 12 | grep GPGGA >> /home/root/log/gpslog.txt");
		break;
		case RREBOOT:
		master_rreboot();
		break;
		case NONE:
		amodem_puts(task.cmd.arg);
		amodem_print(1000);
		break;
		default:
			fprintf(stderr, "ERROR: please input readable command (small letter)\n");
			break;
		}
	}

amodem_close();
return 0;
}
