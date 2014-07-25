/*
 * test_exp.c
 *
 *  Created on: Jul 25, 2014
 *      Author: root
 */
#include "scheduler.h"
#include "system.h"
int main(){
	system_cfg_read();
	system_cfg_show();
	scheduler_init();
	scheduler_read("./config/schedule.txt");


	return 0;
}
