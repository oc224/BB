#include <stdio.h>
#include <math.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))
int wav2CIR(const char* rx,const char *tx){
wav *wav_rx,*wav_tx;
int M;//length of filter
int L;//nearest of 2 power of M
int N;// 2L
int Nx;
fftw_complex *H;
fftw_plan *p,* p_cross;
wav_tx=wav_open(tx);
wav_rx=wav_open(rx);

M=wav_tx->length;
L=pow(2,ceil(log(M)/log(2)));
N=2*L;
Nx=wav_rx->length;
//Nx=L;
printf("kernel length : %d\n",M);
printf("L = %d\n",L);
int i;
int il;
int k;
// fft tx
H = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
wav_read(wav_tx,H,N);
p = fftw_plan_dft_1d(N,H,H,FFTW_FORWARD,FFTW_MEASURE);
fftw_excute(p);
X = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
p = fftw_plan_dft_1d(N,X,X,FFTW_FORWARD,FFTW_MEASURE);
p_cross = fftw_plan_dft_1d(N,cross,cross,FFTW_FORWARD,FFT_MEASURE);
cross = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
while (i<Nx){
il = min(i+L-1,Nx)
wav_read(wav_rx,X,L);
fftw_excute(p);//fft x
// cross =  X * H
fftw_excute(p_cross);//ifft cross
// ifft (cross)
k = min(i+N-1,M+Nx-1);

i = i+L;
}

}
