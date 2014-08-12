#include <stdio.h>

#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"

int main() {
/*
int a_modem_init(); ok
int a_modem_open(); ok
inline void a_modem_close();ok
void a_modem_msg_show(a_modem_msg *);ok
int a_modem_msg_add(a_modem_msg*,char *msg_str);ok

int a_modem_wait_ack(char*,int);
int a_modem_wait_info(char *key_word, int timeout, $
int a_modem_wait_remote(char*,int,int);

int a_modem_play(char * filename);
int a_modem_record(int duration_mili);


int a_modem_status(); // get status (internal temp,$
void a_modem_status_show();

int a_modem_print_configs(char * filepath); // save$
int a_modem_set_deploy_configs();
int a_modem_set_devel_configs(); // set the prefera$

int a_modem_sync_clock_gps();
int a_modem_sync_time_gps();
int a_modem_is_clock_Sync(int samp_interval, int N_$
int a_modem_sync_status();

int a_modem_upload_file(const char *fname);
int a_modem_msg_send(const char*msg);


*/
	system_cfg_read();
	system_cfg_show();
	printf("!!acoustic modem check\n");
	a_modem_init();

	a_modem_msg_show(&msg);
	a_modem_msg_show(&msg_remote);

	// open
	printf("!open\n");
	a_modem_open();
	printf("\n");

	//clear ffs
//	a_modem_ffs_clear();

	//wait ack
	a_modem_clear_io_buffer();
	printf("!wait timeout\n");
	a_modem_puts("at\r");
	a_modem_wait_ack("OK", 2000);
	printf("Should not be blocking..\n");
	printf("\n");

	// play
	printf("!play wavform\n");
	a_modem_play("lfm_data_t3_l1.wav");
	printf("should hear sound\n");
	printf("\n");

	//wait ack
	a_modem_clear_io_buffer();
	printf("!wait timeout\n");
	RS232_SendBuf(a_modem_dev_no, "at\r", 3);
	if (a_modem_wait_ack("OK", 2000) == FAIL)
		printf("no response\n");
	else
		printf("got response\n");
	printf("Should got response\n");
	printf("\n");

	//record
	printf("!record\n");
	a_modem_record(1000);
	printf("check RX\n");
	printf("\n");
	a_modem_msg_show(&msg);

	// set devel configs
	printf("!set devel configs\n");
	a_modem_set_devel_configs();
	printf("wait ack\n");
	printf("\n");

// get status
	printf("!status get\n");
	a_modem_status();
	printf("\n");

// show status
	printf("!show status\n");
	a_modem_status_show();
	printf("\n");

//show msg list
	printf("!show msg list\n");
	a_modem_msg_show(&msg);
	printf("\n");
// sync gps
	/*printf("!sync gps\n");
	 a_modem_sync_gps();
	 printf("\n");*/

	/*// is sync
	 printf("!sync test\n");
	 if (a_modem_is_clock_Sync(10,3))printf("sync.\n");
	 else printf("not sync\n");
	 printf("\n");*/
	// close

	/*	//upload file
		printf("!upload file\n");
		a_modem_upload_file("25080801.WAV");
		printf("\n");*/
a_modem_close();
return 0;

}
