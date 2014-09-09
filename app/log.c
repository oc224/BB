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
return NULL;}

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
printf("path :%s\n",t_logger->path);
}
int log_event(logger* t_logger,unsigned int level,const char* msg){
return 0;
//if really need to log
printf("debug1\n");
if (t_logger->level>level) return SUCCESS;
//update time stamp*/
printf("debug1\n");
time(&t_logger->stamp);
//puts event msg and flush
printf("debug1\n");
if (fprintf(t_logger->fp,"%s %s\n",ctime(&t_logger->stamp),msg)<0)
return FAIL;
printf("debug1\n");
fflush(t_logger->fp);

return SUCCESS;
}

