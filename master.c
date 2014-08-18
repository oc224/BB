#include "system.h"
#include "acoustic_modem.h"
#include "ms.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 128
#define BUFSHORT 20
#define M2S_TXPATH "/home/root/log/M2S_TXLOG.txt"
#define M2S_RXPATH "/home/root/log/M2S_RXLOG.txt"
#define S2M_TXPATH "/home/root/log/S2M_TXLOG.txt"
#define S2M_RXPATH "/home/root/log/S2M_RXLOG.txt"

char buf[BUFSIZE];
cmd t_cmd;

int wait_command_user()
{
	/*t_cmd.typeurn command (enum or integer)*/
	static int cnt=1;
	char arg0[BUFSHORT];
	t_cmd.type=NONE;
	t_cmd.isremote=0;

	/*console prompt*/
	printf("master<%d>:", cnt);
	fgets(buf, BUFSIZE, stdin);
	sscanf(buf,"%s",arg0);

	/*decode*/
	if (strcmp(arg0,"talk")==0){
		t_cmd.type=TALK;
		t_cmd.isremote=1;
	}else if(strcmp(arg0,"play")==0){
		t_cmd.type=PLAY;
		t_cmd.isremote=0;
	}else if (strcmp(arg0,"record")==0){
		t_cmd.type=RECORD;
		t_cmd.isremote=0;
	}else if (strcmp("sync",arg0)==0){
		t_cmd.type=SYNCALL;
		t_cmd.isremote=1;
	}else if (strcmp("upload",arg0)==0){
		t_cmd.type=UPLOAD;
		t_cmd.isremote=0;
	}else{
		t_cmd.type=NONE;
		t_cmd.isremote=0;
	}
	/*return*/
	if (t_cmd.type>=0)
		cnt++;
	return t_cmd.type;
}

int main()
{
	char remote[20];
	/*init cfg...*/
	system_cfg_read();
	system_cfg_show();
	printf("NODE NAME : %s\n\n ",t_node.name);
	a_modem_init();
	a_modem_open();

	while (1)
	{
		wait_command_user();
		printf("command %d\n",t_cmd.type);
		if (t_cmd.isremote){
		sprintf(remote,"%d",t_cmd.type);
		a_modem_msg_send(remote);}
		switch (t_cmd.type)
		{
		case TALK://ok
		master_talk();
		break;
		case PLAY://ok
		play(buf);
		break;
		case RECORD://ok
		record(buf);
		break;
		case SYNCALL:
		master_sync();
		break;
		case HELP:
		help();
		break;
		case WAIT_REMOTE:
		wait_remote(buf);
		break;
		case CLEAR_FFS:
		a_modem_ffs_clear();
		break;
		case MSG_SHOW:
		a_modem_msg_show(&msg);
		a_modem_msg_show(&msg_remote);
		break;
		case MSG_SEND:
		msg_send();
		break;
		case UPLOAD:
		upload(buf);
		break;
		case NONE:
			fprintf(stderr, "ERROR: please input readable command (small letter)\n");
			break;
		default:
			fprintf(stderr, "ERROR: please input readable command (small letter)\n");
			break;
		}
	}

a_modem_close();
return 0;
}
