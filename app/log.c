#include "common.h"
#include "log.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_LEVEL 2

logger* log_open(const char *path){
//malloc for logger
logger *t_logger=(logger*)malloc(sizeof(logger));
if (t_logger==NULL){
printf("open error\n");
return NULL;}
//open log file
t_logger->fp=fopen(path,"a");
if (t_logger->fp==NULL){
printf("open error\n");
exit(FAIL);}

//fill logger struct
t_logger->level=DEFAULT_LEVEL;
t_logger->path=strdup(path);
return t_logger;
}

int log_close(logger* t_logger){
fclose(t_logger->fp);
free(t_logger);
return SUCCESS;
}

void log_show(logger* t_logger){
printf("Logger :\n");
printf("level : %d \n",t_logger->level);
printf("path : %s\n",t_logger->path);
}

int log_event(logger* t_logger,unsigned int level,const char* msg){
//if really need to log
if (level>t_logger->level) return SUCCESS;
//update time stamp*/
time(&t_logger->stamp);
//puts event msg and flush
if (fprintf(t_logger->fp,"%s %s\n",ctime(&t_logger->stamp),msg)<0)
return FAIL;
fflush(t_logger->fp);
return SUCCESS;
}

