#include <stdio.h>
#include <fftw3.h>
#include "signal.h"
#include "wav.h"
#define N 10
int main(){
FILE *fp;
fftw_complex *out;
int i;
out=(fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
/*open output file*/
if ((fp=fopen("res2","wb"))==NULL){
printf("fail to open \n");
return 0;
}
/*feed output space*/
for (i=0;i<N;i++){
out[i][0]=i;out[i][1]=i;
}
/*show*/
fftw_complex_show(out,N);
/*write out*/
double_write_file(fp,out,N);

/**/
fclose(fp);
return 0;
}
