#include "amodem.h"
#include "ms.h"
#include "common.h"
//#include "system.h"
//#include "scheduler.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "signal.h"
#define BUFSIZE 80
#define TX_DEFAULT "mseq10_T1_l1"
//TODO upload_data upgrade
//TODO send time stamp

/*int master_talk(){
char buf[BUFSIZE];
//sync
master_sync();
//send ack
amodem_puts_remote(255,ACK);
//play
amodem_play("t1.wav");
//wait ack
amodem_wait_remote(ACK,buf,BUFSIZE,REMOTE_TIMEOUT);
//send stamp
amodem_puts_remote(255,modem.latest_tx_stamp+8);
//wait
amodem_wait_remote(ACK,buf,BUFSIZE,REMOTE_TIMEOUT);
//sync
master_sync();
//send ack
amodem_puts_remote(255,ACK);
//record
amodem_record(1000);
//send ack
amodem_puts_remote(255,ACK);
//recv stamp
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
return 0;
}*/
/*
int slave_talk(){
char buf[BUFSIZE];
//sync
slave_sync();
//wait ack
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
//record
amodem_record(1000);
//send ack
amodem_puts_remote(255,ACK);
//recv stamp
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
//send ack
amodem_puts_remote(255,ACK);
//sync
slave_sync();
//wait ack
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
//play
amodem_play("t1.wav");
//wait ack
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
//send stamp
amodem_puts_remote(255,modem.latest_tx_stamp+8);
return 0;
}*/

/*int master_atalk(){
char buf[BUFSIZE];
//sync
master_sync();
//send ack
amodem_puts_remote(255,ACK);
//play
amodem_play("t1.wav");
//wait ack
amodem_wait_ack(&msg_remote,ACK,REMOTE_TIMEOUT);
//record
amodem_record(1000);
//recv stamp
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
return 0;}

int slave_atalk(){
char buf[BUFSIZE];
//sync
slave_sync();
//wait ack
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
//record
amodem_record(1000);
//send ack
amodem_puts_remote(255,ACK);
//play
amodem_play("t1.wav");
//send stamp
amodem_puts_remote(255,modem.latest_tx_stamp+8);
return 0;}

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
}*/
/*
int master_quick(){
//play
amodem_play("t1.wav");
//wait ack
amodem_wait_remote(NULL,0,REMOTE_TIMEOUT);
//record
amodem_record(1000);
return 0;
}

int slave_quick(){
//record
amodem_record(1000);
//send ack
amodem_puts_remote(255,ACK);
//play
amodem_play("t1.wav");
return 0;
}*/

int master_sync(){
int i;
char buf[BUFSIZE];
int clock[2],time[2];
//local sync
clock[0]=amodem_sync_clock_gps(10);
time[0]=amodem_sync_time_gps();
//active wait response
for (i=0;i<3;i++){
amodem_puts_remote(255,ACK);
if (amodem_wait_remote(NULL,REMOTE_TIMEOUT,buf,BUFSIZE)!=NULL){
sscanf(buf,"%d %d",clock+1,time+1);
break;}}
//show result
printf("\n\nSYNC\n");
printf("----------------------------------------\n");
printf("Local : Clock %d , Time %d\n",clock[0],time[0]);
printf("Remote : Clock %d , Time %d\n ",clock[1],time[1]);
printf("----------------------------------------\n");
return 0;
}

int slave_sync(){
char buf[BUFSIZE];
int clock,time;
//local sync
clock=amodem_sync_clock_gps(10);
time=amodem_sync_time_gps();
sprintf(buf,"%d %d",clock,time);
printf("sync time done: %d %d\n",clock,time);
//passive response
if (amodem_wait_remote(NULL,REMOTE_TIMEOUT,buf,BUFSIZE)!=NULL)amodem_puts_remote(255,buf);
return 0;
}

void help(){
system("cat /root/config/help.txt");}

int play(const char *buf){
char txname[40];
if (sscanf(buf,"%*s %s",txname)==1){
return amodem_play(txname);
}else if (strlen(modem.def_tx_wav)>3){
return amodem_play(modem.def_tx_wav);
}else{
printf("no default tx filename\n");
return FAIL;
}
}

int record(const char *buf){
int mili;
if (sscanf(buf,"%*s %d",&mili)==1) return amodem_record(mili);
else{
printf("no duration arg, use default value 1000ms\n");
return amodem_record(1000);
}
}


int upload(const char *buf){
char fname[40];
//input (default fname)
if (sscanf(buf,"%*s %s",fname)<1){
fprintf(stdout,"download last data\n");
strcpy(fname,modem.latest_rx_fname);
//upload log
amodem_upload_file(fname);
//upload wav
strcpy(strstr(fname,"log"),"wav");
amodem_upload_file(fname);
}else
amodem_upload_file(fname);
return SUCCESS;
}


int recanal(const char *arg){
//record & anal
char fname[40];
char path_in[100],path_out[100],tx[100];
DATA_COOK dc;
//input arg
fname[0]=0;
sscanf(arg,"%*s %s",fname);
if (strlen(fname)<1){
sprintf(tx,"/root/tx/%s.wav",TX_DEFAULT);
}else
{
sprintf(tx,"/root/tx/%s.wav",fname);
}

//record
amodem_record(2000);
sleep(2);

//upload
strcpy(fname,modem.latest_rx_fname);
amodem_upload_file(fname);
strcpy(strstr(fname,"log"),"wav");
amodem_upload_file(fname);

//anal
sprintf(path_in,"/root/raw_data/%s",fname);
strcpy(path_out,path_in);
strcpy(strstr(path_out,".wav"),".out");
wav2CIR(path_in,tx,path_out,&dc);
return 0;
}

/*
int puts_remote(255,){
char msg[80];
printf("enter msg :");
fgets(msg,80,stdin);
return amodem_puts_remote(255,msg);
}

int wait_remote(){
char buf[80];
amodem_wait_remote(buf,80,REMOTE_TIMEOUT);
printf("%s\n",buf);
return 0;
}*/
/*
int master_rreboot(){
printf("this will take 30 seconds, wait\n");
sleep(30);
//wait ack
amodem_wait_remote(ACK,0,REMOTE_TIMEOUT);
return 0;
}

int slave_rreboot(){
amodem_puts("\r");
amodem_puts("reboot\r");
sleep(25);
//send ack
amodem_puts_remote(255,ACK);
return 0;
}
*/
