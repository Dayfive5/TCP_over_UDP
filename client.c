#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define RCVSIZE 1024
#define SYN 1
#define SYN_ACK 2
#define ACK 3



typedef struct {   
  int code;   //code is either SYN, SYN_ACK, ACK
}TCP_listener;

int main (int argc, char *argv[]) {

  /*---------------------INITIALIZATION------------------ */

  struct sockaddr_in servaddr;
  int port;
  int new_port;
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

  /*---------------------THREE-WAY HANDSHAKE------------------ */

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
      //extracting the new port for messages
      new_port = servhandshake.code - SYN_ACK;
      servhandshake.code = servhandshake.code - new_port;
      memcpy(buffer, &servhandshake, TCP_len);
      //printf("new port : %d\n",new_port);
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

  /*------------------CONNEXION ESTABLISHED--------------------*/
  /*--------------------TESTING NEW PORT-----------------------*/
  //set a socket for messages
  int msg_serv = socket(AF_INET, SOCK_DGRAM, 0);
  if(msg_serv < 0){
    perror("Can't create socket");
    return -1;
  }
  servaddr.sin_port = htons(new_port);
  
  char *test="OK";  
  sendto(msg_serv, (char *)test, strlen(test), MSG_CONFIRM, (struct sockaddr *) &servaddr, len);
  printf("test sent\n");
  printf("____________________________________\n");

  /*---------------------SENDING MESSAGE-----------------------*/
  
  //MSG_CONFIRM to tell the link layer that you got a successful reply from the other side
  sendto(msg_serv, (char *)hello, strlen(hello), MSG_CONFIRM, (struct sockaddr *) &servaddr, len);
  printf("Hello message sent.\n");
  int n = recvfrom(msg_serv, (char *)buffer, RCVSIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
  buffer[n]='\0';
  printf("Server : %s\n", buffer);
  printf("____________________________________\n");

  /*------------SERVER ANSWERED : RECEIVING DATA--------------*/
  
  FILE *file;
  file = fopen("TP3ter.pdf", "a");

  int i = 0;
  int ackSize = 12;
  int segSize = 1024; 
  int chunkSize = 1016;
  char *receive_chunk = NULL;
  receive_chunk = malloc(sizeof(char) * (chunkSize + 1));
  char *seg = NULL;
  seg = malloc(sizeof(char) * (segSize + 1));
  char header[8];
  //int n_seq = 0;
  char *ack;
  ack = malloc(sizeof(char) * (12 + 1));

  while(1){
    //receive file
    i = recvfrom(msg_serv, (char *)seg, segSize, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    seg[i]='\0';


    if((strcmp(seg, "EOF")) == 0){
      //EOF
      printf("EOF\n");
      break;
    }
    //parse the data (8 for header, 1016 for data)
    memcpy(header, seg, 8);
    header[8]='\0';
    receive_chunk = seg+8;
    //n_seq = atoi(header);

    //append file
    fwrite(receive_chunk, sizeof(char), chunkSize, file);

    //ACK each segment
    memcpy(ack, "ACK_00000000", ackSize);
    memcpy(ack+4, header, 8);

    //wait
    usleep(100);

    //send ACKs
    sendto(msg_serv, (char *)ack, ackSize, MSG_CONFIRM, (struct sockaddr *) &servaddr, len);
    printf("%s sent.\n", ack);

    //reset buffers
    memset(ack, '\0', ackSize);
    memset(header, '\0', 8);
    memset(seg, '\0', segSize);
    memset(receive_chunk, '\0', chunkSize);
  }




/*------------------- END : FREE THE SOCKET ------------------*/ 
close(msg_serv);
close(sock);
return 0;
}

/*--------------------------------------------------------------

                            FUNCTIONS

--------------------------------------------------------------*/



