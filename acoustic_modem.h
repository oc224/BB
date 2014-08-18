#include "stdio.h"

#define a_modem_dev_no 18
#define a_modem_serial_baudrate 115200
#define a_modem_dev_path "/dev/ttyUSB2"
#define ONLINE_COMMAND 1600000 /*Time it takes from online mode to command mode*/
#define WAIT_INTVAL 100000 /*inteval time for reading serial port*/
#define N_ITER_DIV (WAIT_INTVAL/1000)
#define COMMAND_DELAY 200000 /*Latency of entering command mode*/
#define GPSPIPE_TIME 8 /*seconds that gpspipe feed modem*/
#define WAIT_TXTIME 5000
#define LIST_SIZE 16
#define TX_SIZE 32
#define CFG_DEPLOY "/home/root/config/modem_cfg_deploy.txt"
#define CFG_DEVEL "/home/root/config/modem_cfg_devel.txt"
/*point to oldest unread msg*/
#define MSG_PULL(msg) msg.text[(msg.i+LIST_SIZE+msg.N_unread-1)%LIST_SIZE]

typedef enum {
	NOT_SYNC, QUALIFY, SYNC
} a_modem_sync_state;

typedef struct {
	float board_temp; //ref modem manual, atv
	float dsp_bat;
	float mdm_bat; // ref modem manual mdm_battery
	float rtc_bat; // ref modem manual rtc_battery
	a_modem_sync_state sync_state;
	char latest_tx_stamp[TX_SIZE];
	char latest_rx_fname[TX_SIZE];
	char def_tx_wav[TX_SIZE];
	FILE *tx_p;//TODO
	FILE *rx_p;//TODO
} a_modem;

typedef struct {
} a_network;

typedef struct{
	char* text[LIST_SIZE];
	int i;/*point to latest msg aka text[i] is latest msg*/
	/*New text[i]>text[i-1]>...>text[i-N_unread+1]*/
	int N_unread;/*number of unread msg*/
}a_modem_msg;

extern a_modem modem;
extern a_modem_msg msg;
extern a_modem_msg msg_remote;

int a_modem_init();
int a_modem_open();
inline void a_modem_close();

void a_modem_msg_show(a_modem_msg *);
int a_modem_msg_add(a_modem_msg*,char *msg_str);

int a_modem_wait_ack(char*,int);
int a_modem_wait_info(char *key_word, int timeout, char *info,int info_size);
int a_modem_wait_remote(char*,int,int);
int a_modem_gets(char* buf,int size);
inline int a_modem_puts(const char*msg);
inline void a_modem_clear_io_buffer();

int a_modem_play(char * filename);
int a_modem_record(int duration_mili);

int a_modem_status();
void a_modem_status_show();

int a_modem_print_configs(char * filepath); // save cfg all output for future ref
int a_modem_cfg_set(const char *);
#define a_modem_cfg_deploy() a_modem_cfg_set(CFG_DEPLOY);
#define a_modem_cfg_devel() a_modem_cfg_set(CFG_DEVEL);


int a_modem_sync_clock_gps(int);
int a_modem_sync_time_gps();
int a_modem_is_clock_Sync(int);
int a_modem_sync_status();

int a_modem_upload_file(const char *fname);
int a_modem_msg_send(const char*msg);

int a_modem_ffs_clear();
void a_modem_print(int);
