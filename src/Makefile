objects = main.o request.o response.o \
    utils.o
VPATH = tools

myserver.out : $(objects)
	gcc -o myserver.out $(objects)

main.o :
request.o :
response.o :
utils.o :

.PHONY : clean
clean :
	-rm myserver.out $(objects)
