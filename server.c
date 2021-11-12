#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define RCVSIZE 1024
#define DATA 0
#define SYN 1
#define SYN_ACK 2
#define ACK 3
#define END 4
#define END_ACK 5

int receiveTextMsg(int sock, char buffer[RCVSIZE], struct sockaddr_in clientaddr, socklen_t len, char *hello);

typedef struct {   
  int code;   //code is either DATA, SYN, SYN_ACK, ACK, END or END_ACK
}TCP_listener;

int main (int argc, char *argv[]) {

  /*---------------------INITIALIZATION------------------ */

  struct sockaddr_in servaddr, clientaddr;
  int port;
  int new_port = 6667;
  char buffer[RCVSIZE];
  char *hello = "Hello client, it's the server !";
  int nbytes;


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

  /*---------------------THREE-WAY HANDSHAKE------------------ */
  TCP_listener handshake;
  int TCP_len = (int) sizeof(handshake);
  socklen_t len = sizeof(clientaddr);
  nbytes = recvfrom(sock, (char *)buffer, RCVSIZE, MSG_WAITALL, (struct sockaddr *) &clientaddr, &len);
  memcpy(&handshake, buffer, TCP_len); //we copy the memory bloc of buffer in handshake
  printf("____________________________________\n");
  printf("Waiting for three-way handshake with the client...\n");
  printf("SYN received, code : %d, sending SYN_ACK...\n",handshake.code);

  //set a socket for messages
  int msg_serv = socket(AF_INET, SOCK_DGRAM, 0);
  if(msg_serv < 0){
    perror("Can't create socket");
    return -1;
  }
  servaddr.sin_port= htons(new_port);
  if (bind(msg_serv, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
    perror("Bind failed\n");
    close(msg_serv);
    return -1;
  }

  //server ready, sending SYNACK + new port
  handshake.code = SYN_ACK + new_port;
  while(1){
    memcpy(buffer, &handshake, TCP_len);
    if ((nbytes = sendto(sock, buffer, TCP_len, 0, (struct sockaddr *) &clientaddr, len)) == -1){
      perror("server: sendto failed");
      exit(2);
    }
    nbytes = recvfrom(sock,buffer,RCVSIZE,0,NULL, NULL);
    if (nbytes >= TCP_len){
      memcpy(&handshake, buffer,TCP_len);
      if(handshake.code == ACK || handshake.code == DATA){
        break;
      }
    }
  }
  printf("ACK received, code : %d\n", handshake.code);
  printf("Connexion established !\n");
  printf("____________________________________\n");

  /*------------------CONNEXION ESTABLISHED--------------------*/
  /*---------------------SET NEW PORT--------------------------*/
  
  char test[RCVSIZE];
  int n;
  int len_test = sizeof(clientaddr);
  n = recvfrom(msg_serv, (char *)test, RCVSIZE, MSG_WAITALL, ( struct sockaddr *) &clientaddr, &len_test);
  test[n]='\0';
  printf("test : %s : new port established !\n",test);
  printf("____________________________________\n");

  
  /*----------NEW PORT ESTABLISHED, WAIT FOR CLIENT------------*/
  
  //recieving a text message
  if((receiveTextMsg(msg_serv, buffer, clientaddr, len, hello)) == -1) {
    perror("Server : received message failed");
    exit(1);
  }
  printf("____________________________________\n");

  /*------------CLIENT ANSWERED : SENDING DATA----------------*/

  //receiving a file
  while(1){
    break;
  }


/*------------------- END : FREE THE SOCKET -------------------*/ 
close(msg_serv);
close(sock);
return 0;
}



/*--------------------------------------------------------------

                            FUNCTIONS

--------------------------------------------------------------*/



int receiveTextMsg(int sock, char buffer[RCVSIZE], struct sockaddr_in clientaddr, socklen_t len, char *hello){
  //MSG_WAITALL to block until we receive a message
  int n = recvfrom(sock, (char *)buffer, RCVSIZE, MSG_WAITALL, (struct sockaddr *) &clientaddr, &len);
  buffer[n] = '\0';
  printf("Client : %s\n", buffer);
  sendto(sock, (char*)hello, strlen(hello), MSG_CONFIRM, (struct sockaddr *) &clientaddr, len);
  printf("Hello message sent.\n");
  return 0;
}
