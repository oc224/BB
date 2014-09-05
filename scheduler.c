#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "system.h"
#include <string.h>
#include <time.h>
#include <signal.h>
#include "acoustic_modem.h"
#include "scheduler.h"
#define BUFSIZE 128

static schedule modem_schedule;
//this_node t_node;

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

	printf("Establishing handler for signal %d\n", SIG);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = scheduler_exce;
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

	modem_schedule.n_task = 0;
	return SUCCESS;
}

void scheduler_task_show() {
	int i;
	task *prev;
	task *next;
	next = modem_schedule.p_head;
	printf("---------------\n");
	printf("%d tasks\n", modem_schedule.n_task);
	printf("index type duration arg\n");
	for (i = 0; i < modem_schedule.n_task; i++) {
		printf("%d %d %d %s\n", next->index, next->this_task, next->duration,
				next->arg);
		prev = next;
		next = prev->next_task;
	}
	printf("---------------\n");
}

int scheduler_task_add(char *cfg_msg) {
	char type_task[16];
	char arg[128];
	memset(arg, 0, 128);
	//printf("debug task add : %s\n",cfg_msg);
	// pointer
	task *new_task = (task*) malloc(sizeof(task));
	if (modem_schedule.n_task == 0) {	//first task
		modem_schedule.p_this = new_task;
		modem_schedule.p_head = new_task;
		new_task->next_task = new_task;
		new_task->prev_task = new_task;
	} else {
		modem_schedule.p_this->next_task = new_task;
		new_task->prev_task = (task *) modem_schedule.p_this;
		new_task->next_task = modem_schedule.p_head;
		modem_schedule.p_head->prev_task = new_task;
		modem_schedule.p_this = new_task;
	}
	modem_schedule.n_task++;
	sscanf(cfg_msg, "%s %d %s", type_task, &modem_schedule.p_this->duration,
			arg);
	if (strcmp(type_task, "play") ==0) {
		modem_schedule.p_this->this_task = SD_PLAY;
	} else if (strcmp(type_task, "record") == 0) {
		modem_schedule.p_this->this_task = SD_RECORD;
	} else if (strcmp(type_task, "sleep") == 0) {
		modem_schedule.p_this->this_task = SD_SLEEP;
	} else if (strcmp(type_task, "sync") == 0) {
		modem_schedule.p_this->this_task = SD_SYNC;
	}
	modem_schedule.p_this->arg = strdup(arg);
	//printf("debug ,arg:%s\n",arg);
	modem_schedule.p_this->index = modem_schedule.n_task;
	return SUCCESS;
}

int scheduler_read(char *filename) {
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
			modem_schedule.p_this=modem_schedule.p_head;
			scheduler_task_show();
			return SUCCESS;
		}
		if (strlen(buf)>1)scheduler_task_add(buf);
	}
	return FAIL;
}

void scheduler_exce(int sig, siginfo_t *si, void *uc) {
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
    //printf("task : %d\n",modem_schedule.p_this->index);
	switch (modem_schedule.p_this->this_task) {
	case SD_PLAY:
		//printf("debug, amodem play\n");
		amodem_play(modem_schedule.p_this->arg);
		break;
	case SD_RECORD:
		//printf("debug, amodem record\n");
		amodem_record(modem_schedule.p_this->duration);
		break;
	case SD_SLEEP:
		//printf("debug, amodem sleep\n");
		break;
	case SD_SYNC:
		a_modem_sync_clock_gps(4);
		a_modem_sync_time_gps();
		break;
	default:
		break;
	}

	// book next task
	int carry=0;
//	its.it_interval.tv_nsec=0;
//	its.it_interval.tv_sec=0;
	its.it_value.tv_nsec+=(modem_schedule.p_this->duration%1000)*1000000;
	if (its.it_value.tv_nsec>=1000000000){
		its.it_value.tv_nsec+=-1000000000;
		carry=1;
	}
	its.it_value.tv_sec+=modem_schedule.p_this->duration/1000+carry;
	//printf("%ld %ld\n",its.it_value.tv_sec,its.it_value.tv_nsec);
    if (timer_settime(timerid, TIMER_ABSTIME, &its, NULL) == -1)
         errExit("timer_settime");
	modem_schedule.p_this=modem_schedule.p_this->next_task;
}

int scheduler_start(int hh,int mm, int ss,char type) {
	/*input
	type a -> TIMER_ABSTIMER
		r -> RELATIVE */
	struct timespec now_clock;
	struct tm start_time;
	long seconds;
	time_t now;
	/*figures out Number of seconds before first interrupt*/
	time(&now);
	start_time=*localtime(&now);
	start_time.tm_hour=hh;start_time.tm_min=mm;start_time.tm_sec=ss;
	time(&now);
	switch(type){
	case 'a':
	seconds=difftime(mktime(&start_time),now);
	break;
	case 'r':
	seconds=3;
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
	switch (type){
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

    printf("scheduler start\n");
	return SUCCESS;
}

int scheduler_stop() {
	its.it_interval.tv_nsec=0;
	its.it_interval.tv_sec=0;
	its.it_value.tv_sec=0;
	its.it_value.tv_nsec=0;
    if (timer_settime(timerid, TIMER_ABSTIME, &its, NULL) == -1)
         errExit("timer_settime");
printf("scheduler stop\n");
	return SUCCESS;
}
