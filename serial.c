#include "serial.h"
#include <sys/io.h>             // ioperm
#include <unistd.h>             // ioperm
#include <termios.h>

/* 
 * Opens the serial port named `dev'
 */
void open_port(int *fd, char *dev) {
	plog("open_port device=%s", dev);
	*fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (*fd == -1) {
		perror("Couldn't open serial port");
		exit(-1);
	}
    // test whether a file descriptor refers to a terminal
	if (isatty(*fd) != 1) {
		perror("Invalid terminal device");
		exit(-1);
	}

	stty_raw(*fd);		// configure the device
	// rts_off(*fd);
}

void close_port(int fd) {
	plog("close_port");
	if (fd > 0) {
		close(fd);
		fd = -1;
	}
}

int set_baudrate(int fd, int baud_rate)
{
	//Set BAUDRATE
	struct termios tty_state;	
	tcgetattr(fd, &tty_state);
	
#define BCASE(tty_state,n) case n: cfsetispeed((tty_state),B##n); cfsetospeed((tty_state),B##n); break;
    switch (baud_rate)
    {
      BCASE(&tty_state, 9600);
      BCASE(&tty_state, 19200);
      BCASE(&tty_state, 38400);
      // Believe it or not, speeds beyond 38400 aren't required by POSIX.
#ifdef B57600
      BCASE(&tty_state, 57600);
#endif
#ifdef B115200
      BCASE(&tty_state, 115200);
#endif
#ifdef B230400
      BCASE(&tty_state, 230400);
#endif
#ifdef B460800
      BCASE(&tty_state, 460800);
#endif
#ifdef B921600
      BCASE(&tty_state, 921600);
#endif
    default:
      return -1;
    }
#undef BCASE
    if (tcsetattr(fd, TCSANOW, &tty_state) != 0)
    {
      return errno;
    }

}

int write2port(int fd, const char *data, int size_data) {
	//plog("--> write2port");
	int size = 0;
	int maxfd = 0;
	int s = 0;
	fd_set fdset;

	size = size_data;

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	maxfd = fd + 1;
	select(maxfd, 0, &fdset, 0, NULL);    // block until the data arrives

	if (FD_ISSET(fd, &fdset) == FALSE)    // data ready?
	{
		printf("Not Ready (W)\n");
		return 0;						  //writen continues trying
	}

	s = serial_sendBytes(fd, data, size);            // dump the data to the device
	fsync(fd); 							  // sync the state of the file

	return s;
}

int read_from_port(int fd, char *data, int size_data) {
	//plog("<-- read_from_port");
	int ret = -1;
	int count = -1;
	struct timeval timeout;
	timeout.tv_sec = 3L;
	timeout.tv_usec = 0;
	
	int maxfd = 0;
	fd_set fdset;

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	maxfd = fd + 1;

	ret = select(maxfd, &fdset, 0, 0, &timeout);     // block until the data arrives
	if (ret < 1)
    {
    	perror("read_from_port select error!");
		return -1;
    }

	if (FD_ISSET(fd, &fdset) == TRUE)      // data ready
	{
		count = read(fd, data, size_data);
		if (count == -1)
		{
			perror("read_from_port read error!");
			if (ENXIO == errno)
			{
				return -1; 
			}
			else
			{
				return -1;
			}
		}
		return count;
	}
	else
	{
		plog("Not Ready!\n");
		return 0;
	}
}


/*
 * serial port configuration
 */
int stty_raw(int fd) {
	plog("stty_raw");
	struct termios tty_state;
	int i = 0;

	if (tcgetattr(fd, &tty_state) < 0) {
		exit(-1);
	}

	for (i = 0; i < TTY_STORE; i++) {
		if (sttyfds[i] == -1) {
			orig_tty_state[i] = tty_state;
			sttyfds[i] = fd;
			break;
		}
	}

	// non-canonical mode turns off input character processing
	// echo is off
	tty_state.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
	tty_state.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT);
	tty_state.c_oflag &= ~OPOST;
	tty_state.c_cflag |= (CS8 | CRTSCTS);
	tty_state.c_cc[VMIN] = 1;
	tty_state.c_cc[VTIME] = 0;

	if (cfsetospeed(&tty_state, B115200) < 0) {
		perror("cfsetospeed");
		exit(-1);
	}

	if (tcsetattr(fd, TCSAFLUSH, &tty_state) < 0) {
		perror("tcsetattr");
		exit(-1);
	}

	return 0;
}

int serial_sendBytes(int fd, uint8_t* message, uint32_t length)
{
	//plog("serial_sendBytes buffer = %p,%p,%p,%p,%p, %d",message[0],message[1],message[2],message[3],message[4], length);
	int ret;
	do 
	{
		ret = write(fd, message, length);
		if (ret == -1)
		{
			return errno;
		}
		length -= ret;
		message += ret;
	}
	while (length > 0);

	return 0;
}
