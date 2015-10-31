# A Shitty Little Webserver

### Pros:

1. It's so shitty!
2. Hello World!
3. SO_REUSEPORT
4. ASYNC Polling

### Cons:

1. It's not portable, sorry FreeBSD
2. It's still in heavy development

## Installation

### Prerequisites

1. Linux - early decision to not be portable
2. Kernel 3.9 or later
3. zlib, pthreads, and the joyent http-parser

### Commands

	./bootstrap
	./configure
	make
	make install

## TODO

1. Logging (http requests)
2. Better caching and keep-alive
3. Metrics framework and endpoint
