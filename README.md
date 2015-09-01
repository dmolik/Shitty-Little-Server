# A Shitty Little Webserver

### Pros:

1. It's so shitty!
2. Hello World
3. SO_REUSEPORT
4. ASYNC Polling


## Installation

### Prerequisites

1. Linux - early decision to not be portable
2. Kernel 3.9 or later
3. autotools - won't be necerrary with later releases

### Commands

	./bootstrap
	./configure
	make
	make install

## TODO

1. Better error handling (logging and errno)
2. Logging
3. Daemonization
4. Config file
5. Getopts
6. Better caching and keep-alive
