#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "common.h"
#include "amodem.h"
#include "scheduler.h"
#define BUFSIZE 128

static SD *pSD ;

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
static timer_t timerid;
static struct itimerspec its;

int scheduler_init() {

	struct sigevent sev;
	//sigset_t mask;
	struct sigaction sa;

	/* Establish handler for timer signal */

//	printf("Establishing handler for signal %d\n", SIG);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = scheduler_exec;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIG, &sa, NULL) == -1)
		errExit("sigaction");
	/* Block timer signal temporarily */

/*	printf("Blocking signal %d\n", SIG);
	sigemptyset(&mask);
	sigaddset(&mask, SIG);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
		errExit("sigprocmask");*/

	/* Create the timer */

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCKID, &sev, &timerid) == -1)
		errExit("timer_create");

	//printf("timer ID is 0x%lx\n", (long) timerid);

//	sd->n_task = 0;
	return SUCCESS;
}

int scheduler_set(SD *sd,int mode,int N,int hh,int mm,int ss){
switch (mode){
case 1:
sd -> init_time = 'r';
sd -> hh = hh;
sd -> mm = mm;
sd -> ss = ss;
sd -> N = N;
sd -> n_task = 0;
sd -> isBlock = 1;
sd -> p_head = NULL;
sd -> p_this = NULL;
break;
default :
ERR_PRINT("mode invalid\n");
return FAIL;
break;
}

return SUCCESS;
}

void scheduler_show(SD *sd) {
	SDtask *pShow;
	printf("-------------------------\n");
	printf("INIT TIME TYPE : %c\n",sd->init_time);
	printf("Number of Cycle : %d\n",sd->N);
	printf("START TIME : %d:%d:%d\n",sd->hh,sd->mm,sd->ss);
	printf("IS BLOCK : %d\n",sd->isBlock);
	printf("%d tasks\n", sd->n_task);
	printf("index type duration arg\n");
	
	pShow = sd->p_head;
	while (pShow != NULL){
		printf("%d %d %d %s\n", pShow->index, pShow->this_task, pShow->duration,pShow->arg);
		pShow = pShow -> next_task;
	}

//	for (pShow = sd->p_head; pShow->next_task != NULL; pShow = pShow->next_task) {
//	}
	printf("-------------------------\n\n");
}

int scheduler_task_add(SD *sd,char *cfg_msg) {
	char type_task[16];
	char arg[128];
	memset(arg, 0, 128);
	//printf("debug task add : %s\n",cfg_msg);

	//new item setup
	SDtask *new_task = (SDtask*) malloc(sizeof(SDtask));
	SDtask *task_tail;
	sd->n_task++;
	sscanf(cfg_msg, "%s %d %s", type_task, &new_task->duration,
			arg);
	if (strcmp(type_task, "play") ==0) {
		new_task->this_task = SD_PLAY;
	} else if (strcmp(type_task, "record") == 0) {
		new_task->this_task = SD_RECORD;
	} else if (strcmp(type_task, "sleep") == 0) {
		new_task->this_task = SD_SLEEP;
	} else if (strcmp(type_task, "sync") == 0) {
		new_task->this_task = SD_SYNC;
	}
	new_task->arg = strdup(arg);
	//printf("debug ,arg:%s\n",arg);
	new_task->index = sd->n_task;
	new_task->next_task = NULL;

	//printf("debug task add : %s\n",cfg_msg);

	if (sd->n_task == 1) {	//first task
	//printf("first task\n");
	sd->p_head = new_task;
	} else {
	//printf("non first task\n");
	task_tail = sd->p_head;
	while ( task_tail->next_task != NULL ) task_tail = task_tail->next_task;
	task_tail->next_task = new_task;
	}
	//printf("debug task add : %s\n",cfg_msg);

	return SUCCESS;
}
/*
int scheduler_read(SD *sd,char *filename) {
	FILE *fp;
	char buf[BUFSIZE];
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("fail to read schedule file\n");
		return FAIL;
	}
	// locate identity
	do {
		if (fgets(buf, BUFSIZE, fp) == NULL) {
			printf("fail to find the corresponding script\n");
			return FAIL;
		}
//		 strcasestr(buf, t_node.name);	//TODO
	} while (strcasestr(buf, t_node.name)==NULL);
	//actually read
	while (fgets(buf, BUFSIZE, fp)!=NULL) {
		if (strstr(buf, "}") != NULL) {//end
			printf("read script finish\n");
			sd->p_this=sd->p_head;
			scheduler_task_show();
			return SUCCESS;
		}
		if (strlen(buf)>1)scheduler_task_add(buf);
	}
	return FAIL;
}
*/
void scheduler_exec(int sig, siginfo_t *si, void *uc) {
int carry;
/*	timer_t *tidp;
	           int or;

	           tidp = si->si_value.sival_ptr;

	           printf("    sival_ptr = %p; ", si->si_value.sival_ptr);
	           printf("    *sival_ptr = 0x%lx\n", (long) *tidp);

	           or = timer_getoverrun(*tidp);
	           if (or == -1)
	               errExit("timer_getoverrun");
	           else
	               printf("    overrun count = %d\n", or);*/
	//timer_t now;
	//time(&now);
	//system_msg_dump("start");
	//system_msg_dump(ctime(&now));


	// do this task
    //printf("task : %d\n",sd->p_this->index);
	switch (pSD->p_this->this_task) {
	case SD_PLAY:
		//printf("debug, amodem play\n");
		amodem_play(pSD->p_this->arg);
		break;
	case SD_RECORD:
		//printf("debug, amodem record\n");
		amodem_record(pSD->p_this->duration);
		break;
	case SD_SLEEP:
		//printf("debug, amodem sleep\n");
		break;
	case SD_SYNC:
		amodem_sync_clock_gps(4);
		amodem_sync_time_gps();
		break;
	default:
		break;
	}

	// book next task
	if (pSD->p_this->index == pSD->n_task) {
	pSD->p_this = pSD -> p_head;
	pSD->N--;//finish one cycle
	printf("one cycle done\n");}
	if (pSD->N==0) {
	printf("SD done\n");
	return;
	}
	carry=0;
//	its.it_interval.tv_nsec=0;
//	its.it_interval.tv_sec=0;
	its.it_value.tv_nsec+=(pSD->p_this->duration%1000)*1000000;
	if (its.it_value.tv_nsec>=1000000000){
		its.it_value.tv_nsec+=-1000000000;
		carry=1;
	}
	its.it_value.tv_sec+=pSD->p_this->duration/1000+carry;
	//printf("%ld %ld\n",its.it_value.tv_sec,its.it_value.tv_nsec);
    if (timer_settime(timerid, TIMER_ABSTIME, &its, NULL) == -1)
         errExit("timer_settime");
	pSD->p_this=pSD->p_this->next_task;
}

