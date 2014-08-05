#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fftw3.h>
#include "wav.h"
#define FP t_wav->fp
#define N 1200
wav* wav_open(const char *fname){
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
fprintf(stderr,"fail to open .wav file\n");
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

int wav_read(wav* t_wav,fftw_complex *dest,int L){
//read at most N samples to fftw_complex
int i;
int16_t data[2];

for (i=0;i<L;i++){
if (fread(data,2,2,FP)<2)break;
dest[i][0]=(double)data[0];dest[i][1]=(double)data[1];
printf("%d %4.4f %4.4f \n",i,dest[i][0],dest[i][1]);
}

return i;
}
int wav_write(wav* t_wav,fftw_complex* out,int N){

}

int wav_close(wav* t_wav){
fclose(FP);
free(t_wav);
return 0;
}

int main(){
fftw_complex *in;
in=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*N);

wav * t_wav=(wav*)wav_open("T1_raw.wav");

wav_read(t_wav,in,N);
wav_show(t_wav);
wav_close(t_wav);
return 0;
}
