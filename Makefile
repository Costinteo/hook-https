CFLAGS+=-Wall -g
LDFLAGS+=-lc

.PHONY:clean

all: hook.so.1

test-all: test-http1 test-http2

test-http1:
	@echo "----====[ Testing HTTP1 ]====----"
	LD_PRELOAD=./hook.so.1 curl https://example.com -s --http1.1 1>/dev/null 

test-http2:
	@echo "----====[ Testing HTTP2 ]====----"
	LD_PRELOAD=./hook.so.1 curl https://example.com -s 1>/dev/null 

hook.so.1: hook.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< $(LDFLAGS)

clean: 
	rm hook.so.1
