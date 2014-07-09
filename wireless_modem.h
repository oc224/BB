#define w_modem_serial_baud_rate 115200 //s102 1
#define w_modem_net_addr//s104 ex.1234567890
#define w_modem_unit_addr//s105 2-65534
#define w_modem_pwr//s108 output pwr 20-30
#define w_modem_RSSI//s123
#define w_modem_command_mode "+++"

typedef struct {
int unit_addr;
int dest_addr;
int net_addr;
int rssi;
int pwr;
}w_modem;

//enetering command mode
int w_modem_open(int dev_no,int baudrate);

// get wireless modem info, addr...etc
int w_modem_get_info(int dev_no, *w_modem);

// quit command mode
int w_modem_close(int dev_no);

// connect to a given node
int w_modem_connect(int dest_addr_modem,int dest_ip);
