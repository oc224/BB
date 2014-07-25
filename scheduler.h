/*
 * scheduler.h
 *
 *  Created on: 2014年7月24日
 *      Author: martin
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#define ARG_SIZE 32
typedef struct{
	int n_task;
	task *p_head;
	task *p_this;
} schedule;
typedef struct{
	int index;
	type_task this_task;
	int duration;
	char arg[ARG_SIZE];
	task *next_task;
	task *prev_task;
} task;
typedef enum{
	PLAY,RECORD,SLEEP
}type_task;


#endif /* SCHEDULER_H_ */
