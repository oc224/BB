#ifndef SIGNAL_H
#define SIGNAL_H
#include "NE10.h"
typedef struct{
ne10_float32_t snr;
ne10_float32_t avg;
ne10_float32_t max;
ne10_float32_t offset;
int i_offset;
float ss;
int hh;
int mm;
}DATA_COOK;

int wav2CIR(const char*,const char *,const char *,DATA_COOK *);
int DATA_COOK_show(DATA_COOK *);

#endif
