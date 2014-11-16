
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#include <stdio.h>
#include <sapi/embed/php_embed.h>
#include <zend_stream.h>

#include "php-5.6.0/ext/spl/spl_functions.h"

zval out_function(char * fname)
{
    zval funcname2;
    ZVAL_STRING(&funcname2, fname, 0);
    return funcname2;
}

zval **args[1], funcname, *retval, *obj, start_function_name, **start_args[1];
int k=0;


void first_init()
{
    funcname = out_function("Call_Output");
    start_function_name = out_function("Start");
    retval = NULL;
    obj = NULL;
    args[0] =(zval **) malloc(sizeof(zval **));
    start_args[0] = (zval **) malloc(sizeof(zval **));
}


static int capture_output(const char* str, unsigned int len TSRMLS_DC)
{
        zval *str_z;
        MAKE_STD_ZVAL(str_z);
        ZVAL_STRING(str_z, str, len);

        args[0] = &str_z;
        call_user_function_ex(EG(function_table), &obj, &funcname, &retval, 1, args, 0, NULL TSRMLS_CC);
        return len;
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
    ngx_int_t    rc;
    ngx_buf_t   *b;
    ngx_chain_t  out;
//toxic_parse_headers(r);

//    rc = ngx_http_read_client_request_body(r,ngx_http_toxic_body_handler);
//
//    if (rc == NGX_ERROR || rc >= NGX_HTTP_SPECIAL_RESPONSE) {
//            return rc;
//    }


    if (r->method != NGX_HTTP_HEAD) {

//        char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
//        char url_variable[10], post_variable[10];
//        int i,j;
//        for(i=0;i<10;i++)
//        {
//        	url_variable[i] = alphanum[rand() % 10];
//            if(i==0 && isdigit(url_variable[i]))
//            {
//                i--;
//            }
//        }
//
//        for(i=0;i<10;i++)
//        {
//        	post_variable[i] = alphanum[rand() % 10];
//            if(i==0 && isdigit(post_variable[i]))
//            {
//                i--;
//            }
//        }
//
//        char call_text[11] = "Start('-');";
//        int new_char_len = 20;
//        char new_call_text[new_char_len];
//        int new_text_index = 0;
//        for(j=0; j<11;j++)
//        {
//            if(call_text[j] == '-')
//            {
//                for(i=0;i<10;i++)
//                {
//                    new_call_text[new_text_index] = (char)url_variable[i];
//                    new_text_index++;
//                }
//            }
//            else
//            {
//                if(call_text[j] == '*')
//                {
//                    for(i=0;i<10;i++)
//                    {
//                        new_call_text[new_text_index] = (char) post_variable[i];
//                        new_text_index++;
//                    }
//                }
//                else
//                {
//                	new_call_text[new_text_index] = call_text[j];
//                	new_text_index++;
//                }
//            }
//        }

//        zval *url_zend_variable, *post_zend_variable;
//
//        MAKE_STD_ZVAL(url_zend_variable);
////        ZVAL_STRINGL(url_zend_variable, (char *)r->uri.data, r->uri.len, 0);
//        url_zend_variable->type = IS_STRING;
//        url_zend_variable->value.str.val = (char *)r->uri.data;
//        url_zend_variable->value.str.len = strlen((const char *)r->uri.data);
//        ZEND_SET_SYMBOL(&EG(symbol_table), url_variable, url_zend_variable);
//
//        if (r->method & (NGX_HTTP_POST))
//        {
//        	MAKE_STD_ZVAL(post_zend_variable);
//        	post_zend_variable->type = IS_STRING;
//        	post_zend_variable->value.str.val = (char *)r->header_in->pos;
//        	post_zend_variable->value.str.len = strlen((const char*)r->header_in->pos);
//        	ZEND_SET_SYMBOL(&EG(symbol_table), url_variable, post_zend_variable);
//        }
//        else
//        {
//        	MAKE_STD_ZVAL(post_zend_variable);
////        	ZVAL_FALSE(post_zend_variable);
//        	post_zend_variable->type = IS_BOOL;
//        	post_zend_variable->value.lval=0;
//        	ZEND_SET_SYMBOL(&EG(symbol_table), post_variable, post_zend_variable);
//        }

//        zval **post;
//
//        zend_hash_find(&EG(symbol_table), "_POST", sizeof("_POST"), (void**)&post);
//        add_assoc_stringl_ex(*post, (const char*) post_variable, 10,(char*)r->request_body->bufs->buf->start, strlen((const char*)r->request_body->bufs->buf->start), 0);

        zval *ret_val, *url_arg;
        MAKE_STD_ZVAL(url_arg);
        ZVAL_STRING(url_arg, (char *)r->uri.data, r->uri.len);
        start_args[0] = &url_arg;
//        zend_eval_string(new_call_text, &ret_val, post_variable TSRMLS_CC);
        call_user_function_ex(EG(function_table), &obj, &start_function_name, &ret_val, 1, start_args, 0, NULL TSRMLS_CC);

        if (Z_STRLEN(*ret_val) > 0) {
            toxic_string.data = (u_char*) Z_STRVAL(*ret_val);
            toxic_string.len = Z_STRLEN(*ret_val);

//            zend_delete_global_variable(url_variable, 10);
//            zend_delete_global_variable(post_variable, 10);

        } else {
            toxic_string.data = (u_char*)"$output is not defined.";
            toxic_string.len = strlen("$output is not defined.");
        }
        zend_rebuild_symbol_table();
    }


//    /* we response to 'GET' and 'HEAD' requests only */
//    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
//        return NGX_HTTP_NOT_ALLOWED;
//    }



    /* set the 'Content-type' header */
    r->headers_out.content_type_len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";

//    /* send the header only, if the request type is http 'HEAD' */
//    if (r->method == NGX_HTTP_HEAD) {
//        r->headers_out.status = NGX_HTTP_OK;
//        r->headers_out.content_length_n = toxic_string.len;
//
//        return ngx_http_send_header(r);
//    }
//
    /* allocate a buffer for your response body */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* attach this buffer to the buffer chain */
    out.buf = b;
    out.next = NULL;

    /* adjust the pointers of the buffer */
    b->pos = toxic_string.data;
    b->last = toxic_string.data + toxic_string.len;
    b->memory = 1;    /* this buffer is in memory */
    b->last_buf = 1;  /* this is the last buffer in the buffer chain */

    /* set the status line */
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = toxic_string.len;

    /* send the headers of your response */
    rc = ngx_http_send_header(r);



    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

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

    printf("Starting Toxic PHP script\n");
    int argc = 0;
    char** argv = NULL;
    first_init();
    php_embed_module.ub_write = capture_output;
    php_embed_init(argc, argv PTSRMLS_CC);
    zend_first_try {
        zend_eval_string((char *)name->data, NULL,
                         "Embed 2 Eval'd string" TSRMLS_CC);
    } zend_end_try();
//    php_embed_shutdown(TSRMLS_C);

    return NGX_CONF_OK;
}
