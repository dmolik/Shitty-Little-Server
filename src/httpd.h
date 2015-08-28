#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <sys/epoll.h>

#include <pthread.h>

#define PORT   3000
#define MAXMSG  512

#define SERVER_NAME "shitty little server"

struct t_data {
	int  serv_fd;
	struct sockaddr_in serv_addr;
};

typedef struct t_data cfg_t;

const char * time_s(void);

void s_content(int fd, char *msg);

int peer_helper(int fd, long tid);
