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
char* ret=NULL;
//lock
pthread_mutex_lock(&msg->msg_mutex);

//init check
if (msg->N_unread>0) {
//get pointer that point output string
ret = msg->text[(msg->i-msg->N_unread+1+LIST_SIZE)%LIST_SIZE];
// N_unread update
msg->N_unread--;
}
//return
pthread_mutex_unlock(&msg->msg_mutex);
return ret;
}

int amodem_msg_push(amodem_msg *msg ,char *msg_str){
//lock
pthread_mutex_lock(&msg->msg_mutex);
//put
memset((void *)msg->text[msg->i],0,sizeof(msg->text[msg->i]));
strcpy(msg->text[msg->i],msg_str);
// N_unread i update
if (msg->N_unread>0) msg->i=(msg->i+1)%LIST_SIZE;
if (msg->N_unread>=LIST_SIZE) printf("%s,input msg list overrun\n",__func__);
else msg->N_unread++;
//return
pthread_mutex_unlock(&msg->msg_mutex);
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
		//printf("test\n");
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


int amodem_wait_ack(amodem_msg* msg,char * keyword,int mSec){
char *str;
str=amodem_wait_msg(msg,keyword,mSec,NULL,0);
if (str==NULL) {
printf("%s,ack timeout\n",__func__);
return FAIL;
}else
return SUCCESS;

}

int amodem_puts_remote(int addr,const char*msg){

/* write msg acoustically to remote modems*/
// go to online mode
char buf[36];
//addr
sprintf(buf,"@remoteaddr=%d",addr);
amodem_puts_local(buf);

amodem_mode_select('o',3);
// send msg
amodem_puts_local(msg);
amodem_wait_ack(&msg_local,"Forwarding",2000);
// go back to command mode
amodem_mode_select('c',3);
return SUCCESS;
}

int amodem_puts_local(const char*msg){
        // write a line to serial port
	return RS232_SendBuf(modem.fd,msg,strlen(msg));
}
