#include "toxic.h"


char * toxic_random_string(int length)
{
    char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int aplha_length = strlen(alphanum);
    char * ret_val;
    ret_val = (char *) malloc(sizeof(char) * length);
    int i;
    for(i=0;i<length;i++)
    {
        ret_val[i] = alphanum[rand() % aplha_length];
        if(i==0 && isdigit(ret_val[i]))
        {
            i--;
        }
    }
    return ret_val;
}

void first_init()
{
    funcname = out_function("Call_Output");
    start_function_name = out_function("Start");
    retval = NULL;
    obj = NULL;
    args[0] =(zval **) malloc(sizeof(zval **));
    start_args[0] = (zval **) malloc(sizeof(zval **));
}

zval out_function(char * fname)
{
    zval funcname2;
    ZVAL_STRING(&funcname2, fname, 0);
    return funcname2;
}

void toxic_parse_post(ngx_http_request_t *r)
{
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


    //Just to use ngx_toxic_exit function , for compilation, Need to be deleted !!!!!
    if(!r)
    {
        ngx_toxic_exit(0);
        callbacks[0] = *(toxic_request_callback*)malloc(1);
    }
}

void toxic_parse_get(ngx_http_request_t *r)
{
//    r->args.data
    zval **get, *get_data, **parse_get_args[2], parse_get_function, *get_retval;
    parse_get_function = out_function("parse_str");
    MAKE_STD_ZVAL(get_data);
    ZVAL_STRINGL(get_data, (char*)r->args.data, r->args.len, 0);

    zend_hash_find(&EG(symbol_table), "_GET", sizeof("_GET"), (void**)&get);
    parse_get_args[0] = (zval **) malloc(sizeof(zval **));
    parse_get_args[0] = &get_data;
    parse_get_args[1] = (zval **) malloc(sizeof(zval **));
    parse_get_args[1] = get;
    call_user_function_ex(EG(function_table), &obj, &parse_get_function, &get_retval, 2, parse_get_args, 0, NULL TSRMLS_CC);
}

void toxic_parse_server_vars(ngx_http_request_t *r)
{
    zval **server;
    zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void**)&server);

    //Adding RQUEST_URI
    zval **request_uri;
    zend_hash_find((*server)->value.ht, "REQUEST_URI", sizeof("REQUEST_URI"), (void**)&request_uri);
    ZVAL_STRINGL((*request_uri), (char*)r->uri.data, r->uri.len, 0);
}
