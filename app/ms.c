#include "amodem.h"
#include "ms.h"
#include "system.h"
#include "scheduler.h"
#include <stdlib.h>
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
amodem_puts_remote(ACK);
/*play*/
amodem_play("t1.wav");
/*wait ack*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*send stamp*/
amodem_puts_remote(modem.latest_tx_stamp+8);
/*wait*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*sync*/
master_sync();
/*send ack*/
amodem_puts_remote(ACK);
/*record*/
amodem_record(1000);
/*send ack*/
amodem_puts_remote(ACK);
/*recv stamp*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
return 0;
}

int slave_talk(){
char buf[BUFSIZE];
/*sync*/
slave_sync();
/*wait ack*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*record*/
amodem_record(1000);
/*send ack*/
amodem_puts_remote(ACK);
/*recv stamp*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
/*send ack*/
amodem_puts_remote(ACK);
/*sync*/
slave_sync();
/*wait ack*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*play*/
amodem_play("t1.wav");
/*wait ack*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*send stamp*/
amodem_puts_remote(modem.latest_tx_stamp+8);
return 0;
}

int master_atalk(){
char buf[BUFSIZE];
/*sync*/
master_sync();
/*send ack*/
amodem_puts_remote(ACK);
/*play*/
amodem_play("t1.wav");
/*wait ack*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*record*/
amodem_record(1000);
/*recv stamp*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
printf("Remote TX @ %s\n",buf);
return 0;}

int slave_atalk(){
char buf[BUFSIZE];
/*sync*/
slave_sync();
/*wait ack*/
amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT);
/*record*/
amodem_record(1000);
/*send ack*/
amodem_puts_remote(ACK);
/*play*/
amodem_play("t1.wav");
/*send stamp*/
amodem_puts_remote(modem.latest_tx_stamp+8);
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
}

int master_quick(){
/*play*/
amodem_play("t1.wav");
/*wait ack*/
amodem_wait_remote(NULL,0,REMOTE_TIMEOUT);
/*record*/
amodem_record(1000);
return 0;
}

int slave_quick(){
/*record*/
amodem_record(1000);
/*send ack*/
amodem_puts_remote(ACK);
/*play*/
amodem_play("t1.wav");
return 0;
}

int master_sync(){
int i;
char buf[BUFSIZE];
int clock[2],time[2];
/*local sync*/
clock[0]=amodem_sync_clock_gps(10);
time[0]=amodem_sync_time_gps();
/*active wait response*/
for (i=0;i<3;i++){
amodem_puts_remote(ACK);
if (amodem_wait_remote(buf,BUFSIZE,REMOTE_TIMEOUT)==SUCCESS){
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
clock=amodem_sync_clock_gps(10);
time=amodem_sync_time_gps();
sprintf(buf,"%d %d",clock,time);
printf("sync time done: %d %d\n",clock,time);
/*passive response*/
if (amodem_wait_remote(NULL,BUFSIZE,REMOTE_TIMEOUT)==SUCCESS)amodem_puts_remote(buf);
return 0;
}

void help(){
system("cat /home/root/config/help.txt");}

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
if (sscanf(buf,"%*s %s",fname)<1){
fprintf(stdout,"download last data\n");
strcpy(fname,modem.latest_rx_fname);
}
amodem_upload_file(fname);

strcpy(strstr(fname,"log"),"wav");
printf("download %s as well? (y or n)\n",fname);
if (fgetc(stdin)=='y') return amodem_upload_file(fname);
return SUCCESS;
}


int puts_remote(){
char msg[80];
printf("enter msg :");
fgets(msg,80,stdin);
return amodem_puts_remote(msg);
}

int wait_remote(){
char buf[80];
amodem_wait_remote(buf,80,REMOTE_TIMEOUT);
printf("%s\n",buf);
return 0;
}

int master_rreboot(){
printf("this will take 25 seconds, wait\n");
sleep(25);
/*wait ack*/
amodem_wait_remote(NULL,0,REMOTE_TIMEOUT);
return 0;
}

int slave_rreboot(){
amodem_puts("\r");
amodem_puts("reboot\r");
sleep(25);
/*send ack*/
amodem_puts_remote(ACK);
return 0;
}
