# A Shitty Little Webserver

### Pros:

1. It's so shitty!
2. Hello World
3. SO_REUSEPORT
4. ASYNC Polling

### Cons:

1. I am not sane.
2. It's not protable, sorry FreeBSD
3. It's still in heavy development

## Installation

### Prerequisites

1. Linux - early decision to not be portable
2. Kernel 3.9 or later
3. zlib, pthreads, and joyent http-parser

### Commands

	./bootstrap
	./configure
	make
	make install

## TODO

1. Better error handling (logging and errno)
2. Logging (http requests)
3. Daemonization
4. Config file
5. Getopts
6. Better caching and keep-alive
7. Metrics framework and endpoint
