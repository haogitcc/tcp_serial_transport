#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdlib.h>
#include <stdio.h>              /* perror etc. */
#include <unistd.h>             /* close(fd) etc. */
#include <string.h>

// Socket related headers
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <strings.h>            /* bzero etc. */
#include <termios.h>

// Serial inteface headers
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef BOOL
#define BOOL unsigned char
#endif

#ifndef TRUE
#define TRUE (BOOL)1
#endif

#ifndef FALSE
#define FALSE (BOOL)0
#endif

#define TTY_STORE       16

struct termios orig_tty_state[TTY_STORE];
int sttyfds[TTY_STORE];

void open_port(int *fd, char *dev);
void close_port(int fd);
int set_baudrate(int fd, int baud_rate);

int write_to_port(int fd, const char *data, int size_data);
int read_from_port(int fd, char *data, int size_data);
int stty_raw(int fd);

#endif
