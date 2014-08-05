#include "system.h"
#include "acoustic_modem.h"
#include <stdio.h>
#include <string.h>
#define BUFSIZE 128

typedef enum{
	TALK,INIT,PROB
}command;

int wait_command_user(){
	int ret;
	static int cnt=1;
	char buf[BUFSIZE];

	/*console prompt*/
	fprintf(stdout,"master<%d>:",cnt);
	fgets(buf,BUFSIZE,stdin);

	/*decode*/
	if (strstr(buf,"talk"))ret=TALK;
	else if (strstr(buf,"init"))ret=INIT;
	else if (strstr(buf,"prob"))ret=PROB;
	else ret=-1;
	
	/*return*/
	if (ret>=0)cnt++;
	return ret;

}


int main(){
command t_cmd;
while (1){
t_cmd=wait_command_user();
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
