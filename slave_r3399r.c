#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "system.h"
#include "acoustic_modem.h"
#include "MS_sleep_time.h"
#define BUFSIZE 128
#define TIMEOUT 2000

typedef enum{
	TALK,INIT,PROB
}command;

int wait_command_master(){
	int ret;
	static int cnt=1;
	char buf[BUFSIZE];

	while(1) //wait until remote msg received
	{
		if (a_modem_wait_remote("DATA",TIMEOUT,buf,BUFSIZE) >= 0)
			break;
	}

	/*decode*/
	if (sscanf(buf,"%d",&ret) < 1)
		ret = -1;

	/*return*/
	if (ret >= 0)
		cnt++;
	return ret;
}


int main()
{
	command t_cmd;
	char buf[BUFSIZE];

	while (1)
	{
		t_cmd = wait_command_master();
		switch (t_cmd)
		{
		case TALK: //DO TALK
			a_modem_record(Record_time*1000);
			a_modem_wait_info("@",TIMEOUT, buf, BUFSIZE);
			a_modem_msg_send(buf);
			sleep(WAIT_then_PLAY);
			a_modem_play("lfm_data_t3_l10.wav"); // in this case, Slave is Dylan
			sleep(Record_time-WAIT_then_PLAY);
			a_modem_msg_send(modem.latest_tx_stamp);
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
