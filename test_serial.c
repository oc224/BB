#include <stdio.h>
#include "rs232.h"

int main(){
printf("test serial\n");
int portno=18;
if(RS232_OpenComport(portno, 115200))
  {
    printf("Can not open comport\n");

    return(0);
  }
//RS232_SendBuf(portno,"play /ffs/lfm_data_t1_l10.wav\r",32);
RS232_SendBuf(portno,"ls /\r",5);
RS232_CloseComport(portno);



}
