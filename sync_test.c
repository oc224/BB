/*
 * sync_test.c
 *
 *  Created on: 2014.8.1
 *      Author: r3399r
 */

#include "acoustic_modem.h"
#include <stdio.h>
#include <unistd.h>

int main()
{
	char buf[128];
	int i, j;
	amodem_open();
	amodem_puts("sync\r");
	sleep(1);
	for (i=1; i<=20; i++)
	{
		//for (j=0; j<128; j++)
			//buf[j] = 0;
		amodem_gets(buf,128);
		printf("%s",buf);
	}
	amodem_close();
	return 0;
}
