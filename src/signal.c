#include <stdio.h>
#include <math.h>
#include "wav.h"
#define min(a,b) (((a) < (b)) ? (a) : (b))
int fftw_complex_multiply(fftw_complex *out,fftw_complex* in1,fftw_complex * in2,int N){
int i;
for (i=0;i<N;i++){
out[i][0] = in1[i][0]*in2[i][0]-in1[i][1]*in2[i][1];
out[i][1] = in1[i][0]*in2[i][1]+in1[i][1]*in2[i][0]
}
return 0;
}
int result_puts(fftw_complex *out,int N){
FILE *fp;
int i;
if ((fp=fopen("out.txt","a"))==NULL){
fprintf(stderr,"fail to open output file\n");
return 0;
}
for (i=0;i<N;i++)fprintf(fp,"%f %f\n",out[i][0],out[i][1]);

fclose(fp);
}

int wav2CIR(const char* rx,const char *tx){
/*allocate resources*/
wav *wav_rx,*wav_tx;
int M;//length of filter
int L;//nearest of 2 power of M
int N;// 2L
int Nx;
fftw_complex *H;
fftw_plan *p,* p_cross;
wav_tx=wav_open(tx);
wav_rx=wav_open(rx);

/*some pare*/
M=wav_tx->length;
L=pow(2,ceil(log(M)/log(2)));
N=2*L;
Nx=wav_rx->length;
printf("kernel length : %d\n",M);
printf("L = %d\n",L);

int i=0;
int k;
/* fft tx*/
H = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
wav_read(wav_tx,H,N);
p = fftw_plan_dft_1d(N,H,H,FFTW_FORWARD,FFTW_MEASURE);
fftw_excute(p);
/* fft rx I*/
X = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
p = fftw_plan_dft_1d(N,X,X,FFTW_FORWARD,FFTW_MEASURE);
/*ifft cross*/
p_cross = fftw_plan_dft_1d(N,cross,cross,FFTW_BACKWORD,FFT_MEASURE);
cross = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
while (i<Nx){
/*fft rx II*/
wav_read(wav_rx,X,L);
fftw_excute(p);
/*ifft (cross) = ifft ( X * H )*/
fftw_complex_multiply(cross,X,H,L);
fftw_excute(p_cross);
result_puts(upper half p_cross+buf)
//buf = lower half 

k = min(i+N-1,M+Nx-1);

i = i+L;
}
}
