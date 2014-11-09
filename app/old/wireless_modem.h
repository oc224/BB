#ifndef WIRELESS_MODEM_H
#define WIRELESS_MODEM_H
#define w_modem_serial_baud_rate 115200 //s102 1
#define w_modem_net_addr//s104 ex.1234567890
#define w_modem_unit_addr//s105 2-65534
#define w_modem_pwr//s108 output pwr 20-30
#define w_modem_RSSI//s123
#define w_modem_command_mode "+++"
#define w_modem_dev_no 17 //dev/ttyUSB1
#define w_modem_default_timeout 1000

typedef struct {
	int unit_addr;
	int dest_addr;
	int net_addr;
	int rssi;
	int pwr;
} w_modem;


int w_modem_open();//enetering command mode
int w_modem_info_get();// get wireless modem info, addr...etc
void w_modem_info_show();// show info.
int w_modem_close();// quit command mode
int w_modem_connect(const char *);// connect to a given node

#endif
