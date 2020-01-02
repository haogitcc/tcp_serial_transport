#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>

int server();
void* tcp_serial_transport(void *arg);
int anetKeepAlive(int fd, int interval);
int tcp_sendBytes(int fd, uint8_t* message, uint32_t length);

#endif