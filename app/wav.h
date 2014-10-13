#include <stdio.h>
#include "NE10.h"
#define NORMAL 1
#define REVERSE -1
typedef struct{
int fs;
int N_ch;
int bits;
int blk_align;
long length;
FILE *fp;
}wav;

wav* wav_open(const char *);
void wav_show(wav*);
int wav_read(wav*,ne10_fft_cpx_float32_t*,int,int);
int wav_close(wav*);
int data_st(FILE *fp,void* ptr,int size,int N);
