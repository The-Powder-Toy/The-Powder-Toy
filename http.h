#ifndef HTTP_H
#define HTTP_H

void http_init(char *proxy);
void http_done(void);

char *http_simple_get(char *uri, int *ret, int *len);
char *http_simple_post(char *uri, char *data, int dlen, int *ret, int *len);

void *http_async_req_start(void *ctx, char *uri, char *data, int dlen, int keep);
void http_async_add_header(void *ctx, char *name, char *data);
int http_async_req_status(void *ctx);
void http_async_get_length(void *ctx, int *total, int *done);
char *http_async_req_stop(void *ctx, int *ret, int *len);
void http_async_req_close(void *ctx);

char *http_multipart_post(char *uri, char **names, char **parts, int *plens, char *user, char *pass, int *ret, int *len);

char *http_ret_text(int ret);

#endif
