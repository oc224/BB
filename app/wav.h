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

//open the wav file and read header
//	input : path to the wav file.
//	output : wav data structure
wav* wav_open(const char *);

//show wav data structure
//	input : wav structure
void wav_show(wav*);

//read samples of .wav file (Benthos modem, 2 channel 16bits)
//	input : wav structure
//		output pointer
//		output size
//		mode
//			NORMAL ORDER x[0] x[1] x[2]...
//			REVERSE ORDER x[N-1] x[N-2] x[N-3]...
int wav_read(wav*,ne10_fft_cpx_float32_t*,int,int);

//wav close
//	input : wav struture
int wav_close(wav*);

//store data
// input : 	fp file file pointer
//		ptr pointer to data going to store
//		size the size of basic unit of the data
//		N numbers of data
int data_st(FILE *fp,void* ptr,int size,int N);
