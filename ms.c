#include "acoustic_modem.h"
#include "ms.h"
#include "system.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define BUFSIZE 80
//TODO upload_data upgrade
//TODO send time stamp

int master_talk(){
char buf[BUFSIZE];
/*sync*/
master_sync();
/*send ack*/
a_modem_msg_send(ACK);
/*play*/
a_modem_play("t1.wav");
/*wait ack*/
a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*send stamp*/
a_modem_msg_send(modem.latest_tx_stamp+8);
/*wait*/
a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*sync*/
master_sync();
/*send ack*/
a_modem_msg_send(ACK);
/*record*/
a_modem_record(1000);
/*send ack*/
a_modem_msg_send(ACK);
/*recv stamp*/
a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
return 0;
}

int slave_talk(){
char buf[BUFSIZE];
/*sync*/
slave_sync();
/*send ack*/
a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*record*/
a_modem_record(1000);
/*send ack*/
a_modem_msg_send(ACK);
/*recv stamp*/
a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
/*send ack*/
a_modem_msg_send(ACK);
/*sync*/
slave_sync();
/*wait ack*/
a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*play*/
a_modem_play("t1.wav");
/*wait ack*/
a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*send stamp*/
a_modem_msg_send(modem.latest_tx_stamp+8);
return 0;
}


int master_sync(){
int i;
char buf[BUFSIZE];
a_modem_sync_clock_gps();
a_modem_sync_time_gps();
for (i=0;i<3;i++){
a_modem_msg_send(ACK);
if (a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT)==SUCCESS)return SUCCESS;
}
printf("remote time out\n");
return FAIL;
}

int slave_sync(){
char buf[BUFSIZE];
a_modem_sync_clock_gps();
a_modem_sync_time_gps();
/*sync*/
printf("sync time done\n");
a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
a_modem_msg_send(ACK);
return 0;
}

void help(){
printf("talk\n");
printf("play\n");
printf("record\n");
printf("sync\n");
printf("help\n");
printf("wr --wait remote msg\n");
printf("clearffs\n");
                printf("upload\n");
                printf("wr\n");
                printf("sr\n");
                printf("clearffs\n");
                printf("showmsg\n");
}

int play(const char *buf){
char txname[40];
if (sscanf(buf,"%*s %s",txname)==1){
return a_modem_play(txname);
}else if (strlen(modem.def_tx_wav)>3){
return a_modem_play(modem.def_tx_wav);
}else{
printf("no default tx filename\n");
return FAIL;
}
}

int record(const char *buf){
int mili;
if (sscanf(buf,"%*s %d",&mili)==1) return a_modem_record(mili);
else{
printf("no duration arg, use default value 1000ms\n");
return a_modem_record(1000);
}
}


int upload(const char *buf){
char fname[40];
char * plog;
printf("debug %s\n",buf);
if (sscanf(buf,"%*s %s",fname)<1){
fprintf(stderr,"upload. input invalid\n");
return FAIL;
}
a_modem_upload_file(fname);

strcpy(strstr(fname,"log"),"wav");
printf("download %s as well? (y or n)\n",fname);
if (fgetc(stdin)=='y') return a_modem_upload_file(fname);
return SUCCESS;
}


int msg_send(){
char msg[80];
printf("enter msg :");
fgets(msg,80,stdin);
return a_modem_msg_send(msg);
}

int wait_remote(){
char buf[80];
a_modem_wait_remote(buf,80,REMOTE_TIMEOUT);
printf("%s\n",buf);
return 0;
}
