#include "system.h"
#include "acoustic_modem.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 128
#define TIMEOUT 2000

typedef enum{
	TALK, INIT, PROB
}command;

int wait_command_user()
{
	int ret;
	static int cnt=1;
	char buf[BUFSIZE];

	/*console prompt*/
	printf("master<%d>:", cnt);
	fgets(buf, BUFSIZE, stdin);

	/*decode*/
	if (strstr(buf,"talk"))
		ret=TALK;
	else if (strstr(buf,"init"))
		ret=INIT;
	else if (strstr(buf,"prob"))
		ret=PROB;
	else
		ret=-1;
	
	/*return*/
	if (ret>=0)
		cnt++;
	return ret;
}

int main()
{
	char buf[BUFSIZE];
	int i;
	command t_cmd;

	while (1)
	{
		printf("[0] TALK	[1] INIT	[2] PROB\n");
		t_cmd = wait_command_user();
		if (t_cmd == -1)
			fprintf(stderr, "ERROR: please input readable command (small letter)\n");
		else
			printf("command = %d\n", t_cmd);
		switch (t_cmd)
		{
		case TALK: //DO TALK
			a_modem_msg_send("DATAtalk");
			sleep(1);
			a_modem_play("lfm_data_t1_l10.wav"); // in this case, Master is Charlie
			a_modem_wait_info("TX", TIMEOUT, buf, BUFSIZE);
			printf("%s", buf);

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
