#include <stdio.h>
#include <fftw3.h>
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

int double_write(FILE *,fftw_complex*,int);
wav* wav_open(const char *);
void wav_show(wav*);
int wav_read(wav*,fftw_complex*,int,int);
int double_write_file(FILE*,fftw_complex*,int);
int double_write_file(FILE*,fftw_complex*,int);
int wav_close(wav*);
