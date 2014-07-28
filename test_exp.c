/*
 * test_exp.c
 *
 *  Created on: Jul 25, 2014
 *      Author: root
 */
#include "scheduler.h"
#include "system.h"
#include <unistd.h>
int main(){
	system_cfg_read();
	system_cfg_show();
	scheduler_init();
	scheduler_read("./config/schedule.txt");
	scheduler_start(12,30,0);


	while(1){
		sleep(1);
	}
	return 0;
}
