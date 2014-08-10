#include <stdio.h>
#include <math.h>
#include "wav.h"
#include <fftw3.h>
#include "signal.h"
//#define min(a,b) (((a) < (b)) ? (a) : (b))

int fftw_complex_clear(fftw_complex *in,int N){
int i;
for (i=0;i<N;i++){
in[i][0]=0;in[i][1]=0;
}
return 0;
}

int fftw_complex_multiply(fftw_complex *out,fftw_complex* in1,fftw_complex * in2,int N){
// output length N array out equal to complex multiplication of in1 and in2
int i;
for (i=0;i<N;i++){
out[i][0] = in1[i][0]*in2[i][0]-in1[i][1]*in2[i][1];
out[i][1] = in1[i][0]*in2[i][1]+in1[i][1]*in2[i][0];
}
return 0;
}

int fftw_complex_plus(fftw_complex *out,fftw_complex * in1,fftw_complex* in2,int N){
// complex addition
int i;
for (i=0;i<N;i++){
out[i][0]=in1[i][0]+in2[i][0];
out[i][1]=in1[i][1]+in2[i][1];
}
return 0;
}

void fftw_complex_show(fftw_complex *ptr,int N){
int i;
for (i=0;i<N;i++){
printf("[%d] %18.1f + %18.1f \n",i,ptr[i][0],ptr[i][1]);
}

}

int wav2CIR(const char* rx,const char *tx){
/*allocate resources*/
wav *wav_rx,*wav_tx;
int M;//length of filter
int L;//nearest of 2 power of M
int N;// 2L
int Nx;
int i=0;
int k;
FILE *fp_out;
fftw_complex *H,*X,*cross,*buck1,*buck2,*swap_tmp;
fftw_plan p,p_cross;
wav_tx=wav_open(tx);
wav_rx=wav_open(rx);

/**/
if ((fp_out=fopen("res","wb"))==NULL){
fprintf(stderr,"fail to open output file\n");
return -1;
}

/*some para*/
M=wav_tx->length;
L=pow(2,ceil(log(M)/log(2)));
N=2*L;
Nx=wav_rx->length;
printf("kernel length : %d\n",M);
printf("L = %d\n",L);
H = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
X = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
buck1 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
buck2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
fftw_complex_clear(buck1,N);
fftw_complex_clear(buck2,N);

/* fft tx */
p = fftw_plan_dft_1d(N,H,H,FFTW_FORWARD,FFTW_MEASURE);
fftw_complex_clear(H,N);
wav_read(wav_tx,H,M,REVERSE);
wav_close(wav_tx);
fftw_execute(p);

/* fft rx plan*/
p = fftw_plan_dft_1d(N,X,X,FFTW_FORWARD,FFTW_MEASURE);
/*ifft cross plan*/
p_cross = fftw_plan_dft_1d(N,buck1,buck1,FFTW_BACKWARD,FFTW_MEASURE);
i=0;
while (i<Nx){
/*fft rx II*/
fftw_complex_clear(X,N);
wav_read(wav_rx,X,M,NORMAL);
fftw_execute(p);

/*ifft (cross) = ifft ( X * H )*/
fftw_complex_multiply(buck1,X,H,N);
fftw_execute(p_cross);

//result_puts(upper half p_cross+buf)
fftw_complex_plus(buck2,buck1,buck2+L,L);
double_write_file(fp_out,buck2,L);

/*swap*/
swap_tmp=buck1;
buck1=buck2;
buck2=buck1;
p_cross = fftw_plan_dft_1d(N,buck1,buck1,FFTW_BACKWARD,FFTW_MEASURE);

i = i+L;
}
fclose(fp_out);
wav_close(wav_rx);
}

int main(){
printf("sizeof double = %d bytes\n",sizeof(double));
wav2CIR("lfm_data_T1_l5.wav","T1_raw.wav");
printf("done\n");
return 0;
}
