#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "system.h"
#include "acoustic_modem.h"
#include "ms.h"
#define BUFSIZE 128
#define TIMEOUT 2000
cmd t_cmd;

int wait_command_master(){
	int ret;
	static int cnt=1;
	char buf[BUFSIZE];

	while(1) //wait until remote msg received
	{
		if (a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT)==SUCCESS)
			break;
	}
	printf("remote:%s \n",buf);

	/*decode*/
	if (sscanf(buf,"%d",&ret) < 1)
		ret = -1;

	/*return*/
	if (ret >= 0)
		cnt++;
	t_cmd.type=ret;
	return ret;
}


int main()
{
	/*init*/
	system_cfg_read();
	system_cfg_show();
	printf("\nNODE NAME : %s\n\n ",t_node.name);
	a_modem_init();
	a_modem_open();

	while (1)
	{
		wait_command_master();
		switch (t_cmd.type)
		{
		case TALK: //DO TALK
		printf("go to talk\n");
		slave_talk();
		break;
		case CONVERSATION:
		slave_con();
		break;
		case QUICK:
		slave_quick();
		break;
		case SYNCALL:
		printf("go to sync time\n");
		slave_sync();
		break;
		default:
			break;
		}
	}
a_modem_close();
return 0;
}
