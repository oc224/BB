/*
 *	V7/BSD HACKERS:  SEE NOTES UNDER mode(2) !!!
 *
 *   This file is #included so the main file can set parameters such as HOWMANY.
 *   See the main files (rz.c/sz.c) for compile instructions.
 */

char *Copyr = "Copyright 2006 Omen Technology Inc All Rights Reserved";

#ifdef V7
#include <sys/types.h>
#include <sys/stat.h>
#define STAT
#include <sgtty.h>
#define OS "V7/BSD"
#define void
#define NFGM
#include <setjmp.h>
char *getenv(), *ttyname();
#ifdef LLITOUT
long Locmode;		/* Saved "local mode" for 4.x BSD "new driver" */
long Locbit = LLITOUT;	/* Bit SUPPOSED to disable output translations */
#include <strings.h>
#endif
#endif

#ifdef USG
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define STAT
#include <termio.h>
#define OS "SYS III/V"
#define MODE2OK
#include <string.h>
#ifndef OLD
#include <stdlib.h>
#endif
#include <unistd.h>
#endif

#ifdef POSIX
#define USG
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#define STAT
#include <termios.h>
#define OS "POSIX"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#ifndef READCHECK
#ifndef FIONREAD
#define SV
#endif
#endif
#endif

#ifdef OLD
char *ttyname();
char *getenv();
#define time_t long
#endif


#ifdef T6K
#include <sys/ioctl.h>		/* JPRadley: for the Tandy 6000 */
#endif

#ifdef IT
#include  <sys/time.h>
#endif

#if HOWMANY  > 255
Howmany must be 255 or less
#endif

 /*
 *  Some systems (Venix, Coherent, Regulus) may not support tty raw mode
 *  read(2) the same way as Unix. ONEREAD must be defined to force one
 *  character reads for these systems. Added 7-01-84 CAF
 */

#define sendline(c) putchar(c)
#define xsendline(c) putchar(c)

int Tty;
char linbuf[HOWMANY];
char xXbuf[BUFSIZ];
int Lleft=0;		/* number of characters in linbuf */
#ifdef POSIX
sigjmp_buf timedout;		/* For the timeout interrupt on RX timeout */
#else
jmp_buf timedout;		/* For the timeout interrupt on RX timeout */
#endif
#ifdef ONEREAD
/* Sorry, Regulus and some others don't work right in raw mode! */
int Readnum = 1;	/* Number of bytes to ask for in read() from modem */
#else
int Readnum = HOWMANY;	/* Number of bytes to ask for in read() from modem */
#endif
int Verbose=0;
#ifdef IT
struct itimerval value, value0;		/* Assumes data seg defaults to 0 */
#endif


/*
 *  The following uses an external rdchk() routine if available,
 *  otherwise defines the function for BSD or fakes it for SYSV.
 */

#ifndef READCHECK
#ifdef FIONREAD
#define READCHECK
/*
 *  Return non 0 iff something to read from io descriptor f
 */
rdchk(f)
{
	static long lf;

	ioctl(f, FIONREAD, &lf);
	return ((int) (lf>0));
}

#else		/* FIONREAD */

#ifdef SV
#define READCHECK

int checked = 0;
/*
 * Nonblocking I/O is a bit different in System V, Release 2
 *  Note: this rdchk vsn throws away a byte, OK for ZMODEM
 *  sender because protocol design anticipates this problem.
 */
#define EATSIT
rdchk(f)
{
	int lf, savestat;
	static char bchecked;

	savestat = fcntl(f, F_GETFL) ;
#ifdef O_NDELAY
	fcntl(f, F_SETFL, savestat | O_NDELAY) ;
#else
	fcntl(f, F_SETFL, savestat | O_NONBLOCK) ;
#endif
	lf = read(f, &bchecked, 1) ;
	fcntl(f, F_SETFL, savestat) ;
	checked = bchecked & 0377;	/* force unsigned byte */
	return(lf>0) ;
}
#endif
#endif
#endif


