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
	struct tm start_time;
	time_t now;
	time(&now);
	if (argc>2){
		sscanf(argv[1],"%d:%d:%d",&hh,&mm,&ss);
		start_time.tm_hour=hh;
		start_time.tm_min=mm;
		start_time.tm_sec=ss;
	}else{
		start_time=*localtime(&now);
		start_time.tm_sec+=5;
	}
	if (argc>2){
		fname=strdup(argv[2]);
	}else{
		fname=DEFAULT_SCRIPT;
	}


	printf("script file : %s\n",fname);
	printf("start time : %2d:%2d:%2d\n ",start_time.tm_hour,start_time.tm_min,start_time.tm_sec);
	if (system_cfg_read()==FAIL)	return FAIL;
	system_cfg_show();
	amodem_open();
	if (scheduler_init()==FAIL) return FAIL;
	if (scheduler_read(fname)==FAIL) return FAIL;
	scheduler_start(start_time.tm_hour,start_time.tm_min,start_time.tm_sec,'a');


	while(1){
		sleep(1);
	}
	amodem_close();
	return 0;
}
