
/*
 * slave.c
 *
 *  Created on: 2014年8月3日
 *      Author: martin
 */

#include <stdio.h>
#include <string.h>
#include "system.h"
#include "acoustic_modem.h"
#define BUFSIZE 128
#define WAIT_INT 2000

typedef enum{
	TALK,INIT,PROB
}command;

int wait_command_master(){
	int ret;
	static int cnt=1;
	char buf[BUFSIZE];
	

	while(1){//wait until command
	if (a_modem_wait_info("DATA",WAIT_INT,buf,BUFSIZE)==SUCCESS)break;
	}

	/*decode*/
	if (sscanf(buf,"%d",&ret)<1)ret=-1;
	
	/*return*/
	if (ret>=0)cnt++;
	return ret;

}


int main(){
command t_cmd;
while (1){
t_cmd=wait_command_master();
printf("cmd = %d\n",t_cmd);
switch (t_cmd){
case TALK:
//DO TALK
break;
case INIT:
//DO INIT
break;
case PROB:
//DO PROB
break;
default:
break;
}




}
return 0;
}
