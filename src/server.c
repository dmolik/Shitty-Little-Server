/*
 * Copyright 2015 Dan Molik <dan@danmolik.com>
 *
 * This file is part of Shitty Little Server
 *
 * Shitty Little Server is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Shitty Little Server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Shitty Little Server.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "server.h"


const char * time_s(void)
{
	static char rs_t[200];
	time_t t;
	struct tm *tmp;

	t = time(NULL);
	if ((tmp = gmtime(&t)) == NULL)
		_ERROR("failed to allocate gmtime");

	if (strftime(rs_t, sizeof(rs_t), "%a, %d %b %Y %T GMT", tmp) == 0)
		logger(LOG_ERR, "unable to format time, %s", strerror(errno));

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
		logger(LOG_ERR, "unable to build headers, %s", strerror(errno));

	strcat(s_msg, msg);
	int rc_s = send(fd, s_msg, strlen(s_msg), 0);
	if (rc_s != strlen(s_msg))
		logger(LOG_ERR, "failed to send msg, %s", strerror(errno));
}

int peer_helper(int fd, long tid)
{
	char buffer[MAXMSG];
	bzero(buffer, MAXMSG);
	int nbytes = recv(fd, buffer, MAXMSG, 0);
	if (nbytes < 0) {
		logger(LOG_ERR, "error reading socket: %s", strerror(errno));
		exit (EXIT_FAILURE);
	} else if (nbytes == 0) {
		/* End-of-file. */
		return -1;
	} else {
		/* Data read. */
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

void *t_worker(void *t_data)
{
	long tid = (long) t_data;
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size = sizeof(struct sockaddr_in);

	struct sockaddr_in serv_addr;

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_port        = htons(DEFAULT_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
	int optval = 1;
	setsockopt(serv_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	listen(serv_fd, SOMAXCONN);

	struct epoll_event ev, events[SOMAXCONN];
	int nfds, epollfd, n, peer_fd;

	if ((epollfd = epoll_create1(0)) == -1)
		_ERROR("failed to create fd loop in worker");

	ev.events = EPOLLIN;
	ev.data.fd = serv_fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serv_fd, &ev) == -1)
		_ERROR("failed to add listening socket to fd loop");

	for (;;) {
		if ((nfds = epoll_wait(epollfd, events, SOMAXCONN, -1)) == -1)
			_ERROR("catastropic epoll_wait");

		for (n = 0; n < nfds; ++n) {
			if (events[n].events & EPOLLIN) {
				if (events[n].data.fd == serv_fd) {
					if ((peer_fd = accept(serv_fd, (struct sockaddr *) &peer_addr, &peer_addr_size)) == -1)
						_ERROR("unable to accept connections");

					int fl = fcntl(peer_fd, F_GETFL);
					fcntl(peer_fd, F_SETFL, fl|O_NONBLOCK|O_ASYNC);

					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = peer_fd;
					if (epoll_ctl(epollfd, EPOLL_CTL_ADD, peer_fd, &ev) == -1)
						_ERROR("epoll_ctl on peer_fd");

				} else {
					if (!peer_helper(events[n].data.fd, tid))
						close(events[n].data.fd);

					epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
				}
			}
		}
	}

	close(serv_fd);
	return 0;
}

void term_handler(void)
{
	log_close();
	pthread_exit(NULL);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int rc;
	long t;

	struct option long_opts[] = {
		{ "help",             no_argument, NULL, 'h' },
		{ "verbose",          no_argument, NULL, 'v' },
		{ "foreground",       no_argument, NULL, 'F' },
		{ "config",     required_argument, NULL, 'c' },
		{ "pidfile",    required_argument, NULL, 'p' },
		{ "user",       required_argument, NULL, 'u' },
		{ "group",      required_argument, NULL, 'g' },
		{ 0, 0, 0, 0 },
	};
	for (;;) {
		int idx = 1;
		int c = getopt_long(argc, argv, "h?v+Fc:p:u:g:", long_opts, &idx);
		if (c == -1) break;

		switch (c) {
		case 'h':
		case '?':
			printf("%s v%s\n", PACKAGE, VERSION);
			printf("Usage: %s [-h?Fv] [-c /path/to/config]\n"
			       "          [-u user] [-g group] [-p /path/to/pidfile\n\n",
			        PACKAGE);

			printf("Option:\n");
			printf("  -?, -h, --help    show this help screen\n");
			printf("  -F, --foreground  don't daemonize, stay in foreground\n");
			printf("  -v, --verbose     increase debugging\n");

			printf("  -c, --config      file path containing the config\n");

			printf("  -p, --pidfile     where to store the pidfile\n");
			printf("  -u, --user        the user to run as\n");
			printf("  -g, --group       the group to run under\n\n");

			printf("See also: \n  %s\n", PACKAGE_URL);

			exit(EXIT_SUCCESS);

		case 'v':
			// verbose++
			break;

		case 'F':
			break;

		case 'c':
			break;

		case 'p':
			break;

		case 'u':
			break;

		case 'g':
			break;

		default:
			fprintf(stderr, "unhandled option flag %#02x\n", c);
			return 1;
		}
	}

	int num_threads = (int) sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[num_threads];

	log_open(argv[0], "daemon");
	log_level(0, "info");
	logger(LOG_INFO, "starting shitty little server with %d threads", num_threads);

	for (t=0; t < num_threads; t++) {
		if ((rc = pthread_create(&threads[t], NULL, t_worker, (void *)t)))
			_ERROR("failed to create worker thread");
	}

	// signal handling
	sigset_t mask;
	struct epoll_event ev, events[MAX_EVENTS];
	int sfd, nfds, epollfd;

	struct signalfd_siginfo fdsi;
	ssize_t s;

	int n = 0;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGHUP);

	/* Block signals so that they aren't handled
	 * according to their default dispositions */

	if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
		_ERROR("sigprocmask");

	if ((sfd = signalfd(-1, &mask, O_NONBLOCK)) == -1)
		_ERROR("signalfd");

	if ((epollfd = epoll_create1(0)) == -1)
		_ERROR("epoll_create1");

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = sfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sfd, &ev) == -1)
		_ERROR("epoll_ctl sfd");

	for (;;) {
		if ((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1)
			_ERROR("epoll_wait");

		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == sfd) {
				s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
				if (s != sizeof(struct signalfd_siginfo))
					_ERROR("epoll read, main loop");
				if (fdsi.ssi_signo == SIGINT) {
					term_handler();
				} else if (fdsi.ssi_signo == SIGHUP) {
					printf("HUPPED\n");
				} else if (fdsi.ssi_signo == SIGQUIT) {
					printf("Got SIGQUIT\n");
					term_handler();
				} else {
					printf("Read unexpected signal\n");
				}
			}
		}
	}

	return rc;
}
