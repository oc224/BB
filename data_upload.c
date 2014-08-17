/*
 * data_upload.c
 *
 *  Created on: Jul 29, 2014
 *      Author: root
 */
#include "acoustic_modem.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 32
int main(int argc,char *argv[]){
	FILE *fp;
	char buf[BUFSIZE];
	/* input check */
	if (argc<1){
		printf("please provide file list text file.\n");
		return -1;
	}
	a_modem_open();

	fp=fopen(argv[1],"r");
	if(fp==NULL){
		printf("fail to open file list file \n");
		return -1;
	}

	while (fgets(buf,BUFSIZE,fp)!= NULL){
		if (strlen(buf)<3)continue;
		buf[strlen(buf)-1]=0;//erase \n char
		if ((strstr(buf,"log")==NULL)|(strstr(buf,"wav")==NULL)){
			printf("invalid file name\n");
		}
		printf("uploading file : %s\n",buf);
		a_modem_upload_file(buf);
		printf("done \n");
		sleep(1);
	}
	a_modem_close();
	return 0;
}


