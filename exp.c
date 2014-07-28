/*
 * test_exp.c
 *
 *  Created on: Jul 25, 2014
 *      Author: root
 */
#include "scheduler.h"
#include "system.h"
#include "acoustic_modem.h"
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
//handle ctrl +c intterupt
int main(int argc,char* argv[]){

	int hh,mm,ss;
	char *fname;
	if (argc>2){
		fname=strdup(argv[2]);
	}else{
		fname=DEFAULT_SCRIPT;
	}
	struct tm start_time;
	time_t now;
	time(&now);
	if (argc>1){
		sscanf(argv[1],"%d:%d:%d",&hh,&mm,&ss);
		start_time.tm_hour=hh;
		start_time.tm_min=mm;
		start_time.tm_sec=ss;
	}else{
		start_time=*localtime(&now);
		start_time.tm_sec+=5;
	}

	printf("script file : %s\n",fname);
	printf("start time : %2d:%2d:%2d\n ",start_time.tm_hour,start_time.tm_min,start_time.tm_sec);
	a_modem_open();
	system_cfg_read();
	system_cfg_show();
	scheduler_init();
	scheduler_read(fname);
	scheduler_start(start_time.tm_hour,start_time.tm_min,start_time.tm_sec);


	while(1){
		sleep(1);
	}
	a_modem_close();
	return 0;
}
