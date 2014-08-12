#include <stdio.h>
#include <fftw3.h>
typedef struct{
int fs;
int N_ch;
int bits;
int blk_align;
long length;
FILE *fp;
}wav;


int double_write(FILE *,fftw_complex*,int);

