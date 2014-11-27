#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <sapi/embed/php_embed.h>
#include <zend_stream.h>
#include <SAPI.h>
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
} toxic_request_callback;

typedef struct {
    int done;
    int again;
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
//        zval *str_z;
//        MAKE_STD_ZVAL(str_z);
//        ZVAL_STRING(str_z, str, str_length);

//        args[0] = &str_z;
//        call_user_function_ex(EG(function_table), &obj, &funcname, &retval, 1, args, 0, NULL TSRMLS_CC);
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


static char *ngx_http_toxic(ngx_conf_t *cf, void *post, void *data);


static ngx_conf_post_handler_pt ngx_http_toxic_p = ngx_http_toxic;

/*
 * The structure will holds the value of the
 * module directive toxic
 */
typedef struct {
    ngx_str_t   name;
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
static void ngx_http_toxic_body_handler ( ngx_http_request_t *r );

ngx_int_t  ngx_http_toxic_phase_handler (ngx_http_request_t *r ) {

        ngx_int_t rc = NGX_OK;
    if(r->request_body == NULL) {
         // Getting POST again
            rc = ngx_http_read_client_request_body(r,  ngx_http_toxic_body_handler   );

        if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
            return rc;
        }

        return NGX_DONE;
    }

    // body finished
     if(r->request_body->rest) {
        return NGX_DONE;
    }
     return rc;
}


static void ngx_http_toxic_body_handler ( ngx_http_request_t *r ) {
        ngx_int_t rc = NGX_OK;

        rc = ngx_http_toxic_phase_handler ( r );
        if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
            ngx_http_finalize_request(r,0);
        }
        return;
}


/*
 * Main handler function of the module.
 */
static ngx_int_t
ngx_http_toxic_handler(ngx_http_request_t *r)
{
    char * base_str;
    int base_len = 0;
    SG(headers_sent) = 0;

    void callback_output(const char *str, unsigned int len) {
        if(base_len > 0)
        {
            base_str = (char*)realloc(base_str, sizeof(char) * (len + base_len));
            strncat(base_str, str, len);
        }
        else
        {
            base_str = (char*)malloc(sizeof(char) * len);
//            strncpy(base_str, str, len);
            unsigned int i;
            for(i=0;i<len;i++)
            {
                base_str[i] = str[i];
            }
            base_str[len] = '\0';
        }
        base_len += len;
    };
    ngx_int_t    rc;
//    ngx_buf_t   *b;
//    ngx_chain_t  out;
//    ngx_str_t ret_data;

    if ((r->method & (NGX_HTTP_POST|NGX_HTTP_PUT))) {

        rc = ngx_http_read_client_request_body(r,ngx_http_toxic_body_handler);
        if (rc == NGX_ERROR || rc >= NGX_HTTP_SPECIAL_RESPONSE) {
                return rc;
        }

        zval **post, *post_data, **parse_post_args[2], parse_post_function, *post_retval;
        parse_post_function = out_function("parse_str");
        MAKE_STD_ZVAL(post_data);
        ngx_buf_t * buf;
        ngx_chain_t * chain;
        char *post_body= "";
        int post_len=0;
        chain = r->request_body->bufs;
        while (chain) {
            buf = chain->buf;
            int len = buf->end - buf->start;
            if(post_len > 0)
            {
                post_body = (char*)realloc(post_body, sizeof(char) * (len + post_len));
                strncat(post_body, (char *)buf->start, len);
            }
            else
            {
                post_body = (char*)malloc(sizeof(char) * len);
    //            strncpy(base_str, str, len);
                int i;
                for(i=0;i<len;i++)
                {
                    post_body[i] = buf->start[i];
                }
                post_body[len] = '\0';
            }
            post_len += len;
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
    }



    toxic_request_callback output;
    output.callback = callback_output;
    output.key = toxic_random_string(10);

    callbacks[0] = output;

//    if (r->method != NGX_HTTP_HEAD) {
        zval *ret_val, *url_arg, *request_index_arg;
        MAKE_STD_ZVAL(url_arg);
        MAKE_STD_ZVAL(request_index_arg);
        int k;
        url_arg->value.str.val = malloc(sizeof(char) * r->uri.len);
        for(k=0;k<(int)r->uri.len; k++)
        {
            url_arg->value.str.val[k] = (char)r->uri.data[k];
        }
        url_arg->value.str.val[r->uri.len] = '\0';
        url_arg->value.str.len = (int)r->uri.len;
        url_arg->type = IS_STRING;
//        ZVAL_STRING(url_arg, (char *)r->uri.data, r->uri.len);
        ZVAL_STRINGL(request_index_arg, output.key, strlen(output.key), 0);
        start_args[0] = &url_arg;
        start_args[1] = &request_index_arg;

        call_user_function_ex(EG(function_table), &obj, &start_function_name, &ret_val, 2, start_args, 0, NULL TSRMLS_CC);

        zend_eval_string("$_POST = array();", ret_val, "Cleen");
        sapi_headers_struct *sapi_headers;
        sapi_headers = &SG(sapi_headers);
        r->headers_out.content_type_len = sizeof(sapi_headers->mimetype) - 1;
        r->headers_out.content_type.data = (u_char *) sapi_headers->mimetype;
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = base_len;
        ngx_http_send_header(r);


        ngx_buf_t   *b;
        ngx_chain_t  out;
        b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
        if (b == NULL) {
            return 0;
        }
        out.buf = b;
        out.next = NULL;

        /* adjust the pointers of the buffer */
        b->pos = (u_char*)base_str;
        b->last = (u_char*)base_str + base_len;
        b->memory = 1;    /* this buffer is in memory */
        b->last_buf = 1;  /* this is the last buffer in the buffer chain */

        /* send the buffer chain of your response */



    return ngx_http_output_filter(r, &out);
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
