#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "gps.h"
#include "system.h"

#define GPSPIPE_TPV "gpspipe -w -u -n 10|grep TPV"
#define GPSPIPE_SKY "gpspipe -w -n 10|grep SKY"
#define GPSPIPE_GPRMC "gpspipe -r -n 10|grep GPRMC"
#define BUFSIZE 256

gps_info gps;


int gps_update(){
	/*get info from gps*/
    char buf[BUFSIZE];
    FILE *pp;

/*get TPV msg*/
/*
2014-08-10 13:07:43.402061: {"class":"TPV","tag":"RMC","device":"/dev
/pts/12","mode":3,"time":"2004-12-14T10:54:10.548Z","ept":0.005,
"lat":59.343471667,"lon":18.056260000,"alt":31.500,
"epx":38.327,"epy":49.776,"epv":46.000,"track":139.3300,
"speed":1.697,"climb":-0.700,"eps":99.55}
*/
    if( (pp = popen(GPSPIPE_TPV, "r")) == NULL )
    {
        printf("popen() error!\n");
        return FAIL;
    }
	fgets(buf,BUFSIZE,pp);
	pclose(pp);
/*parse msg*/
	sscanf(strstr(buf,"lat")+5,"%f[,]",&gps.lat);
	sscanf(strstr(buf,"lon")+5,"%f[,]",&gps.lon);
	sscanf(strstr(buf,"speed")+7,"%f[,]",&gps.knot);
	sscanf(strstr(buf,"alt")+5,"%f[,]",&gps.alt);
//	sscanf(strstr(buf,"time")+7,"%d-%d-%dT%d:%d:%f[^0-9]",&gps.year,&gps.month,&gps.day,&gps.hh,&gps.mm,&gps.ss);
/*GPRMC*/
if ((pp=popen(GPSPIPE_GPRMC,"r"))==NULL){
fprintf(stderr,"popen error\n");
return FAIL;
}
/*SKY*/
/*    if( (pp = popen(GPSPIPE_SKY, "r")) == NULL )
    {
        printf("popen() error!\n");
        return FAIL;
    }
	fgets(buf,BUFSIZE,pp);
	pclose(pp);
	gps.no_sat=strcspn(strstr(buf,"satellite"),"{");*/


    return 0;
}

void gps_show(){
    //show
	printf("lat : %f\n",gps.lat);
	printf("lon : %f\n",gps.lon);
	printf("Nsat : %d\n",gps.no_sat);
	printf("speed : %f\n",gps.knot);
	printf("date : %d-%d-%d\n",gps.year,gps.month,gps.day);
	printf("time : %d:%d:%f\n",gps.hh,gps.mm,gps.ss);
}
