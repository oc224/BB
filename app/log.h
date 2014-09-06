#include <stdio.h>
#include <time.h>
typedef struct{
unsigned int level;
char *path;
FILE *fp;
time_t stamp;
}logger;

int log_open(logger* t_logger,const char*);
int log_close(logger*);
int log_event(logger*,unsigned int,const char*);

