objects = main.o request.o response.o \
    utils.o poll.o
sysname=$(shell if [ "$(uname)" = 'Linux' ];then echo 'linux'; else echo 'mac'; fi)
VPATH = tools tools/$(sysname)

myserver.out : $(objects)
	cc -o myserver.out $(objects)

main.o :
request.o :
response.o :
utils.o :
poll.o :

.PHONY : clean
clean :
	-rm myserver.out $(objects)
