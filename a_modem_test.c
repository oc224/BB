#include <stdio.h>

#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"

int main() {
/*
int amodem_init(); ok
int amodem_open(); ok
inline void amodem_close();ok
void amodem_msg_show(amodem_msg *);ok
int amodem_msg_add(amodem_msg*,char *msg_str);ok

int amodem_wait_ack(char*,int);
int amodem_wait_info(char *key_word, int timeout, $
int amodem_wait_remote(char*,int,int);

int amodem_play(char * filename);
int amodem_record(int duration_mili);


int amodem_status(); // get status (internal temp,$
void amodem_status_show();

int amodem_print_configs(char * filepath); // save$
int amodem_set_deploy_configs();
int amodem_set_devel_configs(); // set the prefera$

int amodem_sync_clock_gps();
int amodem_sync_time_gps();
int amodem_is_clock_Sync(int samp_interval, int N_$
int amodem_sync_status();

int amodem_upload_file(const char *fname);
int amodem_msg_send(const char*msg);


*/
	system_cfg_read();
	system_cfg_show();
	printf("!!acoustic modem check\n");
	amodem_init();

	amodem_msg_show(&msg);
	amodem_msg_show(&msg_remote);

	// open
	printf("!open\n");
	amodem_open();
	printf("\n");

	//clear ffs
//	amodem_ffs_clear();

	//wait ack
	amodem_clear_io_buffer();
	printf("!wait timeout\n");
	amodem_puts("at\r");
	amodem_wait_ack("OK", 2000);
	printf("Should not be blocking..\n");
	printf("\n");

	// play
	printf("!play wavform\n");
	amodem_play("lfm_data_t3_l1.wav");
	printf("should hear sound\n");
	printf("\n");

	//wait ack
	amodem_clear_io_buffer();
	printf("!wait timeout\n");
	RS232_SendBuf(amodem_dev_no, "at\r", 3);
	if (amodem_wait_ack("OK", 2000) == FAIL)
		printf("no response\n");
	else
		printf("got response\n");
	printf("Should got response\n");
	printf("\n");

	//record
	printf("!record\n");
	amodem_record(1000);
	printf("check RX\n");
	printf("\n");
	amodem_msg_show(&msg);

	// set devel configs
	printf("!set devel configs\n");
	amodem_cfg_devel();
	printf("wait ack\n");
	printf("\n");

// get status
	printf("!status get\n");
	amodem_status();
	printf("\n");

// show status
	printf("!show status\n");
	amodem_status_show();
	printf("\n");

//show msg list
	printf("!show msg list\n");
	amodem_msg_show(&msg);
	printf("\n");
// sync gps
	/*printf("!sync gps\n");
	 amodem_sync_gps();
	 printf("\n");*/

	/*// is sync
	 printf("!sync test\n");
	 if (amodem_is_clock_Sync(10,3))printf("sync.\n");
	 else printf("not sync\n");
	 printf("\n");*/
	// close

	/*	//upload file
		printf("!upload file\n");
		amodem_upload_file("25080801.WAV");
		printf("\n");*/
amodem_close();
return 0;

}
