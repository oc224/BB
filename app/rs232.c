/*
 ***************************************************************************
 *
 * Author: Teunis van Beelen
 *
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Teunis van Beelen
 *
 * teuniz@gmail.com
 *
 ***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ***************************************************************************
 *
 * This version of GPL is at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 ***************************************************************************
 */

/* last revision: Januari 31, 2014 */

/* For more info and how to use this library, visit: http://www.teuniz.net/RS-232/ */

#include "rs232.h"
#include "system.h"
#include <stdlib.h>

//TODO TIMEOUT BUSYWAITING
int error;
//int Cport[30], int error; 
struct termios new_port_settings,old_port_settings;

/*char comports[30][16]= {"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/dev/ttyS4","/dev/ttyS5",
	"/dev/ttyS6","/dev/ttyS7","/dev/ttyS8","/dev/ttyS9","/dev/ttyS10","/dev/ttyS11",
	"/dev/ttyS12","/dev/ttyS13","/dev/ttyS14","/dev/ttyS15","/dev/ttyUSB0",
	"/dev/ttyUSB1","/dev/ttyUSB2","/dev/ttyUSB3","/dev/ttyUSB4","/dev/ttyUSB5",
	"/dev/ttyAMA0","/dev/ttyAMA1","/dev/ttyACM0","/dev/ttyACM1",
	"/dev/rfcomm0","/dev/rfcomm1","/dev/ircomm0","/dev/ircomm1"};
*/
int RS232_OpenComport(const char* devname,int baudrate)
{
	int baudr; 
	//status;
	int fd;

	switch(baudrate)
	{
		case 50 : baudr = B50;
		break;
		case 75 : baudr = B75;
		break;
		case 110 : baudr = B110;
		break;
		case 134 : baudr = B134;
		break;
		case 150 : baudr = B150;
		break;
		case 200 : baudr = B200;
		break;
		case 300 : baudr = B300;
		break;
		case 600 : baudr = B600;
		break;
		case 1200 : baudr = B1200;
		break;
		case 1800 : baudr = B1800;
		break;
		case 2400 : baudr = B2400;
		break;
		case 4800 : baudr = B4800;
		break;
		case 9600 : baudr = B9600;
		break;
		case 19200 : baudr = B19200;
		break;
		case 38400 : baudr = B38400;
		break;
		case 57600 : baudr = B57600;
		break;
		case 115200 : baudr = B115200;
		break;
		case 230400 : baudr = B230400;
		break;
		case 460800 : baudr = B460800;
		break;
		case 500000 : baudr = B500000;
		break;
		case 576000 : baudr = B576000;
		break;
		case 921600 : baudr = B921600;
		break;
		case 1000000 : baudr = B1000000;
		break;
		default : printf("invalid baudrate\n");
		return(1);
		break;
	}

	fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd==-1)
	{
		perror("unable to open comport ");
		return(1);
	}

	error = tcgetattr(fd, &old_port_settings);
	if(error==-1)
	{
		close(fd);
		perror("unable to read portsettings ");
		return(1);
	}
	memset(&new_port_settings, 0, sizeof(new_port_settings)); /* clear the new struct */

	new_port_settings.c_cflag = baudr | CS8 | CLOCAL | CREAD;
	new_port_settings.c_iflag = IGNPAR;
	new_port_settings.c_oflag = 0;
	new_port_settings.c_lflag = ICANON;
	new_port_settings.c_cc[VMIN] = 0; /* block untill n bytes are received */
	new_port_settings.c_cc[VTIME] = 0; /* block untill a timer expires (n * 100 mSec.) */
	error = tcsetattr(fd, TCSANOW, &new_port_settings);
	//IGNPAR Ignore framing errors and parity errors.
	//CLOCAL Ignore modem control lines.
	//CREAD enable receiver
	if(error==-1)
	{
		close(fd);
		perror("unable to adjust portsettings ");
		return(1);
	}

	/*if(ioctl(fd, TIOCMGET, &status) == -1)
	{
		perror("unable to get portstatus");
		return(1);
	}*/

	//status |= TIOCM_DTR; /* turn on DTR */
	//status |= TIOCM_RTS; /* turn on RTS */

	/*if(ioctl(fd, TIOCMSET, &status) == -1)
	{
		perror("unable to set portstatus");
		return(1);
	}*/

	return(fd);
}

inline int RS232_PollComport(int fd,char *buf, int size)
{
	int n;
	n = read(fd, buf, size);
	return(n);
}
inline void RS232_Flush(int fd) {
	tcflush(fd,TCIOFLUSH);
}

inline int RS232_SendByte(int fd,const char byte)
{
	int n;
	n = write(fd, &byte, 1);
	if(n<0) return(1);
	return(0);
}

inline int RS232_SendBuf(int fd,const char *buf, int size)
{
	return(write(fd, buf, size));
}

void RS232_CloseComport(int fd)
{
	int status;

	if(ioctl(fd, TIOCMGET, &status) == -1)
	{
		perror("unable to get portstatus");
	}

	//status &= ~TIOCM_DTR; /* turn off DTR */
	//status &= ~TIOCM_RTS; /* turn off RTS */

	/*if(ioctl(fd, TIOCMSET, &status) == -1)
	{
		perror("unable to set portstatus");
	}*/

	tcsetattr(fd, TCSANOW, &old_port_settings);
	close(fd);
}
