#include "log.h"
#include "amodem.h"
#include "common.h"
#include "ms.h"
#include "master.h"
#include "signal.h"
#include "scheduler.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define BUFSIZE 128
#define BUFSHORT 20
#define MASTER_LOGPATH "/root/log/master_log.txt"
#define GUARD_HEAD 3
#define GUARD_TAIL 3
#define ADDR_BROADCAST 255

logger *t_log;
TASK task_recv_master,task_recv_user,task_exec;

struct SETTING{
float guard_time_head;
float guard_time_tail;
int no_mseq;
int local_addr;
char hostname[20];
}set;


//NODE
typedef struct{
char name[20];
char amodem_addr[4];
int id_mseq;
int id_run;
int this_node;
float snr;
float travel_time;
}NODE;
NODE** node,*node_target;

//read node list from a file
NODE** NODE_read(const char *);
//show node list
void NODE_show(NODE **);
//return node by name
NODE* NODE_lookup(NODE **,const char*);


void command_wait();
int command_exec();
void wait_command_user();
int xcorr();
int ctalk();
int data_anal(DATA_COOK *,const char*);
SD sd;


int main(int argc,char *argv[])
{

//NODE
if ((node=NODE_read(" "))==NULL){
fprintf(stderr,"fail to read node list\n");
return FAIL;}
NODE_show(node);
node_target=NODE_lookup(node,"master");
printf("debug,%s %d\n",node_target->name,node_target->id_mseq);

//SET
gethostname(set.hostname,19);


//log init
t_log=log_open(MASTER_LOGPATH);
log_show(t_log);
log_event(t_log,0,"master program start");
if (argc<2) return FAIL;
printf("amodem on %s\n",argv[1]);

//amodem init
amodem_init(argv[1]);

//scheduler
scheduler_init();


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
int ret;
DATA_COOK dc;
//log
sprintf(buf_log,"task %d",task_exec.type);
log_event(t_log,0,buf_log);

// notify remote
if (task_exec.isremote){
sprintf(remote,"REQ%d",task_exec.type);
amodem_puts_remote(ADDR_BROADCAST,remote);}

//do task_exec
switch (task_exec.type)
{
case TALK:
//master_talk();
break;
case ATALK:
ret=ctalk();
break;
case PROB:
//prob();
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
ret=play(task_exec.arg);
break;
case MSRECORD:
ret=record(task_exec.arg);
break;
case SYNCTIME:
//master_sync();
break;
case HELP:
help();
break;
case UPLOAD:
ret=upload(task_exec.arg);
break;
case ANAL:
ret=data_anal(&dc,"");
break;
case RECANAL:
ret=recanal(task_exec.arg);
break;
case SEND_REMOTE:
amodem_puts_remote(ADDR_BROADCAST,task_exec.arg);
break;
case MSG_SHOW://ok
printf("local msg\n");
amodem_msg_show(&msg_local);
printf("remote msg\n");
amodem_msg_show(&msg_remote);
break;
case STATUS:
break;
case RREBOOT:
//master_rreboot();
break;
case XCORR:
ret=xcorr();
break;
case NONE:
amodem_puts_local(task_exec.arg);
amodem_print(700);
break;
default:
//fprintf(stderr, "ERROR: please input readable command (small letter)\n");
break;
}

return ret;
}

int data_anal(DATA_COOK *dc,const char *txname){
//upload data xcorr...
// input :	txname path to xxx.wav
//		keyin tx wav (without .wav )
// output : dc structure
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
//sscanf(task_exec.arg,"%*s %s",fname);
sprintf(path_in,"%s/%s",PATH_RAW_DATA,fname);
strcpy(path_out,path_in);
strcpy(strstr(path_out,".wav"),".out");
printf("proc %s, output %s ...\n",fname,path_out);

//enter tx wav
if (txname==NULL){
fprintf(stdout,"tx wav:>");
fgets(fname,40,stdin);
fname[strlen(fname)-1]=0;
sprintf(buf,"%s%s.wav",PATH_TX_SIGNAL,fname);}
else{
sprintf(buf,"%s%s.wav",PATH_TX_SIGNAL,txname);
}

wav2CIR(path_in,buf,path_out,dc);
DATA_COOK_show(dc);
return SUCCESS;
}

int file_exist (char *filename)
{
  struct stat   buffer;
  return (stat (filename, &buffer) == 0);
}

