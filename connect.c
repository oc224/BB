/*
 * connect.c
 *
 *  Created on: Jul 29, 2014
 *      Author: root
 */

#include "wireless_modem.h"
#include "system.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define BUFSIZE 200
#define PPP_TIMEOUT 5
#define NET_PATH "/proc/net/route"

int ppp_wait(int timeout){
	FILE *fp;
	char buf[BUFSIZE];
	int i=0;

	while (i<timeout){
		/*check if ppp exist*/
		fp=fopen(NET_PATH,"r");
		if(fp==NULL){
			fprintf(stderr,"fail to open net file\n");
			return FAIL;
		}
		while(fgets(buf,BUFSIZE,fp)!=NULL){
			if (strstr(buf,"ppp")!=NULL)return SUCCESS;
		}
		fclose(fp);

		sleep(1);
		i++;
	}
return FAIL;
}
int ppp(const char *dest_ip){
	char buf[BUFSIZE];
	sprintf(buf,"pppd /dev/ttyUSB1 115200 -crtscts local debug passive persist 10.0.0.10:%s",dest_ip);
	system(buf);
	return SUCCESS;
}

int main(int argc,char *argv[]){
	node dest_node;
	if (argc<2){
		fprintf(stderr,"please provide dest name\n");
		return FAIL;
	}

	system_node_lookup(argv[1],&dest_node);
	w_modem_open();
	w_modem_connect(dest_node.w_add);
	w_modem_close();
	ppp(dest_node.ppp_ip);
	if (ppp_wait(PPP_TIMEOUT)==FAIL){
		fprintf(stderr,"ppp timeout\n");
		return FAIL;
	}
	return 0;
}
