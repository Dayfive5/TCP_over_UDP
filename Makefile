all : server client test

server: server.o
	gcc -o server server.o -Wall

client: client.o
	gcc -o client client.o -Wall

test: test.o
	gcc -o test test.o -Wall

serveur.o: server.c
	gcc -c server.c -Wall

client.o: client.c
	gcc -c client.c -Wall

test.o: test.c
	gcc -c test.c -Wall

clean: 
	rm -f *.o