#include "httpd.h"

const char * time_s(void)
{
	static char rs_t[200];
	time_t t;
	struct tm *tmp;

	t = time(NULL);
	tmp = gmtime(&t);
	if (tmp == NULL) {
		perror("gmtime");
		exit(EXIT_FAILURE);
	}

	if (strftime(rs_t, sizeof(rs_t), "%a, %d %b %Y %T GMT", tmp) == 0)
		fprintf(stderr, "strftime returned 0\n");

	return rs_t;
}

void s_content(int fd, char *msg)
{
	char s_msg[MAXMSG];

	if(sprintf(s_msg,
			"HTTP/1.1 200 OK\r\n"
			"Connection: Close\r\n"
			"Server: %s\r\n"
			"Content-Type: text/html; charset=utf-8\r\n"
			"Date: %s\r\n"
			"Content-Length: %d\r\n\r\n",
				SERVER_NAME, time_s(), (int) strlen(msg)) == -1)
		fprintf(stderr, "borked building the headers\n");

	strcat(s_msg, msg);
	fprintf(stderr, "the send msg(%d) is:\n'%s'", (int) strlen(s_msg), s_msg);
	int rc_s = write(fd, s_msg, strlen(s_msg));
	if (rc_s != strlen(s_msg))
		fprintf(stderr, "there was an issue sending the content\n");
}

int peer_helper(int fd)
{
	char buffer[MAXMSG];
	bzero(buffer, MAXMSG);
	int nbytes;

	nbytes = read(fd, buffer, MAXMSG);
	if (nbytes < 0) {
		/* Read error. */
		perror ("read");
		exit (EXIT_FAILURE);
	} else if (nbytes == 0) {
		/* End-of-file. */
		return -1;
	} else {
		/* Data read. */
		fprintf (stderr, "Server: got message: `%s'\n", buffer);
		s_content(fd, "<!DOCTYPE html>"
			"<html lang=\"en\">"
			"<head>"
			"<meta charset=\"UTF-8\"/>"
			"<title>Hello World!</title>"
			"</head>"
			"<body>"
			"Hello World!"
			"</body>"
			"</html>");
		return 0;
	}
}

int main(void)
{
	struct sockaddr_in serv_addr, peer_addr;

	socklen_t peer_addr_size = sizeof(struct sockaddr_in);

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(PORT);
	inet_aton("0.0.0.0", &serv_addr.sin_addr);

	int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
	int optval = 1;
	setsockopt(serv_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	listen(serv_fd, SOMAXCONN);

	struct epoll_event ev, events[SOMAXCONN];
	int nfds, epollfd, n, peer_fd;

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = serv_fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serv_fd, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}
	for (;;) {
		nfds = epoll_wait(epollfd, events, SOMAXCONN, -1);
		if (nfds == -1) {
			perror("epoll_pwait");
			exit(EXIT_FAILURE);
		}

		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == serv_fd) {
				peer_fd = accept(serv_fd, (struct sockaddr *) &peer_addr, &peer_addr_size);
				if (peer_fd == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				int fl = fcntl(peer_fd, F_GETFL);
				fcntl(peer_fd, F_SETFL, fl|O_NONBLOCK);

				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = peer_fd;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, peer_fd, &ev) == -1) {
					perror("epoll_ctl: peer_fd");
					exit(EXIT_FAILURE);
				}
			} else {
				if (!peer_helper(events[n].data.fd))
					close(events[n].data.fd);
			}
		}
	}

	close(serv_fd);
	return 0;
}