int scheduler_start(SD *sd) {
	/*input
	if type = a
	the first scheduler will be at hh:mm:ss.0
	if type = r
	the first schedule will be 3 seconds later, hh, mm, ss are ignored
	*/
	struct timespec now_clock;
	struct tm start_time;
	long seconds;
	time_t now;
	int OneRound = 0;
	SDtask *pTask;
	/*figures out Number of seconds before first interrupt*/
	time(&now);
	start_time=*localtime(&now);
	start_time.tm_hour=sd->hh;start_time.tm_min=sd->mm;start_time.tm_sec=sd->ss;
	time(&now);

	switch(sd->init_time){
	case 'a':
	seconds=difftime(mktime(&start_time),now);
	break;
	case 'r':
	seconds=sd->ss;
	break;
	default:
	fprintf(stderr,"scheduler arg invalid\n");
	return FAIL;
	break;
	}

	if (seconds<2){
		printf("error, target time has passed\n");
		return FAIL;
	}
	printf("%ld seconds left\n",seconds);

	/*set the first timer interrupt*/
	clock_gettime(CLOCKID,&now_clock);
//	printf(" sec,%ld\nnsec %ld\n",(long)now_clock.tv_sec,(long)now_clock.tv_nsec);
	its.it_interval.tv_nsec=0;
	its.it_interval.tv_sec=0;
	its.it_value.tv_sec=now_clock.tv_sec+seconds;

	switch (sd->init_time){
	case 'a':
	its.it_value.tv_nsec=0;
	break;
	case 'r':
	its.it_value.tv_nsec=now_clock.tv_nsec;
	break;
	default:
	fprintf(stderr,"scheduler arg invalid\n");
	return FAIL;
	break;
	}

        if (timer_settime(timerid, TIMER_ABSTIME, &its, NULL) == -1)
         errExit("timer_settime");

	// pSD
	sd -> p_this = sd->p_head;
	pSD = sd;

	printf("scheduler start\n");

	//block
	if (pSD -> isBlock){
	pTask = sd->p_head;
	while ( pTask != NULL ) {
	OneRound += pTask -> duration/1000;
	pTask = pTask->next_task;
	}
	OneRound = OneRound*pSD->N+seconds+2;
	printf("scheduler executing...%d sec\n",OneRound);
	do{
	OneRound = sleep(OneRound);
	}
	while (OneRound >0);
	printf("scheduler end\n");
	}

	return SUCCESS;
}

int scheduler_stop(SD *sd) {
	its.it_interval.tv_nsec=0;
	its.it_interval.tv_sec=0;
	its.it_value.tv_sec=0;
	its.it_value.tv_nsec=0;
    if (timer_settime(timerid, TIMER_ABSTIME, &its, NULL) == -1)
         errExit("timer_settime");
	printf("scheduler stop\n");
	return SUCCESS;
}
