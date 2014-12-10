#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_event.h>

#include <toxic.h>

zval out_function(char * fname)
{
    zval funcname2;
    ZVAL_STRING(&funcname2, fname, 0);
    return funcname2;
}

zval **args[1], funcname, *retval, *obj, start_function_name, **start_args[2];
int first_time_run=1;


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
} toxic_ctx;

static toxic_request_callback callbacks[10];

void first_init()
{
    funcname = out_function("Call_Output");
    start_function_name = out_function("Start");
    retval = NULL;
    obj = NULL;
    args[0] =(zval **) malloc(sizeof(zval **));
    start_args[0] = (zval **) malloc(sizeof(zval **));
}

static int toxic_output(const char *request_index, unsigned int request_index_length, const char *str, unsigned int str_length TSRMLS_DC)
{
    if(request_index)
    {
        int i;
        for(i=0;i<10;i++)
        {
            if(callbacks[i].key)
            {
                if(strcmp(request_index, callbacks[i].key) == 0)
                {
                    callbacks[i].callback(str, str_length);
                    return str_length;
                }
            }

        }
    }

        return str_length;
}


static int toxic_send_header(sapi_headers_struct *sapi_headers TSRMLS_DC)
{
    callbacks[0].header_callback(sapi_headers);
    return SAPI_HEADER_SENT_SUCCESSFULLY;
}

static void toxic_header_function(sapi_header_line ctr)
{
    callbacks[0].header_function(ctr);
}

static char *ngx_http_toxic(ngx_conf_t *cf, void *post, void *data);


static ngx_conf_post_handler_pt ngx_http_toxic_p = ngx_http_toxic;

/*
 * The structure will holds the value of the
 * module directive toxic
 */
typedef struct {
    ngx_str_t   name;
    ngx_http_upstream_conf_t  upstream;
} ngx_http_toxic_loc_conf_t;

/* The function which initializes memory for the module configuration structure
 */
static void *
ngx_http_toxic_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_toxic_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_toxic_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}

/*
 * The command array or array, which holds one subarray for each module
 * directive along with a function which validates the value of the
 * directive and also initializes the main handler of this module
 */
static ngx_command_t ngx_http_toxic_commands[] = {
    { ngx_string("toxic"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_toxic_loc_conf_t, name),
      &ngx_http_toxic_p },

    ngx_null_command
};


static ngx_str_t toxic_string;
//static char variables[1000][10];

/*
 * The module context has hooks , here we have a hook for creating
 * location configuration
 */
static ngx_http_module_t ngx_http_toxic_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_toxic_create_loc_conf, /* create location configuration */
    NULL                           /* merge location configuration */
};


/*
 * The module which binds the context and commands
 *
 */
