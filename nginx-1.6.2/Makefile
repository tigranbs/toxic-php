
default:	build

clean:
	rm -rf Makefile objs

build:
	$(MAKE) -f objs/Makefile
	$(MAKE) -f objs/Makefile manpage

install:
	$(MAKE) -f objs/Makefile install

upgrade:
	../nginx_bin/sbin/nginx -t

	kill -USR2 `cat ../nginx_bin/logs/nginx.pid`
	sleep 1
	test -f ../nginx_bin/logs/nginx.pid.oldbin

	kill -QUIT `cat ../nginx_bin/logs/nginx.pid.oldbin`