struct {
	unsigned baudr;
	int speedcode;
} speeds[] = {
	110,	B110,
#ifdef B150
	150,	B150,
#endif
	300,	B300,
	600,	B600,
	1200,	B1200,
	2400,	B2400,
	4800,	B4800,
	9600,	B9600,
#ifdef B19200
	19200,	B19200,
#endif
#ifdef EXTA
	19200,	EXTA,
#endif
#ifdef B38400
	38400,	B38400,
#endif
#ifdef EXTB
	38400,	EXTB,
#endif
	0,	0
};
static unsigned
getspeed(code)
{
	register n;

	for (n=0; speeds[n].baudr; ++n)
		if (speeds[n].speedcode == code)
			return speeds[n].baudr;
	if (code > 49)
		return ((unsigned)code);
	return 1;	/* Assume fifo if ioctl failed */
}


#ifdef ICANON
#ifdef POSIX
struct termios oldtty, tty;
#else
struct termio oldtty, tty;
#endif
#else
struct sgttyb oldtty, tty;
struct tchars oldtch, tch;
#endif

/*
 * mode(n)
 *  3: save old tty stat, set raw mode with flow control
 *  2: set XON/XOFF for sb/sz with ZMODEM or YMODEM-g
 *  1: save old tty stat, set raw mode 
 *  0: restore original tty mode
 */
