#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wireless_modem.h"
#include "rs232.h"
#include "system.h"


static w_modem w_modem_info;
int w_modem_err(char *msg) {
	fprintf(stderr, "Wireless modem, %s\n", msg);
	return 0;
}

int w_modem_open() {
//Entering command mode
// open serial port
	if (RS232_OpenComport(w_modem_dev_no, w_modem_serial_baud_rate)) {
		printf("W_modem, Can not open serial port\n");
		return FAIL;
	}
	// Detect op mode
	RS232_SendBuf(w_modem_dev_no,"ats\r",4);
	if (RS232_wait_ack(w_modem_dev_no,"error",w_modem_default_timeout)==SUCCESS){
		printf("W_modem, in command mode\n");
		return SUCCESS;
	}
// send +++ return NO carrier ok
	RS232_SendBuf(w_modem_dev_no,"+++", 3);
	sleep(1);
	if (RS232_wait_ack(w_modem_dev_no,"OK",w_modem_default_timeout)==FAIL){
		printf("W_modem, fail to go to command mode\n");
		return FAIL;
	}
	return SUCCESS;
}

int w_modem_info_get() {
	char buf[BUFSIZE];
	int n, return_state=SUCCESS;
// get wireless modem info, addr...etc
	//open
	if (w_modem_open()==FAIL)return FAIL;
	RS232_Flush(w_modem_dev_no);
	//RSSI
	RS232_SendBuf(w_modem_dev_no,"ats123?\r",8);
	usleep(500000);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	if (n<1){
		printf("W_modem, fail to get wireless modem info (RSSI)\n");
		return_state=FAIL;
	}
	buf[n]=0;
	if (strstr(buf,"N/A")){
		printf("RSSI not available\n");
		w_modem_info.rssi=999;//TODO N/A case
	}else{
		w_modem_info.rssi=atoi(buf);
	}
	memset(buf,0,BUFSIZE);
	//PWR
	RS232_Flush(w_modem_dev_no);
	RS232_SendBuf(w_modem_dev_no,"ats108?\r",8);//pwr
	usleep(500000);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	if (n<1){
		printf("W_modem, fail to get wireless modem info (Power)\n");
		return_state=FAIL;
	}else{
		buf[n]=0;
		w_modem_info.pwr=atoi(buf);
	}
	memset(buf,0,BUFSIZE);
	//UNIT addr
	RS232_Flush(w_modem_dev_no);
	RS232_SendBuf(w_modem_dev_no,"ats105?\r",8);
	usleep(500000);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	if (n<1){
		printf("W_modem, fail to get wireless modem info (unit addr)\n");
		return_state=FAIL;
	}else{
	buf[n]=0;
	w_modem_info.unit_addr=atoi(buf);
	}
	memset(buf,0,BUFSIZE);
	//dest addr
	RS232_Flush(w_modem_dev_no);
	RS232_SendBuf(w_modem_dev_no,"ats140?\r",8);
	usleep(500000);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	if (n<1){
		printf("W_modem, fail to get wireless modem info (dest addr)\n");
		return_state=FAIL;
	}else{
		buf[n]=0;
		w_modem_info.dest_addr=atoi(buf);
	}
	memset(buf,0,BUFSIZE);
	//net addr
	RS232_Flush(w_modem_dev_no);
	RS232_SendBuf(w_modem_dev_no,"ats104?\r",8);
	usleep(500000);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	n=RS232_PollComport(w_modem_dev_no,buf,BUFSIZ);
	if (n<1){
		printf("W_modem, fail to get wireless modem info (net addr)\n");
		return_state=FAIL;
	}else{
	buf[n]=0;
	w_modem_info.net_addr=atoi(buf);
	}
	memset(buf,0,BUFSIZE);
	w_modem_close();
	return return_state;
}

void w_modem_info_show(){
	printf("wireless modem info\n");
	printf("Power : %d\n",w_modem_info.pwr);
	printf("RSSI : %d\n",w_modem_info.rssi);
	printf("Unit Addr : %d\n",w_modem_info.unit_addr);
	printf("Dest Addr : %d\n",w_modem_info.dest_addr);
	printf("Net Addr : %d\n",w_modem_info.net_addr);

}

int w_modem_close() {
// quit command mode
// save config
	RS232_SendBuf(w_modem_dev_no, "AT&W\r", 5);
	if (RS232_wait_ack(w_modem_dev_no,"OK",w_modem_default_timeout)==FAIL){
		printf("W_modem, fail to save config\n");
		return FAIL;
	}
// go to data mode
	RS232_SendBuf(w_modem_dev_no, "ATA\r", 4);
	if (RS232_wait_ack(w_modem_dev_no,"OK",w_modem_default_timeout)==FAIL){
		printf("W_modem, fail to go to data mode\n");
		return FAIL;
	}
	RS232_CloseComport(w_modem_dev_no);
	return SUCCESS;
}

int w_modem_connect(int dest_addr) {
	// connect to a given node
	char buf[BUFSIZE];
	if (w_modem_open()==FAIL){
		printf("W_modem, fail to open\n");
		return FAIL;
	}
	//NET ADDR
	sprintf(buf, "ats140=%d\r", dest_addr);
	RS232_SendBuf(w_modem_dev_no, buf, strlen(buf));
	if (RS232_wait_ack(w_modem_dev_no,"OK",w_modem_default_timeout)==FAIL){
		printf("W_modem, fail to set network addr\n");
		return FAIL;
	}
	//DEST ADDR
	sprintf(buf, "ats104=%d\r", dest_addr);
	RS232_SendBuf(w_modem_dev_no, buf, strlen(buf));
	if (RS232_wait_ack(w_modem_dev_no,"OK",w_modem_default_timeout)==FAIL){
		printf("W_modem, fail to set dest addr\n");
		return FAIL;
	}
	w_modem_close();
	return SUCCESS;
}
