all : server client

server: server.o
	gcc -o server server.o -Wall

client: client.o
	gcc -o client client.o -Wall

serveur.o: server.c
	gcc -c server.c -Wall

client.o: client.c
	gcc -c client.c -Wall

clean: 
	rm -f *.o