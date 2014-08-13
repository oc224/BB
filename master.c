#include "system.h"
#include "acoustic_modem.h"
#include "ms.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 128
#define TIMEOUT 2000
#define M2S_TXPATH "/home/root/log/M2S_TXLOG.txt"
#define M2S_RXPATH "/home/root/log/M2S_RXLOG.txt"
#define S2M_TXPATH "/home/root/log/S2M_TXLOG.txt"
#define S2M_RXPATH "/home/root/log/S2M_RXLOG.txt"
int wait_command_user()
{
	/*return command (enum or integer)*/
	int ret;
	static int cnt=1;
	char buf[BUFSIZE];

	/*console prompt*/
	printf("master<%d>:", cnt);
	fgets(buf, BUFSIZE, stdin);

	/*decode*/
	if (strstr(buf,"talk"))
		ret=TALK;
	else if (strstr(buf,"synctime"))
		ret=SYNC_TIME;
	else if (strstr(buf,"init"))
		ret=INIT;
	else if (strstr(buf,"prob"))
		ret=PROB;
	else if (strstr(buf,"robin"))
		ret=ROBIN;
	else
		ret=-1;

	/*return*/
	if (ret>=0)
		cnt++;
	return ret;
}

int main()
{
	char buf[BUFSIZE], buf2[BUFSIZE], buf3[BUFSIZE];
	int i;
	command t_cmd;

	system_cfg_read();
	system_cfg_show();
	printf("NODE NAME : %s\n\n ",t_node.name);
	a_modem_init();
	a_modem_open();
	while (1)
	{
		printf("[0] TALK	[1] INIT	[2] PROB\n");
		/*so you mean user can also enter the number rather than text?*/
		t_cmd = wait_command_user();
		if (t_cmd == -1)
			fprintf(stderr, "ERROR: please input readable command (small letter)\n");
		else{
			printf("command = %d\n", t_cmd);
			sprintf(buf,"%d",t_cmd);
			a_modem_msg_send(buf);}

		switch (t_cmd)
		{
		case TALK: //DO TALK
			master_talk();
			break;
		case ROBIN:
			master_talk_round_robin();
			break;
		case SYNC_TIME:
			master_sync();
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
a_modem_close();
return 0;
}
