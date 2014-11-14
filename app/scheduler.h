/*
 * scheduler.h
 *
 *  Created on: 2014年7月24日
 *      Author: martin
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <signal.h>

typedef enum{
	SD_PLAY,SD_RECORD,SD_SLEEP,SD_SYNC
}SDtype_task;

typedef struct{
	int index;
	SDtype_task this_task;
	int duration;
	char *arg;
	void *next_task;
} SDtask;

typedef struct{
	char init_time;//
	int N;//exec N cycle.
	int isBlock;
	int hh;
	int mm;
	float ss;
	int n_task;
	SDtask *p_head;
	SDtask *p_this;
	int RoundTime;
} SD;

int scheduler_init();
void scheduler_show(SD *);
int scheduler_task_add(SD *,char *cfg_msg);
int scheduler_set(SD *sd,int mode,int N,int hh,int mm,float ss);
int scheduler_read(SD *,char *filename);
void scheduler_exec(int sig, siginfo_t *si, void *uc);
int scheduler_start(SD *);
int scheduler_stop(SD *);

#endif /* SCHEDULER_H_ */
