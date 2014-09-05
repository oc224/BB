#include "rs232.h"
#include <stdio.h>
#include <unistd.h>
#define devno 18
#define BUFSIZE 32
int main() {
	char buf[32];
	int n;
	printf("simple test\n");
// open rs232
	printf("!open\n");
	if (RS232_OpenComport(devno, 115200)) {
		printf("fail to open \n");
		return 0;
	}
	printf("\n");

// read test if block
	printf("!read port, blocking...\n");
	n = RS232_PollComport(devno, buf, BUFSIZE);
	if (n < 1) {
		printf("no read\n");

	}
	printf("n=%d\n", n);
	printf("read function return\n");
	printf("\n");

// write and read
	RS232_Flush(devno);
	RS232_SendBuf(devno, "at\r", 3);
	usleep(1000000);
	n = RS232_PollComport(devno, buf, BUFSIZE);
	buf[n] = 0;
	printf("n=%d\n", n);
	printf("rs232 port read :%s", buf);

// ack
	printf("!wait ack\n");
	RS232_Flush(devno);
	printf("\n");

// info
	printf("!wait info\n");
	RS232_Flush(devno);
	printf("\n");

// close rs232
	printf("close port\n");
	RS232_CloseComport(devno);
	return 0;
}