int Test7;		/* Test hack simulates 7 bit even parity path */
mode(n)
{
	static did0 = FALSE;

	vfile("mode:%d", n);
	switch(n) {
#ifdef USG
	case 2:		/* Un-raw mode used by sz, sb when -g detected */
#ifdef POSIX
		if(!did0)
			(void) tcgetattr(Tty, &oldtty);
#else
		if(!did0)
			(void) ioctl(Tty, TCGETA, &oldtty);
#endif
		tty = oldtty;

		tty.c_iflag = BRKINT|IXON;

		tty.c_oflag = 0;	/* Transparent output */

		/* Cflag controls hardware config of the tty line */
		if (Test7) {
			tty.c_cflag &= ~(CSIZE);
			tty.c_cflag |= (CS7|PARENB|CREAD);
			if (Test7 == 1)
				tty.c_cflag |= PARODD;
		} else {
			tty.c_cflag &= ~(CSIZE|PARENB);
			tty.c_cflag |= (CS8|CREAD);
		}


#ifdef READCHECK
		tty.c_lflag = Zmodem ? 0 : ISIG;
		tty.c_cc[VINTR] = Zmodem ? -1:030;	/* Interrupt char */
#else
		tty.c_lflag = ISIG;
		tty.c_cc[VINTR] = Zmodem ? 03:030;	/* Interrupt char */
#endif
		tty.c_cc[VQUIT] = -1;			/* Quit char */
#ifdef NFGVMIN
		tty.c_cc[VMIN] = 1;
#else
		tty.c_cc[VMIN] = 3;	 /* This many chars satisfies reads */
#endif
		tty.c_cc[VTIME] = 1;	/* or in this many tenths of seconds */

#ifdef POSIX
		(void) tcsetattr(Tty, TCSADRAIN, &tty);
#else
		(void) ioctl(Tty, TCSETAW, &tty);
#endif
		did0 = TRUE;
		return OK;
	case 1:
	case 3:
#ifdef POSIX
		if(!did0)
			(void) tcgetattr(Tty, &oldtty);
#else
		if(!did0)
			(void) ioctl(Tty, TCGETA, &oldtty);
#endif
		tty = oldtty;

		tty.c_iflag = n==3 ? (IXON|IXOFF) : IXOFF;

		tty.c_lflag = 0;

		tty.c_oflag = 0;

		/* Cflag controls hardware config of the tty line */
		if (Test7) {
			tty.c_cflag &= ~(CSIZE);
			tty.c_cflag |= (CS7|PARENB|CREAD);
			if (Test7 == 1)
				tty.c_cflag |= PARODD;
		} else {
			tty.c_cflag &= ~(CSIZE|PARENB);	/* disable parity */
			tty.c_cflag |= CS8;	/* Set character size = 8 */
		}
#ifdef NFGVMIN
		tty.c_cc[VMIN] = 1; /* This many chars satisfies reads */
#else
		tty.c_cc[VMIN] = HOWMANY; /* This many chars satisfies reads */
#endif
		tty.c_cc[VTIME] = 1;	/* or in this many tenths of seconds */
#ifdef POSIX
		(void) tcsetattr(Tty, TCSADRAIN, &tty);
#else
		(void) ioctl(Tty, TCSETAW, &tty);
#endif
		did0 = TRUE;
#ifdef POSIX
		Baudrate = getspeed(cfgetospeed(&tty));
#else
		Baudrate = getspeed(tty.c_cflag & CBAUD);
#endif
		vfile("Baudrate = %u", Baudrate);
		return OK;
#endif
#ifdef V7
	/*
	 *  NOTE: this should transmit all 8 bits and at the same time
	 *   respond to XOFF/XON flow control.  If no FIONREAD or other
	 *   rdchk() alternative, also must respond to INTRRUPT char
	 *   This doesn't work with V7.  It should work with LLITOUT,
	 *   but LLITOUT was broken on the machine I tried it on.
	 */
	case 2:		/* Un-raw mode used by sz, sb when -g detected */
		if(!did0) {
			ioctl(Tty, TIOCEXCL, 0);
			ioctl(Tty, TIOCGETP, &oldtty);
			ioctl(Tty, TIOCGETC, &oldtch);
#ifdef LLITOUT
			ioctl(Tty, TIOCLGET, &Locmode);
#endif
		}
		tty = oldtty;
		tch = oldtch;
#ifdef READCHECK
		tch.t_intrc = Zmodem ? -1:030;	/* Interrupt char */
#else
		tch.t_intrc = Zmodem ? 03:030;	/* Interrupt char */
#endif
		tty.sg_flags |= (ODDP|EVENP|CBREAK);
		tty.sg_flags &= ~(ALLDELAY|CRMOD|ECHO|LCASE);
		ioctl(Tty, TIOCSETP, &tty);
		ioctl(Tty, TIOCSETC, &tch);
#ifdef LLITOUT
		ioctl(Tty, TIOCLBIS, &Locbit);
#else
		bibi(99);	/* un-raw doesn't work w/o lit out */
#endif
		did0 = TRUE;
		return OK;
	case 1:
	case 3:
		if(!did0) {
			ioctl(Tty, TIOCEXCL, 0);
			ioctl(Tty, TIOCGETP, &oldtty);
			ioctl(Tty, TIOCGETC, &oldtch);
#ifdef LLITOUT
			ioctl(Tty, TIOCLGET, &Locmode);
#endif
		}
		tty = oldtty;
		tty.sg_flags |= (RAW|TANDEM);
		tty.sg_flags &= ~ECHO;
		ioctl(Tty, TIOCSETP, &tty);
		did0 = TRUE;
		Baudrate = getspeed(tty.sg_ospeed);
		return OK;
#endif
	case 0:
		if(!did0)
			return ERROR;
#ifdef USG
#ifdef POSIX
		(void) tcdrain(Tty);	/* Wait for output to drain */
		(void) tcflush(Tty, TCIFLUSH);	/* Flush input queue */
		(void) tcsetattr(Tty, TCSADRAIN, &oldtty);	/* Restore */
		(void) tcflow(Tty, TCOON);	/* Restart output */
#else
		(void) ioctl(Tty, TCSBRK, 1);	/* Wait for output to drain */
		(void) ioctl(Tty, TCFLSH, 1);	/* Flush input queue */
		(void) ioctl(Tty, TCSETAW, &oldtty);	/* Restore modes */
		(void) ioctl(Tty, TCXONC,1);	/* Restart output */
#endif
#endif
#ifdef V7
		ioctl(Tty, TIOCSETP, &oldtty);
		ioctl(Tty, TIOCSETC, &oldtch);
		ioctl(Tty, TIOCNXCL, 0);
#ifdef LLITOUT
		ioctl(Tty, TIOCLSET, &Locmode);
#endif
#endif

		return OK;
	default:
		return ERROR;
	}
}