int xcorr(){
char buf[BUFSIZE];
char fname[40];
char path_out[120],path_in[120];
DATA_COOK dc = {.snr = 0.0, .avg = 0.0, .max = 0.0, .offset = 0.0, .i_offset = 0, .hh = 0, .mm = 0};
//input fname
fname[0]=0;
sscanf(task_exec.arg,"%*s %s",fname);
sprintf(path_in,"%s/%s.wav",PATH_RAW_DATA,fname);
if (!file_exist(path_in)) {
ERR_PRINT("invalid rx data\n");
return FAIL;}
strcpy(path_out,path_in);
strcpy(strstr(path_out,".wav"),".out");
printf("proc %s, output %s ...\n",fname,path_out);

//enter tx wav
fprintf(stdout,"tx wav:>");
fgets(fname,40,stdin);
fname[strlen(fname)-1]=0;
sprintf(buf,"/root/tx/%s.wav",fname);
if (!file_exist(buf)) {
ERR_PRINT("invalid tx data\n");
return FAIL;}

wav2CIR(path_in,buf,path_out,&dc);
DATA_COOK_show(&dc);
return SUCCESS;
}

int prob_2nodes(){
#ifdef CON_MASTER
char buf[30];
sprintf(buf,"atr%d\r",PROB);
amodem_puts_local(buf);
return SUCCESS;
#endif
#ifdef CON_MASTER
return SUCCESS;
#endif

}
int prob(){
//want to prob N nodes setup
#ifdef CON_MASTER
char buf[BUFSIZE];
int i;
//call nodes
for (i=0;node[i]!=NULL;i++)
if(node[i]->this_node==0){
//atr
sprintf(buf,"atr%s\r",node[i]->amodem_addr);
amodem_puts_local(buf);
//wait report
//atx
sprintf(buf,"atx%s\r",node[i]->amodem_addr);
amodem_puts_local(buf);
//wait report
//store
//node[i]->snr =
//node[i]->travel_time =
}

//cal S1 to prob

return SUCCESS;
#endif

#ifdef CON_SLAVE
//sleep
//wait until signal
int i;
char buf[BUFSIZE];

//call nodes
for (i=0;node[i]!=NULL;i++)
if(node[i]->this_node==0){
//atr
sprintf(buf,"atr%s\r",node[i]->amodem_addr);
amodem_puts_local(buf);
//wait report
//atx
sprintf(buf,"atx%s\r",node[i]->amodem_addr);
amodem_puts_local(buf);
//wait report
//store
//node[i]->snr 
//node[i]->travel_time = 
}

//report to master
amodem_mode_select('o',3);

sprintf(buf,"%s report\n",set.hostname);
amodem_puts_local(buf);

for (i=0;node[i]!=NULL;i++)
if(node[i]->this_node==0){
sprintf(buf,"%s, travel time =  %f, snr = %f\n",node[i]->name,node[i]->travel_time,
node[i]->snr);
amodem_puts_local(buf);
}

amodem_mode_select('c',3);
return SUCCESS;
#endif
}

//template
/*int prob(){
#ifdef CON_MASTER

#endif

#ifdef CON_SLAVE

#endif
}*/

int ctalk(){
DATA_COOK dc = {.snr = 0.0, .avg = 0.0, .max = 0.0, .offset = 0.0, .i_offset = 0, .hh = 0, .mm = 0};
char buf[BUFSIZE];

#ifdef CON_MASTER
int N;
int i;
char arg[10];
//input arg N times
sscanf(task_exec.arg,"%*s %s",arg);
if (strlen(arg)!=0) N=atoi(arg);
else N=1;
printf("ctalk, %d times\n",N);

//SD
scheduler_set(&sd,1,1,0,0,12.0);
scheduler_task_add(&sd,"record 2000");
scheduler_task_add(&sd,"sleep 8500");
scheduler_task_add(&sd,"play 1500 mseq10_T1_l1");
scheduler_task_add(&sd,"sleep 8000");
scheduler_show(&sd);
//amodem_puts_local("atr3\r");
scheduler_start(&sd);
data_anal(&dc,TX_DEFAULT);
DATA_COOK_show(&dc);
#endif

#ifdef CON_SLAVE

//SD
scheduler_set(&sd,1,1,0,0,11);
scheduler_task_add(&sd,"sleep 500");
scheduler_task_add(&sd,"play 1500 mseq10_T1_l1");
scheduler_task_add(&sd,"sleep 8000");
scheduler_task_add(&sd,"record 2000");
scheduler_task_add(&sd,"sleep 8000");
scheduler_show(&sd);
scheduler_start(&sd);

data_anal(&dc,TX_DEFAULT);
DATA_COOK_show(&dc);
sprintf(buf,"SNR = %4.1f, RX %d:%d:%f\n",dc.snr,dc.hh,dc.mm,dc.ss+dc.offset);
amodem_mode_select('o',3);
amodem_puts_local(buf);
sleep(2);
amodem_puts_local(modem.latest_tx_stamp);
sleep(2);
amodem_mode_select('c',3);
#endif
return 0;

}

