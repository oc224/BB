/*
 * data_upload.c
 *
 *  Created on: Jul 29, 2014
 *      Author: root
 */
#include "master.h"
#include "amodem.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 32
int main(int argc,char *argv[]){
	FILE *fp;
	char buf[BUFSIZE];
	char fname[BUFSIZE];
	/* input check */
	if (argc<2){
		printf("please provide file list text file.\n");
		return -1;
	}
	amodem_init(argv[2]);
	amodem_open();

	fp=fopen(argv[1],"r");
	if(fp==NULL){
		printf("fail to open file list file \n");
		return -1;
	}

	while (fgets(fname,BUFSIZE,fp)!= NULL){
		if (strlen(fname)<3)continue;
		fname[strlen(fname)-1]=0;//erase \n char
/*		if ((strstr(buf,"log")==NULL)|(strstr(buf,"wav")==NULL)){
			printf("invalid file name\n");
		}*/
		sprintf(buf,"%s.log",fname);
		printf("uploading file : %s\n",buf);
		amodem_upload_file(buf);

		sprintf(buf,"%s.wav",fname);
		printf("uploading file : %s\n",buf);
		amodem_upload_file(buf);


		printf("done \n");
		sleep(1);
	}
	amodem_close();
	return 0;
}


