#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "wav.h"
#include "signal.h"
#include "NE10.h"
#include "arm_neon.h"

int findpeak(const char *fname,float threshold){
// fname = .out file
// threshold for determine value max/avg
FILE *fp;
ne10_float32_t t_value, avg = 0, max=0, snr;
int i = 1, i_max;
//open
fp=fopen(fname,"rb");
if (fp==NULL){
printf("%s, open %s error \n",__func__,fname);
return FAIL;}

//read ele
while (fread(&t_value,4,1,fp)){
i++;
if (isnan(t_value)) continue;
//printf("read %f\n",t_value);
//sum max
avg += (t_value-avg)/(float)i;
if (t_value>max){
max=t_value;
i_max=i-2;}
}
fclose(fp);
snr = max / avg;
if (snr > threshold)printf("peak dectected\n");
printf("snr %f, avg %f, max, %.4f\n max @%.4f sec, (sample index %d)\n",snr,avg,max,(float)i_max/(float)10240,i_max);

return i_max;
}


int rx_time(const char *fname,int i){
//fname log file
//i sample index to the max value in proccessed wavform
FILE *fp;
char buf[100];
int hh,mm;
float ss;

//open log file
printf("open %s...\n",fname);
fp=fopen(fname,"r");
if (fp==NULL){
fprintf(stderr,"%s,fail to open %s\n",__func__,fname);
return FAIL;}
//read
fgets(buf,100,fp);
fgets(buf,100,fp);
sscanf(buf,"%*s %*s %d:%d:%f",&hh,&mm,&ss);
printf("in log %d %d %f\n",hh,mm,ss);
ss+=(float)i/(float)10240;
printf("modifi log %d %d %f\n",hh,mm,ss);
//close
fclose(fp);
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

int wav2CIR(const char* rx,const char *tx,const char *out){
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
is_rx_eof=(wav_read(wav_rx,tmp,L,NORMAL)<0);
ne10_fft_c2c_1d_float32_neon(X,tmp,p,0);


//ifft (cross) = ifft ( X * H )
cpx_clear(tmp,N);
cpx_mul(tmp,X,H,N);
ne10_fft_c2c_1d_float32_neon(buck,tmp,p,1);
ne10_setc_float_neon(buck1,0.0,N);
cpx_abs(buck1,buck,N);

//result_puts(upper half p_cross+buf)
f32_add(buck2,buck1,buck2+L,L);

//data_st(fp_out,buck2,sizeof(ne10_float32_t),L);
if (i==0){
data_st(fp_out,buck2+M-1,sizeof(ne10_float32_t),L-M+1);
}else{
data_st(fp_out,buck2,sizeof(ne10_float32_t),L);
};

if(is_rx_eof){
data_st(fp_out,buck1+L,sizeof(ne10_float32_t),L);
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
rx_time(flog,findpeak(out,10.0));

NE10_FREE(H);
NE10_FREE(X);
NE10_FREE(buck1);
NE10_FREE(buck2);
NE10_FREE(buck);
NE10_FREE(tmp);
fclose(fp_out);
wav_close(wav_rx);
}
