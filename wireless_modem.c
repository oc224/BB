#include "rs232.h"
#include "wireless_modem.h"
#include "system.h"
#include <string.h>
#define buf_size 32

//enetering command mode
int w_modem_open(int dev_no,int baudrate){
char buf[buf_size];
int n;
// open serial port
if (RS232_OpenComport(dev_no,baudrate)){
printf("W_modem, Can not open serial port\n");
return FAIL;
}
// send +++ return NO carrier ok
RS232_SendBuf(dev_no,w_modem_command_mode,3);
n=RS232_PollComport(dev_no,buf,buf_size);
buf[n]=0;
if (!strcasestr(buf,"NO CARRIER OK")){
printf("W_modem, Can not go to command mode\n");
return FAIL;
}
return SUCCESS;
}

// get wireless modem info, addr...etc
int w_modem_get_info(int dev_no, *w_modem){

}

// quit command mode
int w_modem_close(int dev_no){
// save config
RS232_SendBuf(dev_no,"AT&W\r",5);
// go to data mode
RS232_SendBuf(dev_no,"ATA\r",4);
RS232_CloseComport(dev_no);
}

// connect to a given node
int w_modem_connect(int dest_addr_modem,int dest_ip);


