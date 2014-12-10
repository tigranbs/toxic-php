#ifndef TOXIC_H
#define TOXIC_H
#include <sapi/embed/php_embed.h>
#include <zend_stream.h>
#include <SAPI.h>
#include <stdio.h>
#include "php-5.6.0/ext/spl/spl_functions.h"

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

#endif // TOXIC_H
