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
}type_task;

typedef struct{
	int index;
	type_task this_task;
	int duration;
	char *arg;
	void *next_task;
	void *prev_task;
} task;

typedef struct{
	int n_task;
	task *p_head;
	task *p_this;
} schedule;

int scheduler_init();
void scheduler_task_show();
int scheduler_task_add(char *cfg_msg);
int scheduler_read(char *filename);
void scheduler_exce(int sig, siginfo_t *si, void *uc);
int scheduler_start(int hh,int mm, int ss,char type);
int scheduler_stop();

#endif /* SCHEDULER_H_ */
