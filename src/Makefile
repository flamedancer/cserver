objects = main.o task.o request.o response.o \
    utils.o poll.o sem.o
sysname=$(shell if [ "$$(uname)" = 'Linux' ];then echo 'linux'; else echo 'mac'; fi)
VPATH = tools tools/$(sysname)

myserver.out : $(objects)
	cc -o myserver.out $(objects)

main.o :
task.o :
request.o :
response.o :
utils.o :
poll.o :
sem.o :

.PHONY : clean
clean :
	-rm myserver.out $(objects)
