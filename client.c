#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define RCVSIZE 1024
#define DATA 0
#define SYN 1
#define SYN_ACK 2
#define ACK 3
#define END 4
#define END_ACK 5

typedef struct {   
  int code;   //code is either DATA, SYN, SYN_ACK, ACK, END or END_ACK
}TCP_listener;

int main (int argc, char *argv[]) {

  struct sockaddr_in servaddr;
  int port;
  char addrIP[15];
  char buffer[RCVSIZE];
  char *hello = "Hello server, it's the client !";
  int nbytes;


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

  //three-way handshake
  TCP_listener clihandshake, servhandshake;
  int TCP_len = (int) sizeof(clihandshake);
  clihandshake.code = SYN;
  printf("____________________________________\n");
  printf("Starting three-way handshake with server...\n");
  socklen_t len = sizeof(servaddr);

  while(1){
    memcpy(buffer,&clihandshake,TCP_len);
    if ((nbytes = sendto(sock, buffer, TCP_len,0,(struct sockaddr*) &servaddr, len)) == -1){
      perror("client : sendto failed");
      exit(1);
    }
    printf("SYN sent (code 1), waiting for SYN_ACK...\n");
    nbytes = recvfrom(sock,buffer,RCVSIZE,0, (struct sockaddr *) &servaddr, &len);
    if (nbytes == TCP_len){
      memcpy(&servhandshake, buffer, TCP_len);
      break;
    }
  }
  printf("SYN_ACK received, code : %d, sending ACK...\n", servhandshake.code);

  clihandshake.code = ACK;
  memcpy(buffer,&clihandshake,TCP_len);
  if ((nbytes = sendto(sock, buffer, TCP_len,0,(struct sockaddr*) &servaddr, len)) == -1){
      perror("client : sendto failed");
      exit(1);
  }

  printf("Connexion established !\n");
  printf("____________________________________\n");



  //send a message to the server
  //MSG_CONFIRM to tell the link layer that you got a successful reply from the other side
  sendto(sock, (char *)hello, strlen(hello), MSG_CONFIRM, (struct sockaddr *) &servaddr, len);
  printf("Hello message sent.\n");
  int n = recvfrom(sock, (char *)buffer, RCVSIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
  buffer[n]='\0';
  printf("Server : %s\n", buffer);

  
//free the socket
close(sock);
return 0;
}

