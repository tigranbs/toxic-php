#ifndef TOXIC_H
#define TOXIC_H
#include <sapi/embed/php_embed.h>
#include <zend_stream.h>
#include <SAPI.h>
#include <stdio.h>
#include "php-5.6.0/ext/spl/spl_functions.h"

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_event.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>    /* POSIX Threads */

char * toxic_random_string(int length);

zval out_function(char * fname);


zval **args[1], funcname, *retval, *obj, start_function_name, **start_args[2];
static int first_time_run;


typedef struct {
    char *key;
    void(*callback)(const char *str, unsigned int str_length);
    void(*header_callback)(sapi_headers_struct *sapi_headers TSRMLS_DC);
    void(*header_function)(sapi_header_line ctr);
} toxic_request_callback;

typedef struct
{
  unsigned done:1;
  unsigned waiting_more_body:1;
  unsigned body_end:1;
  ngx_uint_t phase;
} toxic_ctx;

static toxic_request_callback callbacks[1];

void first_init();

void toxic_parse_server_vars(ngx_http_request_t *r);

void toxic_parse_post(ngx_http_request_t *r);
void toxic_parse_get(ngx_http_request_t *r);
//void toxic_parse_cookie(ngx_http_request_t *r);
//void toxic_parse_session(ngx_http_request_t *r);

#endif // TOXIC_H
