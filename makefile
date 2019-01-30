all:
	gcc -pthread parent.c wrappers.c 	       -o parent
	gcc -pthread supervisor.c message.h wrappers.c -o supervisor
	gcc -pthread factory.c message.h wrappers.c    -o factory

clean:
	rm -f *.o parent factory supervisor

