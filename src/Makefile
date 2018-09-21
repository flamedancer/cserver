objects = main.o request.o response.o \
    utils.o
VPATH = tools

edit : $(objects)
	gcc -o myserver.out $(objects)

main.o :
request.o :
response.o :
utils.o :

.PHONY : clean
clean :
	rm -f $(objects)
