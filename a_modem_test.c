#include <stdio.h>

#include "acoustic_modem.h"
#include "rs232.h"
#include "system.h"

int main() {

//	inline int a_modem_wait_ack(char *ack_msg,int timeout_mili);//ok
//	inline int a_modem_wait_info(char *key_word,int timeout,char *info,int info_size);//ok
//	inline void a_modem_clear_io_buffer();
//	inline void a_modem_close();//ok
//	int a_modem_is_clock_Sync(int samp_interval,int N_retry);//ok
//	int a_modem_ls();// list files
//	int a_modem_open();//ok
//	int a_modem_play(char * filename);//ok
//	int a_modem_print_configs(char * filepath);// save cfg all output for future ref
//	int a_modem_prob();// get atxn atrn info
//	int a_modem_record(char * timestamp,int duration_mili);//ok
//	int a_modem_set_deploy_configs();
//	int a_modem_set_devel_configs();// set the preferable configs for devel stage, tx pwr...//ok
//	int a_modem_status();//ok
//	void a_modem_status_show();//ok
	printf("!!acoustic modem check\n");
	a_modem_init();


	// open
	printf("!open\n");
	a_modem_open();
	printf("\n");


	// close
	printf("!close\n");
	a_modem_close();
	printf("\n");
	a_modem_open();


	// play
	printf("!play wavform\n");
	a_modem_play("lfm_data_t1_l1.wav");
	printf("should hear sound\n");
	printf("\n");

//wait ack
	a_modem_clear_io_buffer();
	printf("!wait timeout\n");
	a_modem_puts("at\r");
	a_modem_wait_ack("OK", 2000);
	printf("Should not be blocking..\n");
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
	a_modem_msg_show();

	// set devel configs
	printf("!set devel configs\n");
	a_modem_set_devel_configs();
	printf("wait ack\n");
	printf("\n");




// is clock sync
/*	printf("!sync test\n");
	if (a_modem_is_clock_Sync(10, 3))
		printf("sync.\n");
	else
		printf("not sync\n");
	printf("\n");*/

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
	a_modem_msg_show();
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
