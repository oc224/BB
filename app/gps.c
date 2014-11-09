#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "gps.h"
#include "common.h"
#include "rs232.h"
#define GPSPIPE_TPV "gpspipe -w -n 10|grep TPV"
#define GPSPIPE_SKY "gpspipe -w -n 10|grep SKY"
#define GPSPIPE_GPRMC "gpspipe -r -n 15|grep GPRMC"
#define GPSPIPE_GPGGA "gpspipe -r -n 15|grep GPGGA"
#define BUFSIZE 256
#define BAUDRATE_GPS 4800
#define SIZE_SENTENCE 200

gps_info gps;


int gps_update(){
	/*get info from gps*/
    char buf[BUFSIZE],buf2[BUFSIZE];
    FILE *pp;

/*GPRMC*/
char *token;
if ((pp=popen(GPSPIPE_GPRMC,"r"))==NULL){
fprintf(stderr,"popen error\n");
return FAIL;}
buf[0]=0;
fgets(buf,BUFSIZE,pp);
printf("%s\n",buf);
token=strtok(buf,",");//GPRMC

token=strtok(NULL,",");//hhmmss
sscanf(token,"%2d%2d%2d",&gps.hh,&gps.mm,&gps.ss);
sprintf(buf2,"date -s %d:%d:%d",gps.hh,gps.mm,gps.ss);
system(buf2);

token=strtok(NULL,",");//A
token=strtok(NULL,",");//2501XX
sscanf(token,"%f",&gps.lat);
token=strtok(NULL,",");//N
token=strtok(NULL,",");//121
sscanf(token,"%f",&gps.lon);
token=strtok(NULL,",");//E
token=strtok(NULL,",");//xxx
sscanf(token,"%f",&gps.knot);
token=strtok(NULL,",");//xxx

token=strtok(NULL,",");//date
sscanf(token,"%2d%2d%2d",&gps.day,&gps.month,&gps.year);
gps.year=gps.year+2000;
sprintf(buf2,"date -s \"%4d-%02d-%02d %02d:%02d\"",gps.year,gps.month,gps.day,gps.hh,gps.mm);
printf("debug %s\n",buf2);
system(buf);

pclose(pp);

/*GPGGA*/
if ((pp=popen(GPSPIPE_GPGGA,"r"))==NULL){
fprintf(stderr,"popen error\n");
return FAIL;}
buf[0]=0;
fgets(buf,BUFSIZE,pp);
printf("%s\n",buf);
token=strtok(buf,",");//GPGGA
token=strtok(NULL,",");//hhmmss
token=strtok(NULL,",");//2501
token=strtok(NULL,",");//N
token=strtok(NULL,",");//121
token=strtok(NULL,",");//E
token=strtok(NULL,",");//1
token=strtok(NULL,",");//11
sscanf(token,"%d",&gps.no_sat);
pclose(pp);

return 0;
}

void gps_show(){
    //show
	printf("lat : %f\n",gps.lat);
	printf("lon : %f\n",gps.lon);
	printf("Nsat : %d\n",gps.no_sat);
	printf("speed : %f\n",gps.knot);
	printf("date : %d-%d-%d\n",gps.year,gps.month,gps.day);
	printf("time : %d:%d:%d\n",gps.hh,gps.mm,gps.ss);
}

void gps_log(){
system("gpspipe -r -n 15|grep GP >> /home/root/log/GPS.TXT");
}

int gps_dump_modem(const char*dev_gps,const char*sentence,char *buf,int N){
int fd;
char sen[SIZE_SENTENCE];
//open
if((fd = RS232_OpenComport(dev_gps, BAUDRATE_GPS))==OPEN_ERROR){
fprintf(stderr,"%s, fail to open %s\n",__func__,dev_gps);
return FAIL;}

//read key sentence
do{
while (RS232_PollComport(fd,sen,SIZE_SENTENCE)<1) usleep(1000);//wait sentence
}while (strstr(sen,sentence)==NULL);
strcpy(buf,sen);

//return sentence
RS232_CloseComport(fd);
printf("get %s\n",sen);
return SUCCESS;
}

int gps_PortSelect(){
int fd;
char dev_gps[20];
int i_PortProb;
int i_PortConfirm = -1;
int isString;
char buf[SIZE_SENTENCE];

for (i_PortProb=0;i_PortProb<4;i_PortProb++){

//open
sprintf(dev_gps,"/dev/ttyUSB%d",i_PortProb);
if((fd = RS232_OpenComport(dev_gps, BAUDRATE_GPS))==OPEN_ERROR){
fprintf(stderr,"%s, fail to open %s\n",__func__,dev_gps);
continue;}

//read string
sleep(3);
isString = (RS232_PollComport(fd,buf,SIZE_SENTENCE-1)>0);
RS232_CloseComport(fd);

//is gps
if ((isString)&&(strstr(buf,"GPGGA")!=NULL)) {
i_PortConfirm = i_PortProb;
break;}

}
//return
printf("gps on port %d\n",i_PortConfirm);
return i_PortConfirm;
}
