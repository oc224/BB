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
	printf("%s<%d>:",t_node.name, cnt);
	fgets(buf, BUFSIZE, stdin);
	sscanf(buf,"%s",arg0);
	//printf("debug %s\n",arg0);
	/*decode*/
	if (strcmp(arg0,"talk")==0){
		t_cmd.type=TALK;
		t_cmd.isremote=1;
	}else if (strcmp(arg0,"atalk")==0){
		t_cmd.type=ATALK;
		t_cmd.isremote=1;
	}else if (strcmp(arg0,"con")==0){
		t_cmd.type=CONVERSATION;
		t_cmd.isremote=1;
	}else if (strcmp(arg0,"conend")==0){
		t_cmd.type=CONEND;
		t_cmd.isremote=1;
	}else if(strcmp(arg0,"play")==0){
		t_cmd.type=MSPLAY;
		t_cmd.isremote=0;
	}else if (strcmp(arg0,"record")==0){
		t_cmd.type=MSRECORD;
		t_cmd.isremote=0;
	}else if (strcmp("sync",arg0)==0){
		t_cmd.type=SYNCALL;
		t_cmd.isremote=1;
	}else if (strcmp("upload",arg0)==0){
		t_cmd.type=UPLOAD;
		t_cmd.isremote=0;
	}else if (strcmp("quick",arg0)==0){
		t_cmd.type=QUICK;
		t_cmd.isremote=1;
	}else if (strcmp("status",arg0)==0){
		t_cmd.type=STATUS;
		t_cmd.isremote=1;
	}else if (strcmp("sr",arg0)==0){
		t_cmd.type=SEND_REMOTE;
		t_cmd.isremote=0;
	}else if (strcmp("wr",arg0)==0){
		t_cmd.type=WAIT_REMOTE;
		t_cmd.isremote=0;
	}else if (strcmp("showmsg",arg0)==0){
		t_cmd.type=MSG_SHOW;
		t_cmd.isremote=0;
	}else if (strcmp("clearffs",arg0)==0){
		t_cmd.type=CLEAR_FFS;
		t_cmd.isremote=0;
	}else if (strcmp("help",arg0)==0){
		t_cmd.type=HELP;
		t_cmd.isremote=0;
	}else if (strcmp("gpslog",arg0)==0){
		t_cmd.type=GPSLOG;
		t_cmd.isremote=0;
	}else if (strcmp("rreboot",arg0)==0){
		t_cmd.type=RREBOOT;
		t_cmd.isremote=1;
	}else{
		t_cmd.type=NONE;
		t_cmd.isremote=0;
	}
	/*return*/
	if (t_cmd.type>0)
		cnt++;
	return t_cmd.type;
}

int main()
{
	char remote[20];
	/*init cfg...*/
	system_cfg_read();
//	system_cfg_show();
//	printf("NODE NAME : %s\n ",t_node.name);
	a_modem_init();
	a_modem_open();
	scheduler_init();
	scheduler_read("/home/root/config/schedule.txt");
//	scheduler_task_show();

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
		case ATALK://ok
		master_atalk();
		break;
		case CONVERSATION:
		master_con();
		break;
		case CONEND:
		master_conend();
		break;
		case QUICK:
		master_quick();
		break;
		case MSPLAY://ok
		play(buf);
		break;
		case MSRECORD://ok
		record(buf);
		break;
		case SYNCALL://ok
		master_sync();
		break;
		case HELP:
		help();
		break;
		case UPLOAD:
		upload(buf);
		break;
		case SEND_REMOTE://ok
		msg_send();
		break;
		case MSG_SHOW://ok
		a_modem_msg_show(&msg);
		a_modem_msg_show(&msg_remote);
		break;
		case WAIT_REMOTE://ok
		wait_remote(buf);
		break;
		case CLEAR_FFS:
		a_modem_ffs_clear();
		break;
		case STATUS:
		
		break;
		case GPSLOG:
		system("gpspipe -r -n 12 | grep GPGGA >> /home/root/log/gpslog.txt");
		break;
		case RREBOOT:
		master_rreboot();
		break;
		case NONE:
		a_modem_puts(buf);
		a_modem_print(1000);
		break;
		default:
			fprintf(stderr, "ERROR: please input readable command (small letter)\n");
			break;
		}
	}

a_modem_close();
return 0;
}
