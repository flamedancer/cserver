objects = main.o request.o response.o \
    utils.o
VPATH = tools

myserver.out : $(objects)
	cc -o myserver.out $(objects)

main.o :
request.o :
response.o :
utils.o :

.PHONY : clean
clean :
	-rm myserver.out $(objects)
