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
#include "config/config_file.h"


conf_t server_c;

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

int uri_cb(http_parser *p, const char *url, size_t len)
{
	char *url_p = malloc(len + 1);
	bzero(url_p, len + 1);
	strncpy(url_p, url, len);
	logger(LOG_NOTICE, "url: %s", url_p);
	free(url_p);

	return 0;
}

int message_begin_cb(http_parser *p)
{
	logger(LOG_NOTICE, "message recieved");
	return 0;
}
int message_complete_cb(http_parser *p)
{
	logger(LOG_NOTICE, "message complete");
	return 0;
}
int headers_complete_cb(http_parser *p)
{
	logger(LOG_NOTICE, "headers complete");
	return 0;
}
int header_field_cb(http_parser *p,  const char *h, size_t len)
{
	char *field = malloc(len + 1);
	bzero(field, len + 1);
	strncpy(field, h, len);
	logger(LOG_NOTICE, "header field: %s", field);
	free(field);
	return 0;
}
int header_value_cb(http_parser *p,  const char *hv, size_t len)
{
	char *field = malloc(len + 1);
	bzero(field, len + 1);
	strncpy(field, hv, len);
	logger(LOG_NOTICE, "header value: %s", field);
	free(field);
	return 0;
}
int body_cb(http_parser *p,  const char *body, size_t len)
{
	char *field = malloc(len + 1);
	bzero(field, len + 1);
	strncpy(field, body, len);
	logger(LOG_NOTICE, "message body: %s", field);
	free(field);
	return 0;
}

static struct http_parser_settings settings = {
	.on_message_begin    = message_begin_cb,
	.on_message_complete = message_complete_cb,
	.on_headers_complete = headers_complete_cb,
	.on_header_field     = header_field_cb,
	.on_header_value     = header_value_cb,
	.on_url              = uri_cb,
	.on_body             = body_cb
};

int peer_helper(int fd, long tid, http_parser *parser)
{
	char buffer[MAXMSG];
	bzero(buffer, MAXMSG);
	int nbytes  = recv(fd, buffer, MAXMSG, 0);
	if (nbytes < 0) {
		logger(LOG_ERR, "error reading socket: %s", strerror(errno));
		exit (EXIT_FAILURE);
	} else if (nbytes == 0) {
		/* End-of-file. */
		return -1;
	} else {
		if (http_parser_execute(parser, &settings, buffer, nbytes) != nbytes)
			logger(LOG_ERR, "failed to fully parse request");
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
	serv_addr.sin_port        = htons(server_c.port);
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

	http_parser *parser = malloc(sizeof(http_parser));
	if (parser == NULL)
		_ERROR("Could not allocate parser");

	http_parser_init(parser, HTTP_REQUEST);

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
					if (!peer_helper(events[n].data.fd, tid, parser))
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
	server_c.conf         = DEFAULT_CONF_FILE;
	server_c.port         = DEFAULT_PORT;
	server_c.pid          = DEFAULT_PID_FILE;
	server_c.workers      = (int) sysconf(_SC_NPROCESSORS_ONLN);
	server_c.verbose      = 0;
	server_c.daemonize    = 1;
	server_c.uid          = DEFAULT_USER;
	server_c.gid          = DEFAULT_USER;
	server_c.log.type     = DEFAULT_LOG_TYPE;
	server_c.log.level    = DEFAULT_LOG_LEVEL;
	server_c.log.facility = DEFAULT_LOG_FACILITY;

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
			server_c.verbose++;
			break;

		case 'F':
			server_c.daemonize = 0;
			break;

		case 'c':
			server_c.conf = strdup(optarg);
			break;

		case 'p':
			server_c.pid = strdup(optarg);
			break;

		case 'u':
			server_c.uid = strdup(optarg);
			break;

		case 'g':
			server_c.gid = strdup(optarg);
			break;

		default:
			fprintf(stderr, "unhandled option flag %#02x\n", c);
			return 1;
		}
	}
	if (access(server_c.conf, F_OK) != -1) {
		if ((parse_config_file(&server_c, server_c.conf)) == -1) {
			fprintf(stderr, "error parsing config file (%s), %s\n", server_c.conf, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else if (strcmp(server_c.conf, DEFAULT_CONF_FILE) != 0) {
			fprintf(stderr, "config file (%s) does not exist\n", server_c.conf);
			exit(EXIT_FAILURE);
	}

	if (server_c.daemonize) {
		log_open(PACKAGE, server_c.log.facility);
		log_level(LOG_ERR + server_c.verbose, NULL);

		mode_t um = umask(0);
		if (daemonize(server_c.pid, server_c.uid, server_c.gid) != 0) {
			fprintf(stderr, "daemonization failed: (%i) %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		umask(um);
	} else {
		log_open(PACKAGE, "console");
		log_level(LOG_ERR + server_c.verbose, NULL);
		if (!freopen("/dev/null", "r", stdin))
			logger(LOG_WARNING, "failed to reopen stdin </dev/null: %s", strerror(errno));
	}
	logger(LOG_NOTICE, "starting %s with %d threads, on port %d",
		PACKAGE, server_c.workers, server_c.port);

	pthread_t threads[server_c.workers];

	for (t=0; t < server_c.workers; t++) {
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
					logger(LOG_NOTICE, "recieved sigint, someone is watching");
				} else if (fdsi.ssi_signo == SIGHUP) {
					logger(LOG_NOTICE, "recieved sighup, refreshing configs");
				} else if (fdsi.ssi_signo == SIGQUIT) {
					logger(LOG_NOTICE, "recieved sigquit, shutting down");
					term_handler();
				} else {
					printf("Read unexpected signal\n");
				}
			}
		}
	}

	return rc;
}
