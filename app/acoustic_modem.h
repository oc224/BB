#include <stdio.h>
#include "log.h"
#define amodem_serial_baudrate 115200
#define amodem_dev_path "/dev/ttyUSB2"
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
} amodem_sync_state;

typedef struct {
	int fd;
	float board_temp; //ref modem manual, atv
	float dsp_bat;
	float mdm_bat; // ref modem manual mdm_battery
	float rtc_bat; // ref modem manual rtc_battery
	amodem_sync_state sync_state;
	char latest_tx_stamp[TX_SIZE];
	char latest_rx_fname[TX_SIZE];
	char def_tx_wav[TX_SIZE];
	FILE *tx_p;
	FILE *rx_p;
	logger *com_logger;
} amodem;

typedef struct {
} a_network;

typedef struct{
	char* text[LIST_SIZE];
	int i;/*point to latest msg aka text[i] is latest msg*/
	/*New text[i]>text[i-1]>...>text[i-N_unread+1]*/
	int N_unread;/*number of unread msg*/
}amodem_msg;

extern amodem modem;
extern amodem_msg msg_local;
extern amodem_msg msg_remote;

int amodem_init();/*init amodem*/
int amodem_open();/*open the serial port*/
inline void amodem_close();/*close the serial port*/

void amodem_msg_show(amodem_msg *);/*show msg list*/
int amodem_msg_add(amodem_msg*,char *msg_str);/*add to msg list*/

int amodem_wait_ack(char*,int);/*wait local msg ack*/
int amodem_wait_info(char *key_word, int timeout, char *info,int info_size);/*wait local msg info*/
int amodem_wait_remote(char*,int,int);/*wait remote*/
int amodem_gets(char* buf,int size);/**/
inline int amodem_puts(const char*msg);/**/
inline void amodem_clear_io_buffer();

int amodem_play(char * filename);
int amodem_record(int duration_mili);

int amodem_status();
void amodem_status_show();

int amodem_print_configs(char * filepath); // save cfg all output for future ref
int amodem_cfg_set(const char *);
#define amodem_cfg_deploy() amodem_cfg_set(CFG_DEPLOY);
#define amodem_cfg_devel() amodem_cfg_set(CFG_DEVEL);


int amodem_sync_clock_gps(int);
int amodem_sync_time_gps();
int amodem_is_clock_Sync(int);
int amodem_sync_status();

int amodem_upload_file(const char *fname);/*upload a file in /sd on the modem*/
int amodem_msg_send(const char*msg);/*send msg to remote*/

int amodem_ffs_clear();/*clean up ffs*/
void amodem_print(int);/*show msg from remote*/
