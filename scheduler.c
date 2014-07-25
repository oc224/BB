#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include <string.h>
#include "acoustic_modem.h"
#define BUFSIZE 128
static schedule modem_schedule;
extern this_node t_node;
int scheduler_init(){
	modem_schedule.n_task=0;
	return SUCCESS;
}
void scheduler_task_show(){
	int i;
	task *prev;
	task *next;
	next=modem_schedule.p_head;
	printf("---------------\n");
	printf("%d task\n",modem_schedule.n_task);
	printf("index type duration arg\n");
	for (i=0;i<modem_schedule.n_task;i++){
		printf("%d %d %d %s\n",next->index,next->this_task,next->duration,next->arg);
		prev=next;
		next=prev->next_task;
	}
	printf("---------------\n");
}

int scheduler_task_add(char *cfg_msg){
	char type_task[16];
	if (modem_schedule.n_task==0){//first task
		modem_schedule.p_this=(task*)malloc(sizeof(task));
		modem_schedule.p_head=modem_schedule.p_this;
		modem_schedule.p_head->next_task=modem_schedule.p_this;
		modem_schedule.p_head->prev_task=modem_schedule.p_this;
	} else{
		modem_schedule.p_this->next_task=(task*)malloc(sizeof(task));
		modem_schedule.p_this->next_task->prev_task=modem_schedule.p_this;
		modem_schedule.p_this=modem_schedule.p_this->next_task;
		modem_schedule.p_this->next_task=modem_schedule.p_head;
		modem_schedule.p_head->prev_task=modem_schedule.p_this;
	}
	modem_schedule.n_task++;
	sscanf(cfg_msg,"%s %d %s",type_task,modem_schedule.p_this->duration,modem_schedule.p_this->arg);
	if (strcasestr("play",type_task)!=NULL){
		modem_schedule.p_this->this_task=PLAY;
	}else if (strcasestr("record",type_task)!=NULL) {
		modem_schedule.p_this->this_task=RECORD;
	}else if (strcasestr("sleep",type_task)!=NULL){
		modem_schedule.p_this->this_task=SLEEP;
	}
	return SUCCESS;
}
scheduler_read(char *filename){
	FILE *fp;
	char buf[BUFSIZE];
	char type_task[16];
	int is_match;
	fp=fopen(filename,"r");
	if (fp==NULL){
		printf("fail to read schedule file\n");
		return FAIL;
	}
	// locate identity
	do{
		if (fgets(buf,BUFSIZE,fp)==NULL){
			printf("fail to find the corresponding script\n");
			return FAIL;
		}
		is_match=strcasestr(t_node.name,buf);
	}while(is_match==NULL);
	//actually read
	fgets(buf,BUFSIZE,fp);
	if (strstr("}",buf)==NULL){//continue to read
		scheduler_task_add(buf);
	}else{//end
		printf("read script finish\n");
		scheduler_task_show();
		return SUCCESS;
	}
return FAIL;
}
void scheduler_exce(){
	// do this task
	switch (modem_schedule.p_this->this_task){
	case PLAY:
		a_modem_play(modem_schedule.p_this->arg);
		break;
	case RECORD:
		a_modem_record(modem_schedule.p_this->duration);
		break;
	case SLEEP:
		break;
	default:
		break;
	}
	// book next task

}
scheduler_open();
scheduler_close();
