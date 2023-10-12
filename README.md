# hook-https
A small PoC demonstrating TLS interception on both HTTP1 and HTTP2.

Greatly inspired by https://github.com/sebcat/openssl-hook

## Building
Makefile has all batteries included.

```
$ make             # makes the library
$ make test-all    # makes some sample curl calls while hooked
$ make test-http1  # makes a http1 over TLS curl call while hooked
$ make test-http2  # makes a http2 over TLS curl call while hooked
```
