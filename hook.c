#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <nghttp2/nghttp2.h>

#define _CONSTRUCTOR __attribute__((constructor))
#define _DESTRUCTOR __attribute__ ((destructor))

#define PRINT_TARGET stderr

#define LOADORDIE(var, name) \
	do {\
		const char *err; \
		(var) = dlsym(RTLD_NEXT, (name)); \
		fprintf(PRINT_TARGET, "Hooked %s at 0x%X\n", (name), (var)); \
		if ((err = dlerror()) != NULL) { \
			fprintf(stderr, "dlsym %s: %s\n", (name), err); \
			exit(EXIT_FAILURE); \
		} \
	} while(0)

struct hook_struct {
	int (*SSL_read)(void *ssl, void *buf, int num);
	int (*SSL_write)(void *ssl, const void *buf, int num);
	int32_t (*nghttp2_submit_request)(nghttp2_session *session, const nghttp2_priority_spec *pri_spec, const nghttp2_nv *nva, size_t nvlen, const nghttp2_data_provider *data_prd, void *stream_user_data);

};

static struct hook_struct HOOK_TABLE;


void _CONSTRUCTOR hook_init(void) {
	dlerror();
	LOADORDIE(HOOK_TABLE.SSL_read, "SSL_read");
	LOADORDIE(HOOK_TABLE.SSL_write, "SSL_write");
	LOADORDIE(HOOK_TABLE.nghttp2_submit_request, "nghttp2_submit_request");
}

void _DESTRUCTOR hook_fini(void) {
	fprintf(PRINT_TARGET, "Library unloaded...\n");
}

/* Prints n bytes from mem */
/* This is probably pretty slow but it looks nicer when printed */
static void memprint(unsigned char *mem, int n) {
	for (int i = 0; i < n; i++) {
		if (isprint(mem[i])) {
			fprintf(PRINT_TARGET, "%c", mem[i]);
		} else if (mem[i]) {
			fprintf(PRINT_TARGET, "\\x%02X", mem[i]);
		}
	}
}

int SSL_read(void *ssl, void *buf, int num) {
	int ret;
	
	/* Hook post-call */
	ret = HOOK_TABLE.SSL_read(ssl, buf, num);
	if (ssl != NULL && buf != NULL && ret > 0) {
		fprintf(PRINT_TARGET, "SSL_read hook:\n");
		memprint(buf, num);
		fprintf(PRINT_TARGET, "\n\n");
	}

	return ret;
}

int SSL_write(void *ssl, void *buf, int num) {
	/* Hook pre-call */
	if (ssl != NULL && buf != NULL) {
		fprintf(PRINT_TARGET, "SSL_write hook:\n");
		memprint(buf, num);
		fprintf(PRINT_TARGET, "\n\n");
	}

	return HOOK_TABLE.SSL_write(ssl, buf, num);
}

int32_t nghttp2_submit_request(nghttp2_session *session,
							   const nghttp2_priority_spec *pri_spec,
							   const nghttp2_nv *nva, size_t nvlen,
							   const nghttp2_data_provider *data_prd,
							   void *stream_user_data) {

	fprintf(PRINT_TARGET, "nghttp2_submit_request hook:\n");
	for (int i = 0; i < nvlen; i++) {
		const nghttp2_nv *field = &nva[i];
		memprint(field->name, field->namelen);
		memprint((unsigned char *)" ", 1);
		memprint(field->value, field->valuelen);
		fprintf(PRINT_TARGET, "\n");
	}
	fprintf(PRINT_TARGET, "\n");
	return HOOK_TABLE.nghttp2_submit_request(session, pri_spec, nva, nvlen, data_prd, stream_user_data);
}
