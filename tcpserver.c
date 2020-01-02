#include "tcpserver.h"

#define DEVICE "/dev/ttymxc1"
#define DEVICE_NAME "tmr:///dev/ttymxc1"

#define PORT 8086
#define MAX_BUFFER_SIZE 256

static int continueRead = 0;

int server()
{
	plog("Server start PORT=%d", PORT);
	int ret = -1;
	int server_fd = -1;
	int client_fd = -1;
	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(server_fd < 0) {
		perror("Server sockect create\n");
		return -1;
	}
	
	int is_reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &is_reuse, sizeof(is_reuse));
	
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");

	if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
        perror("Server socket bind error");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    if(listen(server_fd, 1) < 0)
	{
        perror("Server socket listen error\n");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    while(1)
    {
		int server_addr_len = sizeof(server_addr);
		client_fd = accept(server_fd, (struct sockaddr*)&server_addr, &server_addr_len);
		if(client_fd < 0)
		{
			perror("Server accept error\n");
			if (EBADF == errno)
				break;
			continue;
		}
		plog("Server accept success");
		plog("client_fd=%d", client_fd);

		anetKeepAlive(client_fd, 10);
		int sendbuf = 6144;
		setsockopt(client_fd, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf));
	    pthread_t t_transport = 0;
	    pthread_attr_t tattr_transport;
	    pthread_attr_init(&tattr_transport);
	    pthread_attr_setdetachstate(&tattr_transport, PTHREAD_CREATE_DETACHED);
	    pthread_create(&t_transport, &tattr_transport, tcp_serial_transport, &client_fd);
		
	}
	plog("Server Disconnect");
	return 0;
}

void* tcp_serial_transport(void *arg)
{
	plog("tcp_serial_transport");
	int ret = -1;
	int connected_fd = -1;
	struct timeval timeout;
  	fd_set rfdset,errorfdset;
	char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
	int count = -1;
	
	connected_fd = *((int *)arg);
	if(connected_fd < 0) {
		perror("Client fd is error\n");
		close(connected_fd);
		connected_fd = -1;
		return NULL;
	}
	plog("connected_fd=%d", connected_fd);
	
	int serial_fd = -1;
	open_port(&serial_fd, DEVICE);

	FD_ZERO(&rfdset);
	FD_ZERO(&errorfdset);
	FD_SET(connected_fd, &rfdset);
	FD_SET(connected_fd, &errorfdset);
	
	timeout.tv_sec = 3L;
	timeout.tv_usec = 0L;

	while(1) {
		plog("read to select ...");
		ret = select(connected_fd + 1, &rfdset, NULL, &errorfdset, NULL);
		if (ret < 1)
		{
			plog("select error #");
			perror("select error\n");
			break;
		}
		else if(ret == 0)
		{
			plog("select timeout");
			continue;
		}
		plog("select ret = %d",ret);

		if(FD_ISSET(connected_fd, &rfdset) || FD_ISSET(connected_fd, &errorfdset))
		{
			plog("connected_fd=%d, is in use ##########----------------->>>>>\n", connected_fd);
			ret = -1;
			// get socket error
			int len = sizeof(ret);
			getsockopt(connected_fd, SOL_SOCKET, SO_ERROR, (void*)&ret, &len);
			
			if(ret != 0) {
				plog("%s, client's socket timeout!!!\n", strerror(errno));
				close(connected_fd);
				connected_fd = -1;
				break;
			}
// >>>>>>>>>>>>>>>>>>>>>>>>>>			
			memset(buffer, 0, MAX_BUFFER_SIZE);
			count = recv(connected_fd, buffer, MAX_BUFFER_SIZE, MSG_DONTWAIT);
			if(count == 0)
				break;

			plog("--> recv buffer count=%d", count);
			pbuffer(buffer, count);
			if(buffer[0] == 0xFF && buffer[1] > 250)
			{
				plog("Invalid Length, must be [0 ~ 250]");
				continue;
			}
			
			count = write2port(serial_fd, buffer, count);
			if(count < 0)
			{
				perror("write2port error\n");
			}
			plog("write2port done");

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<
			memset(buffer, 0, MAX_BUFFER_SIZE);
			//count = read_from_port(serial_fd, buffer, MAX_BUFFER_SIZE);
			count = read_from_port(serial_fd, buffer, MAX_BUFFER_SIZE);
			if(count == 0)
			{
				//plog("read_from_port 0000");
				break;
			}
			else if(count < 0)
			{
				perror("read_from_port error!");
				continue;
			}
			plog("<-- read_from_port buffer count=%d", count);
			pbuffer(buffer, count);

			count = tcp_sendBytes(connected_fd, buffer, count);
			if(count < 0)
			{
				perror("tcp_sendBytes error\n");
			}
			plog("tcp_sendBytes count=%d", count);

			plog("#################");
		}
	}
	plog("tcp_serial_transport end");
	close_port(serial_fd);
}

int tcp_sendBytes(int fd, uint8_t* message, uint32_t length)
{
	plog("tcp_sendBytes buffer = %p,%p,%p,%p,%p, %d",message[0],message[1],message[2],message[3],message[4], length);
	if(fd < -1)
		return -1;
	int ret = -1;
	
  	do 
  	{
		ret = send(fd, message, length, MSG_NOSIGNAL | MSG_DONTWAIT);
		//plog("tcp send : %d\n", ret);
    	if (ret < 0)
    	{
      		return -1;
    	}
    	length -= ret;
    	message += ret;
  	}
 	 while (length > 0);

  	return ret;
}

int anetKeepAlive(int fd, int interval)
{
	int val = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) < 0)
		plog("setsockopt SO_KEEPALIVE error!!!\n");

	val = interval;
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0)
		plog("setsockopt TCP_KEEPIDLE error!!!\n");

	val = 3;
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0)
		plog("setsockopt TCP_KEEPINTVL error!!!\n");	
	
	val = 2;
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0)
		plog("setsockopt TCP_KEEPCNT error!!!\n");

	plog("anetKeepAlive success!\n");
	return 0;

}
