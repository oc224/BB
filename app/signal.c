#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "wav.h"
#include "signal.h"
#include "NE10.h"
#include "arm_neon.h"
#define LENGTH 10
#define THRESHOLD 3000000000.0

int cpx_clear(ne10_fft_cpx_float32_t *in,int N){
ne10_setc_float_neon((ne10_float32_t *)in,0.0,2*N);
return 0;
}

int cpx_mul(ne10_fft_cpx_float32_t *out,ne10_fft_cpx_float32_t* in1,ne10_fft_cpx_float32_t * in2,int N){
// output length N array out equal to complex multiplication of in1 and in2
//ne10_mul_float(out,in1,in2,(ne10_ufloat32_t)N);
//float32x4x2_t i1=vld2q_f32(in1);
//float32x4x2_t i2=vld2q_f32(in2);

int i;
for (i=0;i<N;i++){
out[i].r = in1[i].r*in2[i].r-in1[i].i*in2[i].i;
out[i].i = in1[i].r*in2[i].i+in1[i].i*in2[i].r;
}
//ne10_vmul_vec2f((ne10_vec2f_t *)out,(ne10_vec2f_t *)in1,(ne10_vec2f_t *)in2,N/2);
return 0;
}


int f32_add(ne10_float32_t  *out,ne10_float32_t  * in1,ne10_float32_t * in2,int N){
ne10_add_float_neon(out,in1,in2,N);
return 0;
}


int f32_findpeak(ne10_float32_t *in,uint N,uint L,float threshold){
//float d=0;
//float mean=0;
//float mean=0;
float d;
int i;
ne10_float32_t *addr;
//ne10_float32_t *x1,*x2;

/*for (i=0;i<L;i++) mean+=*(in+i);
mean=mean/L;
*/

for (i=0,addr=in;i<L;i++,addr++){
d=*(addr)-threshold;
if (d>0){
//printf("peak detected , %f,%d\n",d,i);
return i;}

}
//printf("no peak, %f \n",d);
return -1;

/*
x1=in;x2=in;
for (i=0;i<L;i++,x1++) mean+=*x1;
for (;i<N;i++,x1++,x2++){
mean+=*x1-*x2;
d=*(x1+1)-threshold*mean/L;
if (d>0){
printf("peak detected , %f,%d\n",d,i);
return i;
}
}*/

/*for (i=0,x1=in;i<L/2;i++,x1++) x+=*(x1);
for (y1=x1;i<L;i++) y+=*(y1);
x1=in+L;
x2=x1-L/2;
y1=x2-1;
y2=in;
for (;i<N;i++,x1++,x2++,y1++,y2++){
x+=*x1-*x2;
y+=*y1-*y2;
d=y-x;
printf("%f\n",d);
if (d>threshold*x){
//printf("%f\n",d);
return i;
}
}*/
}

int complex_plus(ne10_fft_cpx_float32_t  *out,ne10_fft_cpx_float32_t  * in1,ne10_fft_cpx_float32_t * in2,int N){
ne10_add_vec2f_neon((ne10_vec2f_t *)out,(ne10_vec2f_t *)in1,(ne10_vec2f_t *)in2,N);
return 0;
}

int complex_abs(ne10_float32_t * out,ne10_fft_cpx_float32_t *in,uint N){
ne10_len_vec2f_neon(out,(ne10_vec2f_t *)in,N);
return 0;
}

int wav2CIR(const char* rx,const char *tx,const char *out){
/*allocate resources*/
wav *wav_rx,*wav_tx;
int M;//length of filter
int L;//nearest of 2 power of M
int N;// 2L
int Nx;
int i=0;
int k;
int i_peak;
FILE *fp_out;
ne10_fft_cpx_float32_t *H,*X,*tmp,*buck;
ne10_float32_t *buck1,*buck2,*swap_tmp;
ne10_fft_cfg_float32_t p;
wav_tx=wav_open(tx);
wav_rx=wav_open(rx);
uint16_t *dump;//store to file ptr
/*output file*/
if ((fp_out=fopen(out,"wb"))==NULL){
fprintf(stderr,"fail to open output file\n");
return -1;
}

/*some para*/
M=wav_tx->length;
L=pow(2,ceil(log(M)/log(2)));
N=2*L;
Nx=wav_rx->length;
printf("kernel length : %d\n",M);
printf("Nearest 2 Power: %d\n",L);
printf("fft size : %d\n",N);
printf("rx length : %d\n",Nx);
H = (ne10_fft_cpx_float32_t*) NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*N);
X = (ne10_fft_cpx_float32_t*) NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*N);
buck1 = (ne10_float32_t*) NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*N);
buck2 = (ne10_float32_t*) NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*N);
ne10_setc_float_neon(buck1,0.0,N);
ne10_setc_float_neon(buck2,0.0,N);

//memset((void*)buck1,0,sizeof(ne10_float32_t)*N);
//memset((void*)buck2,0,sizeof(ne10_float32_t)*N);

buck = (ne10_fft_cpx_float32_t*) NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*N);
tmp = (ne10_fft_cpx_float32_t*) NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*N);

p = ne10_fft_alloc_c2c_float32(N);

/* fft tx */
cpx_clear(tmp,N);
cpx_clear(H,N);
wav_read(wav_tx,tmp,M,REVERSE);
wav_close(wav_tx);
ne10_fft_c2c_1d_float32_neon(H,tmp,p,0);

/* fft rx plan*/
i=0;
while (i<Nx){
/*fft rx II*/
cpx_clear(tmp,N);
cpx_clear(X,N);
if (wav_read(wav_rx,tmp,M,NORMAL)<0)return 0;
ne10_fft_c2c_1d_float32_neon(X,tmp,p,0);

/*ifft (cross) = ifft ( X * H )*/
cpx_clear(tmp,N);
cpx_mul(tmp,X,H,N);
ne10_fft_c2c_1d_float32_neon(buck,tmp,p,1);
complex_abs(buck1,buck,N);

//result_puts(upper half p_cross+buf)
//f32_add(buck2,buck1,buck2+L,L);
f32_add(buck1,buck1,buck2+L,L);

// find peak
//i_peak=f32_findpeak(buck2,L,LENGTH,THRESHOLD);
i_peak=f32_findpeak(buck1,L,LENGTH,THRESHOLD);
if(i_peak>=0){
//printf("peak detected @ %f \n",(float )(i+i_peak)/10240.0);
//exit(0);
data_st(fp_out,buck2,sizeof(ne10_float32_t),L);
data_st(fp_out,buck1,sizeof(ne10_float32_t),L);

}
//data_st(fp_out,buck2,sizeof(ne10_float32_t),L);


/*swap*/
swap_tmp=buck1;
buck1=buck2;
buck2=swap_tmp;

i = i+L;
}
NE10_FREE(H);
NE10_FREE(X);
NE10_FREE(buck1);
NE10_FREE(buck2);
NE10_FREE(buck);
NE10_FREE(tmp);
fclose(fp_out);
wav_close(wav_rx);
}

/*int main(int argc,char* argv[]){
printf("proc %s file...\n",argv[1]);
wav2CIR(argv[1],argv[2],"res");
printf("done\n");
return 0;
}*/
