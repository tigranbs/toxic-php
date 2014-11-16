#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <sapi/embed/php_embed.h>
#include <zend_stream.h>

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

//static void toxic_parse_headers(ngx_http_request_t *r)
//{
//    int len = r->header_in->end - r->header_in->start;
//    char str[len+1];
////    int v;
//
////    for(v=0; v<len; v++)
////    {
////        str[v] = *r->header_in->end;
////        r->header_in->end--;
////    }
//
//    strncpy(str, "content-type: text/html; charset=UTF-8\r\n", len);
//
//    zval **http_args[1], http_funcname, *retval=NULL, *http_obj=NULL;
//    http_args[0] =(zval **) malloc(sizeof(zval **));
//    ZVAL_STRING(&http_funcname, "http_parse_headers", 0);
//    zval *http_str_z;
//    MAKE_STD_ZVAL(http_str_z);
//    ZVAL_STRING(http_str_z, (char*)str, len);
//    http_args[0] = &http_str_z;
//    call_user_function_ex(EG(function_table), &http_obj, &http_funcname, &retval, 1, http_args, 0, NULL TSRMLS_CC);
//}


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
    void callback_output(const char *str, unsigned int len) {
        ngx_chain_t  out;
        ngx_buf_t   *b;
        b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
        if (b == NULL) {
            return;
        }

        /* attach this buffer to the buffer chain */
        out.buf = b;
        out.next = NULL;

        /* adjust the pointers of the buffer */
        b->pos = (u_char*)str;
        b->last = (u_char*)str + len;
        b->memory = 1;    /* this buffer is in memory */
        b->last_buf = 1;  /* this is the last buffer in the buffer chain */

        /* set the status line */
        r->headers_out.status = NGX_HTTP_OK;

        /* send the buffer chain of your response */
        ngx_http_output_filter(r, &out);
    };
    ngx_int_t    rc;
//    ngx_buf_t   *b;
//    ngx_chain_t  out;
//    ngx_str_t ret_data;

    rc = ngx_http_read_client_request_body(r,ngx_http_toxic_body_handler);

    if (rc == NGX_ERROR || rc >= NGX_HTTP_SPECIAL_RESPONSE) {
            return rc;
    }


    toxic_request_callback output;
    output.callback = callback_output;
    output.key = "0123456789";

    callbacks[0] = output;

//    if (r->method != NGX_HTTP_HEAD) {
        zval *ret_val, *url_arg, *request_index_arg;
        MAKE_STD_ZVAL(url_arg);
        MAKE_STD_ZVAL(request_index_arg);
        ZVAL_STRING(url_arg, (char *)r->uri.data, strlen((const char *)r->uri.data));
        ZVAL_STRING(request_index_arg, output.key, strlen(output.key));
        start_args[0] = &url_arg;
        start_args[1] = &request_index_arg;
        zval **post;

        zend_hash_find(&EG(symbol_table), "_POST", sizeof("_POST"), (void**)&post);
        add_assoc_stringl_ex(*post, (const char*) "post_var12", 10,(char*)r->request_body->bufs->buf->start, strlen((const char*)r->request_body->bufs->buf->start), 0);
//        zend_eval_string(new_call_text, &ret_val, post_variable TSRMLS_CC);
        call_user_function_ex(EG(function_table), &obj, &start_function_name, &ret_val, 2, start_args, 0, NULL TSRMLS_CC);

//        if (Z_STRLEN(*ret_val) > 0) {
//            ret_data.data = (u_char*)Z_STRVAL(ret_val[0]);
//            ret_data.len = Z_STRLEN(ret_val[0]);

//        } else {
//            ret_data.data = (u_char*)"$output is not defined.";
//            ret_data.len = strlen("$output is not defined.");
//        }
//        zend_rebuild_symbol_table();
//    }
//    else
//    {
//        ret_data.data = (u_char*)"12345";
//        ret_data.len = 5;
//    }

    return 0;
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
