#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "gps.h"
#include "system.h"

#define GPSPIPE_TPV "gpspipe -w -n 10|grep TPV"
#define GPSPIPE_SKY "gpspipe -w -n 10|grep SKY"
#define GPSPIPE_GPRMC "gpspipe -r -n 15|grep GPRMC"
#define GPSPIPE_GPGGA "gpspipe -r -n 15|grep GPGGA"
#define BUFSIZE 256

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
