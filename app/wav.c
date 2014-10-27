#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "common.h"
#include "NE10.h"
#include "wav.h"
#define FP t_wav->fp

wav* wav_open(const char *fname){
/*open the wav file and read header*/
int i;
uint32_t fs;
uint32_t size;
uint16_t bits;
uint16_t N_ch;
uint16_t blk;
long length;
wav * t_wav=(wav *)malloc(sizeof(wav));
/*open*/
if ((FP=fopen(fname,"rb"))==NULL){
fprintf(stderr,"%s, fail to open %s file\n",fname,__func__);
return NULL;
}
/*read header*/
fseek(FP,22,SEEK_SET);
fread(&N_ch,2,1,FP);/*channels*/
fread(&fs,4,1,FP);/*fs*/
fseek(FP,32,SEEK_SET);
fread(&blk,2,1,FP);/*block align*/
fread(&bits,2,1,FP);/*bits per sample*/
fseek(FP,40,SEEK_SET);
fread(&size,4,1,FP);
t_wav->fs=fs;
t_wav->N_ch=N_ch;
t_wav->bits=bits;
t_wav->blk_align=blk;
if (size%blk)printf("wav length error\n");
t_wav->length=size/blk;
/*end*/
fseek(FP,44,SEEK_SET);
return t_wav;
}

void wav_show(wav* t_wav){
printf("fs = %d\n",t_wav->fs);
printf("N ch = %d\n",t_wav->N_ch);
printf("bits = %d\n",t_wav->bits);
printf("block align = %d\n",t_wav->blk_align);
printf("length = %ld \n",t_wav->length);
}

int wav_read(wav* t_wav,ne10_fft_cpx_float32_t *dest,int L,int order){
//read N samples to ne10_fft_cpx_float32_t
int i;
int16_t data[2];
switch (order){
case NORMAL:
for (i=0;i<L;i++){
//read file
if (fread(data,2,2,FP)<2){
printf("%s error\n",__func__);
return FAIL;
break;
}
//store
dest[i].r=(ne10_float32_t)data[0];
dest[i].i=(ne10_float32_t)data[1];
}
//return
return i+1;
break;

case REVERSE:
for (i=L-1;i>-1;i--){
//read file
if (fread(data,2,2,FP)<2){
printf("%s error\n",__func__);
return FAIL;
break;
}
//store
dest[i].r=(ne10_float32_t)data[0];
dest[i].i=(ne10_float32_t)data[1];
}
//return
return L-i;
break;

default:
return FAIL;
break;
}
return FAIL;
}

int wav_close(wav* t_wav){
fclose(FP);
free(t_wav);
return 0;
}

int data_st(FILE *fp,void* ptr,int size,int N){
return fwrite(ptr,size,N,fp);
}
