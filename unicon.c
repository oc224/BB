#include <stdio.h>
#define BUFSIZE 256
typedef enum{
PLAY,
RECORD,
RECORD_OFF,
SYNC,
HELP,
UPLOAD,
}cmd;

#define LISTSIZE 16
//command list
typedef struct{
char *cmd[LISTSIZE];
int i;

} cmd_list;
char buf[BUFSIZE];

int get_command(){
static cnt=1;
buf[0]=0;
fprintf(stdout,"unicon<%d>:",cnt);
fgets(stdin,buf);
//switch case
if (strstr(buf,"play"))cmd=PLAY;
else if (strstr(buf,"record"))cmd=RECORD;
else if (strstr(buf,"record off"))cmd=REOCRD_OFF;
else if (strstr(buf,"sync"))cmd=SYNC;
else if (strstr(buf,"help"))cmd=HELP;
else if (strstr(buf,"upload"))cmd=UPLOAD;
return cmd;
}

void a_modem_enter_tx(){
char buf[BUFSIZE];
fgets(stdin,buf);
strcpy(modem.def_tx_wav,buf);
printf("set default tx file name : %s\n",modem.def_tx_wav);

}
int main(){
int arg1;
char arg_str[48];

a_modem_init();
a_modem_open();

while (1){
/*input & do cmd*/
switch (get_command()){
case PLAY:
arg1=sscanf(buf,"%s %s",arg_str);
if (arg1<1){
if (modem.def_tx_wav[0]==0)a_modem_enter_tx();
a_modem_play(modem.def_tx_wav);
}else{
a_modem_play(arg_str);
}
break;
case RECORD:
sscanf("%*s %d",&arg1);
a_modem_record(arg1);
break;
case SYNC:
a_modem_sync_clock_gps();
a_modem_sync_time_gps();
system("ntpdate -u 211.22.103.157");
break;
case HELP:

case UPLOAD:
sscanf("%*s %s",arg_str);
a_modem_upload_file(arg_str);
break;
default
a_modem_puts(buf);
break;
}
/*output*/
a_modem_print(1000);
}

a_modem_close();
return 0;
}
