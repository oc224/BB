#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "wav.h"
#include "signal.h"
#include "NE10.h"
#include "arm_neon.h"
#define MODEM_BW 10240


int DATA_COOK_show(DATA_COOK *dq){
printf("%20s %f\n","SNR : ",dq->snr);
printf("%20s %f\n","AVG : ",dq->avg);
printf("%20s %f\n","MAX : ",dq->max);
printf("%20s %f\n","OFFSET (msec) : ",dq->offset*1000);
printf("%20s %d\n","I_OFFSET : ",dq->i_offset);
printf("%20s %d:%d:%f\n","Record time : ",dq->hh,dq->mm,dq->ss);
printf("%20s %d:%d:%f\n","RX time : ",dq->hh,dq->mm,dq->ss+dq->offset);
}

int f32_findpeak(ne10_float32_t *input,int N,ne10_float32_t *avg,ne10_float32_t *max,int *i,int *i_offset){
int j;
//printf("N = %d\n",N);
for (j=0;j<N;j++){
(*i)++;

if (isnan(input[j])) continue;
//printf("%f %f  %f %f %f\n",input[j],*avg,input[j]-*avg,(float)(*i),(input[j]-(*avg))/(float)(*i));
*avg += (input[j]-*avg)/(float)(*i);
if (input[j]>*max){
*max=input[j];
*i_offset=*i-1;
}
}

return 0;
}
int f32_findpeak_end(const char *fname,ne10_float32_t avg,ne10_float32_t max,int i_offset,DATA_COOK *dq){
ne10_float32_t snr;
FILE *fp;
char buf[100];
int hh,mm;
float ss,offset;

snr = max / avg;
offset=(float)i_offset/(float)MODEM_BW;
//if (snr > threshold)printf("peak dectected\n");
printf("%20s %f\n","SNR : ",snr);
printf("%20s %f\n","AVG : ",avg);
printf("%20s %f\n","MAX : ",max);
printf("%20s %f\n","OFFSET (msec) : ",offset*1000);
printf("%20s %d\n","I_OFFSET : ",i_offset);
dq->snr=snr;
dq->avg=avg;
dq->max=max;
dq->offset=offset;
dq->i_offset=i_offset;

//open log file
fp=fopen(fname,"r");
if (fp==NULL){
fprintf(stderr,"%s,fail to open %s\n",__func__,fname);
return FAIL;}
//read
fgets(buf,100,fp);
fgets(buf,100,fp);
fclose(fp);
sscanf(buf,"%*s %*s %d:%d:%f",&hh,&mm,&ss);

//return
printf("%20s %d:%d:%f\n","Record time : ",hh,mm,ss);
printf("%20s %d:%d:%f\n","RX time : ",hh,mm,ss+offset);
dq->hh=hh;
dq->mm=mm;
dq->ss=ss;
return SUCCESS;
}

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

//int f32_peak()

int cpx_add(ne10_fft_cpx_float32_t  *out,ne10_fft_cpx_float32_t  * in1,ne10_fft_cpx_float32_t * in2,int N){
ne10_add_vec2f_neon((ne10_vec2f_t *)out,(ne10_vec2f_t *)in1,(ne10_vec2f_t *)in2,N);
return 0;
}

int cpx_abs(ne10_float32_t * out,ne10_fft_cpx_float32_t *in,uint N){
ne10_len_vec2f_neon(out,(ne10_vec2f_t *)in,N);
return 0;
}

int wav2CIR(const char* rx,const char *tx,const char *out,DATA_COOK *dc){
/*allocate resources*/
wav *wav_rx,*wav_tx;
int M;//length of filter
int L;//nearest of 2 power of M
int N;// 2L
int Nx;//length(rx)
int i;
int k;
int i_peak;
char flog[100];
int is_rx_eof;
FILE *fp_out;
ne10_fft_cpx_float32_t *H,*X,*tmp,*buck;
ne10_float32_t *buck1,*buck2,*swap_tmp;
ne10_fft_cfg_float32_t p;
ne10_float32_t avg = 0, max = 0;
int j = 0,j_offset = 0;
if ((wav_tx=wav_open(tx))==NULL){
fprintf(stderr,"%s, unable to open tx wave %s\n",__func__,tx);
return FAIL;}
if ((wav_rx=wav_open(rx))==NULL){
fprintf(stderr,"%s, unable to open rx wave %s\n",__func__,rx);
return FAIL;}

uint16_t *dump;//store to file ptr
/*output file*/
if ((fp_out=fopen(out,"wb"))==NULL){
fprintf(stderr,"fail to open output file\n");
return FAIL;
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

buck = (ne10_fft_cpx_float32_t*) NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*N);
tmp = (ne10_fft_cpx_float32_t*) NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*N);
p = ne10_fft_alloc_c2c_float32(N);


/* fft tx */
cpx_clear(tmp,N);
cpx_clear(H,N);
wav_read(wav_tx,tmp,M,REVERSE);
wav_close(wav_tx);
ne10_fft_c2c_1d_float32_neon(H,tmp,p,0);

i=0;
while (i<Nx){

//fft rx II
cpx_clear(tmp,N);
cpx_clear(X,N);
is_rx_eof=(wav_read(wav_rx,tmp,L,NORMAL)==FAIL);
ne10_fft_c2c_1d_float32_neon(X,tmp,p,0);


//ifft (cross) = ifft ( X * H )
cpx_clear(tmp,N);
cpx_mul(tmp,X,H,N);
ne10_fft_c2c_1d_float32_neon(buck,tmp,p,1);
ne10_setc_float_neon(buck1,0.0,N);
cpx_abs(buck1,buck,N);

//result_puts(upper half p_cross+buf)
f32_add(buck2,buck1,buck2+L,L);

if (i==0){
data_st(fp_out,buck2+M-1,sizeof(ne10_float32_t),L-M+1);
f32_findpeak(buck2+M-1,L-M+1,&avg,&max,&j,&j_offset);
}else{
data_st(fp_out,buck2,sizeof(ne10_float32_t),L);
f32_findpeak(buck2,L,&avg,&max,&j,&j_offset);
};

if(is_rx_eof){
data_st(fp_out,buck1+L,sizeof(ne10_float32_t),L);
f32_findpeak(buck1+L,L,&avg,&max,&j,&j_offset);
break;}

/*swap*/
swap_tmp=buck1;
buck1=buck2;
buck2=swap_tmp;

i = i+L;

}
//log file name
strcpy(flog,rx);
strcpy(strstr(flog,".wav"),".log");

//rx time
f32_findpeak_end(flog,avg,max,j_offset,dc);

//return
NE10_FREE(H);
NE10_FREE(X);
NE10_FREE(buck1);
NE10_FREE(buck2);
NE10_FREE(buck);
NE10_FREE(tmp);
fclose(fp_out);
wav_close(wav_rx);
return SUCCESS;
}
