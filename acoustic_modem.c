#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 128
#define SERIAL_TIMEOUT 500

int a_modem_open(){
if (RS232_OpenComport(a_modem_dev_no,a_modem_serial_baudrate)){
printf("Acoustic modem, Fail to open.\n");
return FAIL;
}
RS232_SendBuf(a_modem_dev_no,"+++",3);
RS232_SendBuf(a_modem_dev_no,"at\r",3);
return SUCCESS;
}

int a_modem_close(){
RS232_CloseComport(a_modem_dev_no);
return SUCCESS;
}

inline void a_modem_clear_FIFO(){
RS232_Flush(a_modem_dev_no);
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
char buf[BUFSIZE];char buf2[BUFSIZE];
int delay=0;int n;
// play
sprintf(buf,"play /ffs/%s\r",filename);//use strcat instead?
if (!RS232_SendBuf(a_modem_dev_no,buf,strlen(buf))){
printf("Acoustic modem, send command error\n");
return FAIL;
}
// get time stamp
if (n=a_modem_wait_info("tx",1000,buf,BUFSIZE)){
buf[n-1]=0;//remove carriage return
printf("info : %s\n",buf);
sprintf(buf2,"echo '%s,%s' >> TXLOG.TXT",filename,buf);
system(buf2);//TODO store tx stamp, save BB time also
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
return SUCCESS;
}

// sync clock
int a_modem_sync_gps(){
	char buf[BUFSIZE];
	char buf2[BUFSIZE];
	a_modem_clear_FIFO();
	RS232_SendBuf(a_modem_dev_no,"sync\r",5);
	if (!a_modem_Is_Sync(10,3)){
		printf("A_modem, fail to sync\n");
		return FAIL;
	}
	RS232_SendBuf(a_modem_dev_no,"gpsd\r",5);
	a_modem_close();
	usleep(1000000);
	system("gpspipe -r -n 12 >> /dev/ttyUSB2");//TODO
	a_modem_open();
	a_modem_clear_FIFO();
	RS232_SendBuf(a_modem_dev_no,"date\r",5);
	usleep(100000);
	a_modem_wait_info("2014",1000,buf,BUFSIZE);
	sprintf(buf2,"echo '%s' >> AMODEM.TXT",buf);
	system(buf2);
	RS232_SendBuf(a_modem_dev_no,"date -store\r",12);
	RS232_SendBuf(a_modem_dev_no,"@latituder\r",11);
	RS232_SendBuf(a_modem_dev_no,"@longitude\r",11);
	//TODO sleep ?
	//TODO CHECK
	return SUCCESS;
}

// samp_interval (sec)
int a_modem_Is_Sync(int samp_interval,int N_retry){
	int i;
	a_modem_clear_FIFO();
	if ((samp_interval<=0)|(N_retry<=0)){
		printf("IS_SYNC, input error");
		return FAIL;
	}
	for (i=0;i<N_retry;i++){
		RS232_SendBuf(a_modem_dev_no,"sync\r",5);
		if (a_modem_wait_ack("synchronized",1000)){
			//printf("a_modem, sync\n");
			return TRUE;
		}
		usleep(samp_interval*1000000);
	}
	printf("a_modem time out\n");
	return FALSE;
}
// list files
int a_modem_ls();

inline int a_modem_wait_info(char *key_word,int timeout,char *info,int info_size){
return RS232_wait_info(a_modem_dev_no,key_word,timeout,info,info_size);
}

inline int a_modem_wait_ack(char *ack_msg,int timeout){
return RS232_wait_ack(a_modem_dev_no,ack_msg,timeout);
}
