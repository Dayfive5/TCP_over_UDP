#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define RCVSIZE 1024


int main (int argc, char *argv[]) {

  struct sockaddr_in servaddr, clientaddr;
  int port;
  char buffer[RCVSIZE];
  char *hello = "Hello client, it's the server !";


  //get port
  if (argc < 2){
    printf("Too few arguments\n");
    printf("Format : ./server <port_server>");
    exit(1);
  } else if (argc > 2){
    printf("Too many arguments\n");
    printf("Format : ./server <port_server>");
    exit(1);
  } else {
      port = atoi(argv[1]);
  }


  //create socket
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  //handle errors
  if (sock < 0) {
    perror("Cannot create socket\n");
    return -1;
  }

  /*
   * fill a memory area, identified by its address 
   * and size, with a precise value
   */
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&clientaddr, 0, sizeof(clientaddr));
  
  //filling server information
  servaddr.sin_family= AF_INET;
  servaddr.sin_port= htons(port);
  servaddr.sin_addr.s_addr= htonl(INADDR_ANY);

  /*
   * bind the socket with the server address to allow client
   * of the network to send messages (it enables the OS 
   * to read messages and send them to the right server)
   */
  if (bind(sock, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
    perror("Bind failed\n");
    close(sock);
    return -1;
  }


  //waiting until datagram packet arrives from client
  while (1) {
    socklen_t len = sizeof(clientaddr);
    //MSG_WAITALL to block until we receive a message
    int n = recvfrom(sock, (char *)buffer, RCVSIZE, MSG_WAITALL, (struct sockaddr *) &clientaddr, &len);
    buffer[n] = '\0';
    printf("Client : %s\n", buffer);
    sendto(sock, (char*)hello, strlen(hello), MSG_CONFIRM, (struct sockaddr *) &clientaddr, len);
    printf("Hello message sent.\n");
  }

  
//free the socket
close(sock);
return 0;
}
