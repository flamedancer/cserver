objects = main.o request.o response.o \
    utils.o
VPATH = tools

edit : $(objects)
	cc -o myserver.out $(objects)

main.o :
request.o :
response.o :
utils.o :

.PHONY : clean
clean :
	-rm myserver $(objects)
