#include <stdio.h>
#include "acoustic_modem.h"

int main(){
int n;
printf("acoustic modem check\n");
// open
printf("!open\n");
a_modem_open();
printf("\n");

//wait ack
a_modem_clear_FIFO();
printf("!wait timeout\n");
a_modem_wait_ack("OK",2000);
printf("Should not be blocking..\n");
printf("\n");

// set devel configs
printf("set devel configs\n");
a_modem_set_devel_configs();
printf("wait ack\n");
n=a_modem_wait_ack("config",1000);
printf("ack=%d, should be 1\n",n);
printf("\n");

// play
printf("play wavform\n");
a_modem_play("lfm_data_t3_l1.wav");
printf("should hear sound\n");
printf("\n");

// is sync
printf("sync test");
a_modem_Is_Sync(10,3);
printf("\n");

// sync gps
printf("sync gps\n");
a_modem_sync_gps();
printf("\n");
// close
a_modem_close();
return 0;
}
