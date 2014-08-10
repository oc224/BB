#include <fftw3.h>
int fftw_complex_multiply(fftw_complex*,fftw_complex*,fftw_complex*,int);

int fftw_complex_plus(fftw_complex *,fftw_complex *,fftw_complex*,int);

int wav2CIR(const char*,const char *);
