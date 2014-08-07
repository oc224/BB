#include "system.h"
#include "acoustic_modem.h"
#include "MS_sleep_time.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 128
#define TIMEOUT 2000
#define M2S_TXPATH "/home/root/log/M2S_TXLOG.txt"
#define M2S_RXPATH "/home/root/log/M2S_RXLOG.txt"
#define S2M_TXPATH "/home/root/log/S2M_TXLOG.txt"
#define S2M_RXPATH "/home/root/log/S2M_RXLOG.txt"

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
	char buf[BUFSIZE], buf2[BUFSIZE], buf3[BUFSIZE];
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
			a_modem_msg_send("talk");
			sleep(WAIT_then_PLAY);
			a_modem_play("lfm_data_t1_l10.wav"); // in this case, Master is Charlie
			printf("Master -> Slave %s\n", modem.latest_tx_stamp);
			sprintf(buf3, "echo '%s\n' >> %s",modem.latest_tx_stamp,M2S_TXPATH);
			system(buf3);

			a_modem_wait_info("@", TIMEOUT, buf, BUFSIZE);
			sscanf(buf, "%*s @ %s > %*s",buf2);
			printf("Master -> Slave RX time:%s\n", buf2);
			sprintf(buf3, "echo '%s\n' >> %s",buf2,M2S_RXPATH);
			system(buf3);

			a_modem_record(Record_time*1000);

			a_modem_wait_info("WAV",TIMEOUT, buf2, BUFSIZE);
			a_modem_wait_info("TX", TIMEOUT, buf, BUFSIZE);
			printf("Slave -> Master %s", buf);
			printf("Slave -> Master RX time:%s\n", buf2);
			sprintf(buf3, "echo '%s\n' >> %s",buf,S2M_TXPATH);
			system(buf3);
			sprintf(buf3, "echo '%s\n' >> %s",buf2,S2M_RXPATH);
			system(buf3);

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
