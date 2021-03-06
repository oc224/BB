#ifndef AMODEM_H
#define AMODEM_H
#include <stdio.h>
#include <pthread.h>
#include "log.h"
#include "common.h"
#define amodem_serial_baudrate 115200
#define TIMEOUT_SERIAL 2000/*default timeout for reading modem*/
#define TIMEOUT_SYNC 15
#define TIMEOUT_COPY 10000
#define GPSPIPE "gpspipe -r -n 20 |grep 'GPGGA' >> /dev/ttyUSB2" /*feed modem gps GPGGA setence.*/
#define DELAY_BEFORE_MODE_SWAP 1
#define DELAY_MODE_TRANS 1
#define DELAY_AFTER_MODE_SWAP 1
#define DELAY_MODE_SELECT (DELAY_BEFORE_MODE_SWAP+DELAY_AFTER_MODE_SWAP+DELAY_MODE_TRANS)
#define WAIT_INTVAL 100000 /*inteval time for reading serial port*/
#define N_ITER_DIV (WAIT_INTVAL/1000)
#define GPSPIPE_TIME 8 /*seconds that gpspipe feed modem*/
#define LIST_SIZE 16
#define TX_SIZE 32
#define SYNC_FALSE 0
#define SYNC_TRUE 1
typedef enum {
	NOT_SYNC, QUALIFY, SYNC
} amodem_sync_state;

typedef enum{
ONLINE,COMMAND
}amodem_mode;


typedef struct {
	int fd;
	char dev_path[20];
	pthread_mutex_t readthread;//=PTHREAD_MUTEX_INITIALIZER;
	float board_temp; //ref modem manual, atv
	float dsp_bat;
	float mdm_bat; // ref modem manual mdm_battery
	float rtc_bat; // ref modem manual rtc_battery
	amodem_sync_state sync_state;
	amodem_mode mode;
	char latest_tx_stamp[TX_SIZE];
	char latest_rx_fname[TX_SIZE];
	char def_tx_wav[TX_SIZE];
	FILE *tx_p;
	FILE *rx_p;
	logger *com_logger;
} amodem;


typedef struct{
	char text[LIST_SIZE][80];
	int i;/*point to latest msg aka text[i] is latest msg*/
	/*New text[i]>text[i-1]>...>text[i-N_unread+1]*/
	int N_unread;/*number of unread msg*/
	int N;
	//msg *head, *tail;
	pthread_mutex_t msg_mutex;// = PTHREAD_MUTEX_INITIALIZER;
	//pthread_mutex_t msg_write;
}amodem_msg;

extern amodem modem;
extern amodem_msg msg_local;
extern amodem_msg msg_remote;

int amodem_init(char *);/*init amodem*/
int amodem_open();/*open the serial port*/
inline void amodem_close();/*close the serial port*/
int amodem_mode_select(char,int);
int amodem_end();

/*low level io*/
int amodem_msg_push(amodem_msg *msg_list ,char *msg_str);
char* amodem_msg_pop(amodem_msg* msg);
char* amodem_wait_msg(amodem_msg *msg,char *key_word, int mSec, char *info,int info_size);
void amodem_msg_show(amodem_msg *);/*show msg list*/

/*io*/
#define amodem_wait_local(key_word,mSec,buf,bufsize) amodem_wait_msg(&msg_local,key_word,mSec,buf,bufsize)
#define amodem_wait_remote(key_word,mSec,buf,bufsize) amodem_wait_msg(&msg_remote,key_word,mSec,buf,bufsize)
int amodem_wait_ack(amodem_msg* msg,char* keyword,int mSec);
int amodem_puts_local(const char*msg);
int amodem_puts_remote(int addr,const char*msg);/*send msg to remote*/
void amodem_print(int);/*show msg from remote*/

/*exp*/
int amodem_play(char * filename);
int amodem_record(int duration_mili);
int amodem_upload_file(const char *fname);/*upload a file in /sd on the modem*/

/*status*/
int amodem_status();
void amodem_status_show();

/*config*/
int amodem_cfg_set(const char *);
#define amodem_cfg_deploy() amodem_cfg_set(CFG_DEPLOY);
#define amodem_cfg_devel() amodem_cfg_set(CFG_DEVEL);

/*sync*/
int amodem_sync_clock_gps(int);
int amodem_sync_time_gps();
int amodem_sync_status();

#endif