void wait_command_user()
{
	/*
	input from console
	output to task_exec field
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
	}else if (strcmp(arg0,"prob")==0){
		type=PROB;
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
	}else if (strcmp("synctime",arg0)==0){
		type=SYNCTIME;
		isremote=1;
	}else if (strcmp("upload",arg0)==0){
		type=UPLOAD;
		isremote=0;
	}else if (strcmp("anal",arg0)==0){
		type=ANAL;
		isremote=0;
	}else if (strcmp("recanal",arg0)==0){
		type=RECANAL;
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
	task_push(&task_recv_user,type,isremote,buf);
	}


}

void command_wait(){
//make sure command get
#ifdef CON_MASTER
//from user
do {
wait_command_user();
}while(task_pop(&task_recv_user,&task_exec)==FAIL);
#endif
#ifdef CON_SLAVE
//from master
while (task_pop(&task_recv_master,&task_exec)==FAIL) usleep(100000);
#endif
}

int task_push(TASK *task,cmd_type type,bool isremote,char *line){
//
if (task->type!=EMPTY) {
printf("%s,task full\n",__func__);
return FAIL;
}

task->type=type;
task->isremote=isremote;
strcpy(task->arg,line);
return SUCCESS;
}

int task_pop(TASK *task,TASK *task_get){
//
if (task->type==EMPTY) return FAIL;

memcpy(task_get,task,sizeof(TASK));
task->type=EMPTY;
return SUCCESS;
}

NODE** NODE_read(const char *pathname){
NODE ** list=NULL;
int N=3;
int i;
int err=-1;
char hostname[20];
gethostname(hostname,19);

list=(NODE **)malloc(sizeof(NODE *)*(N+1));


list[0]=(NODE *)malloc(sizeof(NODE));
strcpy(list[0]->name,"master");
strcpy(list[0]->amodem_addr,"210");
list[0]->id_mseq=1;
list[0]->id_run=1;
list[0]->this_node=0;

list[1]=(NODE *)malloc(sizeof(NODE));
strcpy(list[1]->name,"charlie");
strcpy(list[1]->amodem_addr,"213");
list[1]->id_mseq=2;
list[1]->id_run=2;
list[1]->this_node=0;

list[2]=(NODE *)malloc(sizeof(NODE));
strcpy(list[2]->name,"dylan");
strcpy(list[2]->amodem_addr,"214");
list[2]->id_mseq=3;
list[2]->id_run=3;
list[2]->this_node=0;

list[3]=NULL;

//find this node in the list
for (i=0;list[i]!=NULL;i++)
if (strcmp(hostname,list[i]->name)==0) {
err=0;
list[i]->this_node=1;}

if (err==-1) ERR_PRINT("ca't find the local node in the nodelist\n");
//printf("%s,can't find local node in the nodelist\n",__func__);

return list;
}

void NODE_show(NODE **node){
int i;
printf("%10s%10s%10s%10s%10s\n","name","addr","id_mseq","id_run","this");
printf("--------------------------------------------------\n");
for (i=0;node[i]!=NULL;i++)
printf("%10s%10s%10d%10d%10d\n",node[i]->name,node[i]->amodem_addr,node[i]->id_mseq,node[i]->id_run,node[i]->this_node);
}

NODE* NODE_lookup(NODE **list,const char* name){
int i;
//look sequentail
for (i=0;list[i]!=NULL;i++)
if (strcmp(list[i]->name,name)==0) return list[i];

//fail
printf("%s, node %s not found\n",__func__,name);
return NULL;
}
