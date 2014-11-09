#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <time.h>
typedef struct{
unsigned int level;
char *path;
FILE *fp;
time_t stamp;
}logger;

logger* log_open(const char*);

int log_close(logger*);

int log_event(logger*,unsigned int,const char*);

void log_show(logger*);

#endif
