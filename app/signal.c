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
#define THRESHOLD 30.0


int DATA_COOK_show(DATA_COOK *dq){
printf("%20s %f\n","SNR : ",dq->snr);
printf("%20s %f\n","AVG : ",dq->avg);
printf("%20s %f\n","MAX : ",dq->max);
printf("%20s %f\n","OFFSET (msec) : ",dq->offset*1000);
printf("%20s %d\n","I_OFFSET : ",dq->i_offset);
printf("%20s %d:%d:%f\n","Record time : ",dq->hh,dq->mm,dq->ss);
printf("%20s %d:%d:%f\n","RX time : ",dq->hh,dq->mm,dq->ss+dq->offset);
}

static int f32_findpeak_end(const char *fname,ne10_float32_t avg,ne10_float32_t max,int i_offset,DATA_COOK *dq){
ne10_float32_t snr;
FILE *fp;
char buf[100];
int hh,mm;
float ss,offset;

snr = 20*log10(max / avg);
offset=(float)i_offset/(float)MODEM_BW;
//if (snr > threshold)printf("peak dectected\n");
/*printf("%20s %f\n","SNR : ",snr);
printf("%20s %f\n","AVG : ",avg);
printf("%20s %f\n","MAX : ",max);
printf("%20s %f\n","OFFSET (msec) : ",offset*1000);
printf("%20s %d\n","I_OFFSET : ",i_offset);*/
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
/*printf("%20s %d:%d:%f\n","Record time : ",hh,mm,ss);
printf("%20s %d:%d:%f\n","RX time : ",hh,mm,ss+offset);*/
dq->hh=hh;
dq->mm=mm;
dq->ss=ss;
return SUCCESS;
}

static int cpx_clear(ne10_fft_cpx_float32_t *in,int N){
ne10_setc_float_neon((ne10_float32_t *)in,0.0,2*N);
return 0;
}

static int cpx_mul(ne10_fft_cpx_float32_t *out,ne10_fft_cpx_float32_t* in1,ne10_fft_cpx_float32_t * in2,int N){
// output length N array out equal to complex multiplication of in1 and in2
int i;
for (i=0;i<N;i++){
out[i].r = in1[i].r*in2[i].r-in1[i].i*in2[i].i;
out[i].i = in1[i].r*in2[i].i+in1[i].i*in2[i].r;
}
return 0;
}

static int f32_add(ne10_float32_t  *out,ne10_float32_t  * in1,ne10_float32_t * in2,int N){
ne10_add_float_neon(out,in1,in2,N);
return 0;
}

static int cpx_add(ne10_fft_cpx_float32_t  *out,ne10_fft_cpx_float32_t  * in1,ne10_fft_cpx_float32_t * in2,int N){
ne10_add_vec2f_neon((ne10_vec2f_t *)out,(ne10_vec2f_t *)in1,(ne10_vec2f_t *)in2,N);
return 0;
}

static int cpx_abs(ne10_float32_t * out,ne10_fft_cpx_float32_t *in,uint N){
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
char flog[100];
int is_rx_eof;
int j = 0;
int N_out;
int state = 20;
int i_out=1;
int i_max=0;
FILE *fp_out;
ne10_fft_cpx_float32_t *H,*X,*tmp,*buck;
ne10_float32_t *buck1,*buck2,*swap_tmp,*p_out;
ne10_fft_cfg_float32_t p;
ne10_float32_t avg = 0, max = 0;
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

//output pointer
if (i==0){
p_out = buck2+M-1;
N_out = L-M+1;
}else{
p_out = buck2;
N_out = L;
}
if(is_rx_eof){
p_out = buck1+L;
N_out = L;
}

//store
data_st(fp_out,p_out,sizeof(ne10_float32_t),N_out);

//peak pick (LOCAL FIRST MAXIMUM POINT)
if (state > 0){
// cal avg, compare max
for (j=0;j<N_out;j++,i_out++){
if (isnan(p_out[j])) continue;
avg += (p_out[j]-avg)/(float)i_out;

if (p_out[j] > max){
max=p_out[j];
i_max=i_out-1;
if (p_out[j]>avg*THRESHOLD) state--;
}
}
}

if (is_rx_eof) break;


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
f32_findpeak_end(flog,avg,max,i_max,dc);

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


/*
int wav2CIR_exp(const char* rx,const char *tx,const char *out,DATA_COOK *dc){
wav *wav_tx,wav_rx;
int Nx,Ny;
ne10_fft_cpx_float32_t *x, *y;
if (wav_tx = wav_open(tx) == NULL){
printf(stderr,"%s, fail to open %s\n",__func__,tx);
return 0;
}
//load tx
Nx = wav_tx -> length;
x = NE10_MALLOC(sizeof(ne10_fft_cpx_float32_t)*Nt);
wav_read(wav_tx,t,M,NORMAL);


//loop

//load rx


//put y



}
*/
