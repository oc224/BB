#define a_modem_dev_no 18
#define a_modem_serial_baudrate 115200
#define a_modem_dev_path "/dev/ttyUSB2"
#define LIST_SIZE 16
#define TX_SIZE 32

#include "stdio.h"
typedef enum {
	NOT_SYNC, QUALIFY, SYNC
} a_modem_sync_state;

typedef struct {
	float board_temp; //ref modem manual, atv
	float dsp_bat;
	float mdm_bat; // ref modem manual mdm_battery
	float rtc_bat; // ref modem manual rtc_battery
	a_modem_sync_state sync_stae; //synchronized or ...
	char latest_tx_stamp[TX_SIZE];
	char latest_rx_fname[TX_SIZE];
	char def_tx_wav[TX_SIZE];
	FILE *tx_p;
	FILE *rx_p;
} a_modem;

typedef struct {
} a_network;

typedef enum{
	REC,REC_PLY,TX
}a_modem_command;

typedef struct{
	char* text[LIST_SIZE];
	int i;
	char* wanted;
}a_modem_msg;

int a_modem_init();
int a_modem_open();
inline void a_modem_close();

void a_modem_msg_show();
int a_modem_msg_add(char *msg_str);

inline int a_modem_wait_ack(char *ack_msg, int timeout_mili);
inline int a_modem_wait_info(char *key_word, int timeout, char *info,int info_size);
int a_modem_wait_msg(char *buf,int size);

int a_modem_play(char * filename);
int a_modem_play_smart(char * filename,int mili_sec);
int a_modem_record(int duration_mili);

int a_modem_prob(); // get atxn atrn info

int a_modem_status(); // get status (internal temp, pwr cond...) fill struct a_modem
void a_modem_status_show();

int a_modem_print_configs(char * filepath); // save cfg all output for future ref
int a_modem_set_deploy_configs();
int a_modem_set_devel_configs(); // set the preferable configs for devel stage, tx pwr...

int a_modem_sync_clock_gps();
int a_modem_sync_time_gps();
int a_modem_is_clock_Sync(int samp_interval, int N_retry);

int a_modem_upload_file(const char *fname);
int a_modem_msg_send(const char*msg);

int a_modem_slave();
int a_modem_master();

inline int a_modem_gets(char* buf,int size);
inline int a_modem_puts(const char*msg);
inline void a_modem_clear_io_buffer();