sendbrk()
{
#ifdef V7
#ifdef TIOCSBRK
#define CANBREAK
	sleep(1);
	ioctl(Tty, TIOCSBRK, 0);
	sleep(1);
	ioctl(Tty, TIOCCBRK, 0);
#endif
#endif
#ifdef USG
#define CANBREAK
#ifdef POSIX
	tcsendbreak(Tty, 200);
#else
	ioctl(Tty, TCSBRK, 0);
#endif
#endif
}

flushmoc()
{
	fflush(stdout);
}
flushmo()
{
	fflush(stdout);
}

/*
 * This version of readline is reasoably well suited for
 * reading many characters.
 *
 * timeout is in tenths of seconds
 */
void
alrm(c)
{
#ifdef POSIX
	siglongjmp(timedout, -1);
#else
	longjmp(timedout, -1);
#endif
}
readline(timeout)
int timeout;
{
	register n;
	register char *p;
	static char *cdq;	/* pointer for removing chars from linbuf */
	int c;

	if (--Lleft >= 0) {
		return (*cdq++ & 0377);
	}
#ifdef IT
	if (timeout == 0)
		timeout = 1;
	value.it_value.tv_sec = timeout/10;
	value.it_value.tv_usec = (timeout%10)*100000;
	if (Verbose > 5)
		fprintf(stderr, "Calling read: timeout=%d  Readnum=%d ",
		  timeout, Readnum);
#else
	n = timeout/10;
	if (n < 2)
		n = 2;
	if (Verbose > 5)
		fprintf(stderr, "Calling read: alarm=%d  Readnum=%d ",
		  n, Readnum);
#endif
#ifdef POSIX
	if (sigsetjmp(timedout, 1))
#else
	if (setjmp(timedout))
#endif
	{
#ifdef TIOCFLUSH
/*		ioctl(Tty, TIOCFLUSH, 0); 	*/
#endif
		Lleft = 0;
		if (Verbose>1)
			fprintf(stderr, "Readline:TIMEOUT\n");
		return TIMEOUT;
	}
	signal(SIGALRM, alrm);
#ifdef IT
	if (setitimer(ITIMER_REAL, &value, NULL)) {
//		sprintf(endmsg, "itimer error errno=%", errno);
		setitimer(ITIMER_REAL, &value0, NULL);
		return TIMEOUT;
	}
#else
	alarm(n);
#endif
	errno = 0;
	Lleft=read(Tty, cdq=linbuf, Readnum);
#ifdef IT
	setitimer(ITIMER_REAL, &value0, NULL);
#else
	alarm(0);
#endif
	if (Verbose > 5) {
		fprintf(stderr, "Read returned %d bytes errno=%d\n",
		  Lleft, errno);
	}
	if (Lleft < 1)
		return TIMEOUT;
	if (Verbose > 8) {
		for (p=cdq, n = Lleft; --n >= 0; ) {
			fprintf(stderr, "%02x ", *p++ &0377);
		}
		fprintf(stderr, "\n");
		for (p=cdq, n = Lleft; --n >= 0; ) {
			c = *p++ & 0177;
			if (!isprint(c))
				c = '.';
			fprintf(stderr, " %c ", c);
		}
		fprintf(stderr, "\n");
	}
	--Lleft;
	return (*cdq++ & 0377);
}



/*
 * Purge the modem input queue of all characters
 */
purgeline()
{
	Lleft = 0;
#ifdef USG
#ifdef POSIX
	tcflush(Tty, 0);
#else
	ioctl(Tty, TCFLSH, 0);
#endif
#else
	lseek(Tty, 0L, 2);
#endif
}

/*
 * Send a string to the modem, processing for \336 (sleep 1 sec)
 *   and \335 (break signal)
 */
zmputs(s)
char *s;
{
	register c;

	while (*s) {
		switch (c = *s++) {
		case '\336':
			sleep(1); continue;
		case '\335':
			sendbrk(); continue;
		default:
			sendline(c);
		}
	}
	flushmo();
}


/* VARARGS */
vfile(f, a, b, c, d)
/* VARARGS */
char *f;
long a, b, c, d;
{
	if (Verbose > 2) {
		fprintf(stderr, f, a, b, c, d);
		fprintf(stderr, "\n");
	}
}

/* End of rbsb.c */
