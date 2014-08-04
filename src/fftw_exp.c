#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>

#define N 16

int main(){
	int i;
	fftw_complex *in, *out;
	fftw_plan p;
	/*init */
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
	p = fftw_plan_dft_1d(N,in,out,FFTW_FORWARD,FFTW_MEASURE);
	printf("Note : please use Matlab to confirm the result \n");
	printf("by fft[0:15]'\n");
	/*feed */
	for (i=0;i<N;i++)*in[i]=(double)i;
	fftw_execute(p);
	
	/*show result */
	for (i=0;i<N;i++)printf("out[%d]:%f+i%f\n",i,out[i][0],out[i][1]);

	/*end*/
	fftw_destroy_plan(p);
	fftw_free(in);fftw_free(out);





}

