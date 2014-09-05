#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "acoustic_modem.h"
#define BUFSIZE 256
typedef enum{
PLAY,
RECORD,
SYNCALL,
HELP,
UPLOAD,
NONE,
MSG_SEND,
MSG_SHOW,
WAIT_REMOTE,
CLEAR_FFS
}cmd;

/*
#define LISTSIZE 16
//command list
typedef struct{
char *cmd[LISTSIZE];
int i;

} cmd_list;*/

char buf[BUFSIZE];

int get_command(){
static int cnt=1;
cmd tcmd;
buf[0]=0;
fprintf(stdout,"unicon<%d>:",cnt);
fgets(buf,BUFSIZE,stdin);
if (strstr(buf,"play"))tcmd=PLAY;
else if (strstr(buf,"record"))tcmd=RECORD;
else if (strstr(buf,"syncall"))tcmd=SYNCALL;
else if (strstr(buf,"help"))tcmd=HELP;
else if (strstr(buf,"upload"))tcmd=UPLOAD;
else if (strstr(buf,"wr"))tcmd=WAIT_REMOTE;
else if (strstr(buf,"sr"))tcmd=MSG_SEND;
else if (strstr(buf,"showmsg"))tcmd=MSG_SHOW;
else if (strstr(buf,"clearffs"))tcmd=CLEAR_FFS;
else tcmd=NONE;
if (tcmd!=NONE)cnt++;
return tcmd;
}

void amodem_enter_tx(){
char buf[BUFSIZE];
fgets(buf,BUFSIZE,stdin);
strcpy(modem.def_tx_wav,buf);
printf("set default tx file name : %s\n",modem.def_tx_wav);
}

int main(){
int arg1;
char arg_str[48];
amodem_init();
amodem_open();

while (1){
/*input & do cmd*/
switch (get_command()){
case MSG_SHOW:
amodem_msg_show(&msg_local);
amodem_msg_show(&msg_remote);
break;
case MSG_SEND:
//sscanf(buf,"%*s %s",arg_str);
amodem_msg_send(buf+3);
arg_str[0]=0;
break;
case WAIT_REMOTE:
amodem_wait_remote(arg_str,48,5000);
printf("remote %s\n",arg_str);
break;
case PLAY:
arg1=sscanf(buf,"%*s %s",arg_str);
if (arg1<1){
if (modem.def_tx_wav[0]==0)amodem_enter_tx();
amodem_play(modem.def_tx_wav);
}else{
amodem_play(arg_str);
}
break;
case RECORD:
sscanf(buf,"%*s %d",&arg1);
amodem_record(arg1);
break;
case SYNC:
amodem_sync_clock_gps(20);
amodem_sync_time_gps();
system("ntpdate -u 211.22.103.157");
break;
case CLEAR_FFS:
amodem_ffs_clear();
break;
case HELP:
printf("play\n");
printf("record\n");
printf("syncall\n");
printf("help\n");
printf("upload\n");
printf("wr\n");
printf("sr\n");
printf("clearffs\n");
printf("showmsg\n");
break;
case UPLOAD:
sscanf(buf,"%*s %s",arg_str);
amodem_upload_file(arg_str);
break;
default:
amodem_puts(buf);
break;
}
/*output*/
amodem_print(1000);
}

amodem_close();
return 0;
}
