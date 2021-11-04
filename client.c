#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define RCVSIZE 1024

int main (int argc, char *argv[]) {

  struct sockaddr_in servaddr;
  int port;
  char addrIP[15];
  char buffer[RCVSIZE];
  char *hello = "Hello server, it's the client !";


  //get port
  if (argc < 3){
    printf("Too few arguments\n");
    printf("Format : ./client <ip_server> <port_server>");
    exit(1);
  } else if (argc > 3){
    printf("Too many arguments\n");
    printf("Format : ./client <ip_server> <port_server>");
    exit(1);
  } else {
      strcpy(addrIP, argv[1]);
      port = atoi(argv[2]);
  }

  //create socket
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  // handle error
  if (sock < 0) {
    perror("cannot create socket\n");
    return -1;
  }

  /*
   * fill a memory area, identified by its address 
   * and size, with a precise value
   */
  memset(&servaddr, 0, sizeof(servaddr));

  //filling server information
  servaddr.sin_family= AF_INET;
  servaddr.sin_port= htons(port);
  servaddr.sin_addr.s_addr= htonl(INADDR_ANY);

  //send a message to the server
  socklen_t len = sizeof(servaddr);
  //MSG_CONFIRM to tell the link layer that you got a successful reply from the other side
  sendto(sock, (char *)hello, strlen(hello), MSG_CONFIRM, (struct sockaddr *) &servaddr, len);
  printf("Hello message sent.\n");
  int n = recvfrom(sock, (char *)buffer, RCVSIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
  buffer[n]='\0';
  printf("Server : %s\n");

  
//free the socket
close(sock);
return 0;
}

