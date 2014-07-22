#include <stdio.h>

/*
int w_modem_open(int baudrate);//enetering command mode
int w_modem_get_info();// get wireless modem info, addr...etc
void w_modem_info_show();// show info.
void w_modem_close();// quit command mode
int w_modem_connect(int dest_addr);// connect to a given node
*/
void main(){
	printf("!w_test\n");

/*	printf("!open\n");
	w_modem_open();

	printf("!close\n");
	w_modem_close();*/

	printf("!get info\n");
	w_modem_info_get();

	printf("!show info\n");
	w_modem_info_show();


}
