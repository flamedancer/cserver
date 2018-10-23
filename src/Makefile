objects = main.o request.o response.o \
    utils.o poll.o
VPATH = tools tools/mac

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
