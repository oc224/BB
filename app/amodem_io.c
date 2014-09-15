#define _GNU_SOURCE
#include "amodem.h"
#include "rs232.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define BUFSIZE 100/*default size for buffer*/

char* amodem_msg_pop(amodem_msg* msg){
/*point to oldest unread msg*/
char* ret;
//init check
if (msg->N_unread==0) return NULL;
//get pointer that point output string
ret = msg->text[(msg->i-msg->N_unread+1+LIST_SIZE)%LIST_SIZE];
// N_unread update
msg->N_unread--;
return ret;
}

int amodem_msg_push(amodem_msg *msg ,char *msg_str){
//put
free(msg->text[msg->i]);
msg->text[msg->i]=strdup(msg_str);

// N_unread i update
msg->i=(msg->i+1)%LIST_SIZE;
if (msg->N_unread>=LIST_SIZE){
printf("input msg list overrun\n");
}else{
msg->N_unread++;
}
return SUCCESS;
}


char* amodem_wait_msg(amodem_msg *msg,char *key_word, int mSec, char *info,
                int info_size) {
        /*block until key_word prompt or timout(miliseconds), if key_word prompt, 
        store that line contained key_word to info, the output info is a string
        (end with a NULL char)
        */

        char *new_msg;
        int delay=0;
        int Niter=mSec/100;
        int is_copy=(info!=NULL);
        int is_nullkeyword=(key_word==NULL);
        char* ret=NULL;

        if (is_copy) info[0]=0;/*make sure input buffer clear when this funtion fail*/

        while(delay<Niter) {//before timeout
                usleep(100000);
                delay++;
                if ((new_msg=amodem_msg_pop(msg))!=NULL) {//got new msg
                        if ((is_nullkeyword)||(strcasestr(new_msg,key_word)!=NULL)) {
                        ret=new_msg;
                        break;  }
                }
        }
	//return
        if ((ret!=NULL)&&(is_copy)) strncpy(info,new_msg,info_size);
        return ret;
}

int amodem_puts_remote(const char*msg){

/* write msg acoustically to remote modems*/
// go to online mode
amodem_mode_select('o');
// send msg
amodem_puts(msg);
// go back to command mode
amodem_mode_select('c');
	/*amodem_puts("ato\r");
        if (amodem_wait_local_ack("connect",2*TIMEOUT_SERIAL)==NULL){
                fprintf(stderr,"msg_send, fail to enter online mode\n");
                return FAIL;
        }
        usleep(DELAY_ATO);
        if (amodem_wait_local_ack("forwarding",2*TIMEOUT_SERIAL)==NULL){
                fprintf(stderr,"msg_send, fail to forward msg\n");
                return FAIL;
        }
        usleep(ONLINE_COMMAND);

        amodem_puts("+++\r");
        usleep(COMMAND_DELAY);*/
        return SUCCESS;
}

int amodem_puts(const char*msg){
        // write a line to serial port
        msg_local.N_unread=0;
        msg_remote.N_unread=0;
        return RS232_SendBuf(modem.fd,msg,strlen(msg));
}
