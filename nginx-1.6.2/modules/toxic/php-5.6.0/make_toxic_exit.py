#!/usr/bin/python

import os
import re
pattern = r'[^a-zA-Z0-9_](exit)[(]'
toxic_exit_function = """ 
#ifndef TOXIC_EXIT
#define TOXIC_EXIT

static void (*ngx_toxic_exit) (int status);
int toxic_exit(int status);

#endif
 """ 

zend_c = """

int toxic_exit(int status)
{
    if(ngx_toxic_exit)
        ngx_toxic_exit(status);
    else
        exit(status);
    return 1;
}

"""  
for dname, dirs, files in os.walk("."):
    files = [ fi for fi in files if fi.endswith(".c") or fi.endswith(".h") ]
    for fname in files:
        fpath = os.path.join(dname, fname)
        with open(fpath) as f:
            s = f.read()
        s = re.sub(r'([^a-zA-Z0-9_])(exit)([(])', r'\1toxic_exit\3', s)
        with open(fpath, "w") as f:
            f.write(s)
        print fpath

print "Adding function to Zend/zend.h"
with open('Zend/zend.h', 'a') as f:
    f.write(toxic_exit_function)

print "Adding function to Zend/zend.c"
with open('Zend/zend.c', 'a') as f:
    f.write(zend_c)