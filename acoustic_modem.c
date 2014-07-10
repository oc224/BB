#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"
#include <stdio.h>
#include <string.h>
#define BUFSIZE 128
#define SERIAL_TIMEOUT 500

int a_modem_open(){
if (RS232_OpenComport(a_modem_dev_no,a_modem_serial_baudrate)){
printf("Acoustic modem, Fail to open.\n");
return FAIL;
}
RS232_SendBuf(a_modem_dev_no,"at\r",3);
return SUCCESS;
}

int a_modem_close(){
RS232_CloseComport(a_modem_dev_no);
return SUCCESS;
}

inline void a_modem_clear_FIFO(){
tcflush(a_modem_dev_no,TCIOFLUSH);
}

int a_modem_set_devel_configs(){
tcflush(a_modem_dev_no,TCIFLUSH);
RS232_SendBuf(a_modem_dev_no,"+++\r",4);
RS232_SendBuf(a_modem_dev_no,"@P1EchoChar=Ena\r",16);
RS232_SendBuf(a_modem_dev_no,"@TxPower=1\r",11);
RS232_SendBuf(a_modem_dev_no,"cfg store\r",10);
return SUCCESS;
}

int a_modem_set_deploy_configs(){
tcflush(a_modem_dev_no,TCIFLUSH);
RS232_SendBuf(a_modem_dev_no,"+++",3);
RS232_SendBuf(a_modem_dev_no,"@TxPower=8\r",11);
RS232_SendBuf(a_modem_dev_no,"cfg store\r",10);
return SUCCESS;
}

int a_modem_play(char * filename){
tcflush(a_modem_dev_no,TCIFLUSH);
char buf[BUFSIZE];
int delay=0;
// play
sprintf(buf,"play /ffs/%s\r",filename);//use strcat instead?
if (!RS232_SendBuf(a_modem_dev_no,buf,strlen(buf))){
printf("Acoustic modem, send command error\n");
return FAIL;
}
// get time stamp
if (a_modem_wait_info("tx",1000,buf,BUFSIZE)){
printf("info : %s\n",buf);
return SUCCESS;
}else{
printf("info timeout\n");
return FAIL;
}
}

int a_modem_record(char * timestamp,int duration){
RS232_SendBuf(a_modem_dev_no,"record on\r",10);
usleep(duration*1000);
RS232_SendBuf(a_modem_dev_no,"record off\r",11);
}

// sync clock
int a_modem_sync_gps();

// list files
int a_modem_ls();

int a_modem_wait_info(char *key_word,int timeout,char *info,int info_size){
//non block
int n;
char buf[BUFSIZE];
int delay=0;
while(delay<timeout){
n=RS232_PollComport(a_modem_dev_no,buf,BUFSIZE);
if (n<1){
delay++;
}else{
buf[n]=0;
printf("%s\n",buf);//debug
if (n<info_size){
if (strcasestr(buf,key_word)){
strcpy(info,buf);//copy msg
return n;
}
}else{
printf("info_size to small\n");
return FAIL;
}
}
usleep(1000);
}
// timeout
printf("ack timeout\n");
return FALSE;
}

int a_modem_wait_ack(char *ack_msg,int timeout){
//non block, ensure FIFO clear after this call
int n;
char buf[BUFSIZE];
int delay=0;
while(delay<timeout){
n=RS232_PollComport(a_modem_dev_no,buf,BUFSIZE);
if (n<1){//input not ready
delay++;
}else{// input ready
buf[n]=0;
printf("%s",buf);//debug
if (strcasestr(buf,ack_msg))return TRUE;
delay=0;//TODO find better way
}
usleep(1000);
}
printf("ack timeout\n");
return FALSE;
}
