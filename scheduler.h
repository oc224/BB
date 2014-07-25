/*
 * scheduler.h
 *
 *  Created on: 2014年7月24日
 *      Author: martin
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_
typedef enum{
	PLAY,RECORD,SLEEP
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


#endif /* SCHEDULER_H_ */