ngx_module_t ngx_http_toxic_module = {
    NGX_MODULE_V1,
    &ngx_http_toxic_module_ctx,    /* module context */
    ngx_http_toxic_commands,       /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_http_toxic_handler(ngx_http_request_t *r);


//static unsigned int check_headers(char *header_str)
//{
//    if(strstr(header_str, "Content-Type") == NULL) return 1;
//    return 0;
//}

static ngx_int_t toxic_excecute(ngx_http_request_t *r, char * content_type)
{
//    char * base_str;
    ngx_chain_t *out_chain;
    out_chain = ngx_pcalloc(r->pool, sizeof(ngx_chain_t));
    int base_len = 0;
    SG(headers_sent) = 0;
    SG(callback_run) = 0;
    SG(request_info).no_headers = 0;

    void callback_output(const char *str, unsigned int len) {
        if(len <= 0) return;
        if(out_chain->buf)
        {
            out_chain->buf->last_buf = 0;
            out_chain->next = (ngx_chain_t*)malloc(sizeof(ngx_chain_t));
            out_chain = out_chain->next;
        }
        ngx_buf_t   *b;
        b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
        if (b == NULL) {
            return;
        }
        out_chain->buf = b;
        b->pos = (u_char*)malloc(sizeof(u_char) * len+1);
        unsigned int i;
        for(i=0;i<len;i++)
        {
            b->pos[i] = (u_char)str[i];
        }
        b->pos[i] = '\0';
        b->last = b->pos + len + 1;
        b->memory = 1;    /* this buffer is in memory */
        b->last_in_chain = 1;
        b->last_buf = 1;
        b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
        b->file->directio = 0;
        b->in_file = 0;

        base_len += len;
        out_chain->next = NULL;
    };


    void header_callback(sapi_headers_struct *sapi_headers TSRMLS_DC) {
//        sapi_headers_struct *sapi_headers;
        if(sapi_headers->mimetype)
        {
            r->headers_out.content_type_len = strlen(sapi_headers->mimetype);
            r->headers_out.content_type.data = (u_char *)malloc(sizeof(u_char) * r->headers_out.content_type_len);
            r->headers_out.content_type.len = r->headers_out.content_type_len;
            strncpy((char*)r->headers_out.content_type.data, sapi_headers->mimetype, r->headers_out.content_type_len);
            r->headers_out.status = sapi_headers->http_response_code;

            ngx_table_elt_t  *cc, **ccp;

            ccp = r->headers_out.cache_control.elts;

            if (ccp == NULL) {

                if (ngx_array_init(&r->headers_out.cache_control, r->pool,
                                   1, sizeof(ngx_table_elt_t *))
                    != NGX_OK)
                {
                    return;
                }
            }

            ccp = ngx_array_push(&r->headers_out.cache_control);
            if (ccp == NULL) {
                return;
            }

            cc = ngx_list_push(&r->headers_out.headers);
            if (cc == NULL) {
                return;
            }

            cc->hash = 1;
            ngx_str_set(&cc->key, "Cache-Control");
            ngx_str_set(&cc->value, "private");

            *ccp = cc;


            efree(sapi_headers->mimetype);
        }
        else
        {
            r->headers_out.content_type_len = strlen("text/html");
            r->headers_out.content_type.data = (u_char *)malloc(sizeof(u_char) * r->headers_out.content_type_len);
            r->headers_out.content_type.len = r->headers_out.content_type_len;
            strncpy((char*)r->headers_out.content_type.data, "text/html", r->headers_out.content_type_len);
            r->headers_out.status = sapi_headers->http_response_code;
        }
    }

    void header_function (sapi_header_line ctr)
    {
        ngx_table_elt_t  *h;

        if (ctr.line) {
            if(strstr(ctr.line, "Content-type") != NULL) return;
            if(strstr(ctr.line, "Content-Len") != NULL) return;
            if(strstr(ctr.line, "Cache") != NULL) return;
            int i, key_index=0;
            for(i=0;i < (int)ctr.line_len; i++)
            {
                if(ctr.line[i] == ':')
                {
                    key_index = i;
                    break;
                }
            }
            if(key_index == 0) return;
            h = ngx_list_push(&r->headers_out.headers);
            if (h == NULL) {
                return;
            }
            h->key.len = key_index;
            h->key.data = (u_char *)ngx_alloc(sizeof(u_char) * h->key.len, NULL);
            if(!h->key.data) return;
            h->value.len = (ctr.line_len - 1) - h->key.len;
            if(h->value.len <= 0) return;
            h->value.data = (u_char *)ngx_alloc(sizeof(u_char) * h->value.len, NULL);
            if(!h->value.data) return;
            strncpy((char*)h->key.data, ctr.line, h->key.len);
            strncpy((char*)h->value.data, ctr.line + h->key.len + 1, h->value.len);
        }
    }

    toxic_request_callback output;
    output.callback = callback_output;
    output.header_callback = header_callback;
    output.header_function = header_function;
    output.key = toxic_random_string(10);

    callbacks[0] = output;

        zval *ret_val, *url_arg, *request_index_arg;
        MAKE_STD_ZVAL(url_arg);
        MAKE_STD_ZVAL(request_index_arg);
//        int k;
//        url_arg->value.str.val = malloc(sizeof(char) * r->uri.len);
//        for(k=0;k<(int)r->uri.len; k++)
//        {
//            url_arg->value.str.val[k] = (char)r->uri.data[k];
//        }
//        url_arg->value.str.val[r->uri.len] = '\0';
//        url_arg->value.str.len = (int)r->uri.len;
//        url_arg->type = IS_STRING;
        ZVAL_STRING(url_arg, (char *)r->uri.data, r->uri.len);
        ZVAL_STRINGL(request_index_arg, output.key, strlen(output.key), 0);
        start_args[0] = &url_arg;
        start_args[1] = &request_index_arg;

        call_user_function_ex(EG(function_table), &obj, &start_function_name, &ret_val, 2, start_args, 0, NULL TSRMLS_CC);

        zend_eval_string("$_POST = array();", ret_val, "Cleen");

        r->headers_out.content_length_n = base_len;

        ngx_http_send_header(r);
        /* send the buffer chain of your response */
        if(out_chain->buf)
        {
            ngx_http_output_filter ( r , out_chain );
        }

    return NGX_DONE;
}


static void toxic_post_body_handler(ngx_http_request_t *r)
{
    if ((r->method & (NGX_HTTP_POST|NGX_HTTP_PUT))) {
        zval **post, *post_data, **parse_post_args[2], parse_post_function, *post_retval;
        parse_post_function = out_function("parse_str");
        MAKE_STD_ZVAL(post_data);
        ngx_buf_t * buf;
        ngx_chain_t * chain;
        char *post_body;
        post_body = (char*)malloc(1);
        int post_len=0;
        chain = r->request_body->bufs;
        while (chain) {
            buf = chain->buf;
            int len = buf->end - buf->start;
            if(len > 0)
            {
                char * temp_buf;
                if(post_len == 0)
                {
                    temp_buf = (char*)malloc(len + 1);
                    strncpy(temp_buf, (char*)buf->start, len);
                }
                else
                {
                    temp_buf = (char*)malloc(post_len+len + 1);
                    strncpy(temp_buf, post_body, post_len);
                    strncat(temp_buf, (char*)buf->start, len);
                }
                post_len += len;
//                if(post_len > 0) free(post_body);
                post_body = temp_buf;
            }
            chain = chain->next;
        }
        ZVAL_STRINGL(post_data, post_body, post_len, 0);

        zend_hash_find(&EG(symbol_table), "_POST", sizeof("_POST"), (void**)&post);
        parse_post_args[0] = (zval **) malloc(sizeof(zval **));
        parse_post_args[0] = &post_data;
        parse_post_args[1] = (zval **) malloc(sizeof(zval **));
        parse_post_args[1] = post;
    //        add_assoc_stringl_ex(*post, (const char*)toxic_random_string(10) , 10,(char*)r->request_body->bufs->buf->start, strlen((const char*)r->request_body->bufs->buf->start), 0);
        call_user_function_ex(EG(function_table), &obj, &parse_post_function, &post_retval, 2, parse_post_args, 0, NULL TSRMLS_CC);

        toxic_excecute(r, "text/html");
    }
    else
    {
        toxic_excecute(r, "text/html");
    }
    ngx_http_finalize_request(r, NGX_OK);
}

/*
 * Main handler function of the module.
 */
static ngx_int_t
ngx_http_toxic_handler(ngx_http_request_t *r)
{
    ngx_int_t                   rc;

    rc = ngx_http_read_client_request_body(r,toxic_post_body_handler);

    if (rc == NGX_ERROR || rc >= NGX_HTTP_SPECIAL_RESPONSE) {
            return rc;
    }

    return NGX_OK;
}

/*
 * Function for the directive toxic , it validates its value
 * and copies it to a static variable to be printed later
 */
static char * ngx_http_toxic(ngx_conf_t *cf, void *post, void *data)
{
    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_toxic_handler;

    ngx_str_t  *name = data; // i.e., first field of ngx_http_toxic_loc_conf_t

    if (ngx_strcmp(name->data, "") == 0) {
        return NGX_CONF_ERROR;
    }
    toxic_string.data = name->data;
    toxic_string.len = ngx_strlen(toxic_string.data);
    if(first_time_run == 1)
    {
        printf("Starting Toxic PHP script\n");
        int argc = 0;
        char** argv = NULL;
        first_init();
        php_embed_module.toxic_output = toxic_output;
        php_embed_module.send_headers = toxic_send_header;
        php_embed_module.toxic_header_handler = toxic_header_function;
        php_embed_module.is_toxic = 1;
        php_embed_init(argc, argv PTSRMLS_CC);
        zend_first_try {
            zend_eval_string((char *)name->data, NULL,
                             "Embed 2 Eval'd string" TSRMLS_CC);
        } zend_end_try();
    //    php_embed_shutdown(TSRMLS_C);
        first_time_run = 0;
    }

    return NGX_CONF_OK;
}
