#include "acoustic_modem.h"
#include "ms.h"
#include "system.h"
#include "scheduler.h"
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

int master_con(){
scheduler_start(0,0,0,'r');
return 0;}

int slave_con(){
scheduler_start(0,0,0,'r');
return 0;}

int master_conend(){
scheduler_stop();
return 0;
}

int slave_conend(){
scheduler_stop();
return 0;
}

int master_quick(){
/*play*/
a_modem_play("t1.wav");
/*wait ack*/
a_modem_wait_remote(NULL,0,REMOTE_TIMEOUT);
/*record*/
a_modem_record(1000);
return 0;
}

int slave_quick(){
/*record*/
a_modem_record(1000);
/*send ack*/
a_modem_msg_send(ACK);
/*play*/
a_modem_play("t1.wav");
return 0;
}

int master_sync(){
int i;
char buf[BUFSIZE];
int clock[2],time[2];
/*local sync*/
clock[0]=a_modem_sync_clock_gps(10);
time[0]=a_modem_sync_time_gps();
/*active wait response*/
for (i=0;i<3;i++){
a_modem_msg_send(ACK);
if (a_modem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT)==SUCCESS){
sscanf(buf,"%d %d",clock+1,time+1);
break;}}
/*show result*/
printf("Local : Clock %d , Time %d\n",clock[0],time[0]);
printf("Remote : Clock %d , Time %d\n ",clock[1],time[1]);
return 0;
}

int slave_sync(){
char buf[BUFSIZE];
int clock,time;
/*local sync*/
clock=a_modem_sync_clock_gps(10);
time=a_modem_sync_time_gps();
sprintf(buf,"%d %d",clock,time);
printf("sync time done: %d %d\n",clock,time);
/*passive response*/
if (a_modem_wait_remote(NULL,BUFSIZE,REMOTE_TIMEOUT)==SUCCESS)a_modem_msg_send(buf);
return 0;
}

void help(){
printf("talk\n");
printf("con\n");
printf("");
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
if (sscanf(buf,"%*s %s",fname)<1){
fprintf(stdout,"download last data\n");
strcpy(fname,modem.latest_rx_fname);
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

