#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>


#define RCVSIZE 1024


int receiveTextMsg(int sock, char buffer[RCVSIZE], struct sockaddr_in clientaddr, socklen_t len, char *hello);
int sendWaitACK(int msg_serv, struct sockaddr_in clientaddr, struct sockaddr_in servaddr, char *seg, int segSize, int timeoutUsec);
int sendFile(struct sockaddr_in clientaddr, struct sockaddr_in servaddr, int msg_serv);


int main (int argc, char *argv[]) {

  /*---------------------INITIALIZATION------------------ */

  struct sockaddr_in servaddr, clientaddr;
  int port;
  char* new_port_string="6667";
  int new_port = atoi(new_port_string);
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
  
  socklen_t len = sizeof(clientaddr);
  nbytes = recvfrom(sock, (char *)buffer, RCVSIZE, MSG_WAITALL, (struct sockaddr *) &clientaddr, &len);
  
  printf("____________________________________\n");
  printf("Waiting for three-way handshake with the client...\n");
  if ((strcmp(buffer, "SYN"))==0){
        printf("SYN received, sending SYN_ACK and new port...\n");
    }
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
  memcpy(buffer, "SYN_ACK", 7);
  strncat(buffer, new_port_string, 4);
  
  while(1){
    if ((nbytes = sendto(sock, buffer, 11, 0, (struct sockaddr *) &clientaddr, len)) == -1){
      perror("server: sendto failed");
      exit(2);
    }
    memset(buffer, '\0', sizeof(buffer));

    nbytes = recvfrom(sock,buffer,RCVSIZE,0,NULL, NULL);
    
    if ((strcmp(buffer, "ACK"))==0){
      printf("ACK received\n");
      break;
    }
  }
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
  
  //receiving a text message
  if((receiveTextMsg(msg_serv, buffer, clientaddr, len, hello)) == -1) {
    perror("Server : received message failed");
    exit(1);
  }
  printf("____________________________________\n");

  /*------------CLIENT ANSWERED : SENDING DATA----------------*/

  //fragment and send a file
  sendFile(clientaddr, servaddr, msg_serv);


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


int sendFile(struct sockaddr_in clientaddr, struct sockaddr_in servaddr, int msg_serv){
  int len = sizeof(clientaddr);

  FILE* fileToOpen;
  FILE* fileToReceive;

  fileToOpen = fopen("TP3.pdf", "r");
  fileToReceive = fopen("TP3bis.pdf", "a");

  //creating a buffer to put the file content 
  char *buf = NULL;

  //make sure the file isn't NULL
  if(fileToOpen != NULL){

    //reach the EOF (0L = long int with all his bits = 0)
    if(fseek(fileToOpen, 0L, SEEK_END) == 0){

      //get the file's size (ftell() reads the value of the stream indicator pointed by fileToOpen)
      long bufferSize = ftell(fileToOpen);
      if(bufferSize == -1){
        printf("Error : can't get the size of the file\n");
        exit(1);
      }

      //allocate the needed memory to the buffer
      buf = malloc(sizeof(char)* (bufferSize+1));

      //now go back to the start of the file
      if(fseek(fileToOpen, 0L, SEEK_SET) != 0) {
        printf("Error : can't go back to the start of the file\n");
        exit(1);
      }

      //read the file in our buffer
      size_t new_len = fread(buf, sizeof(char), bufferSize, fileToOpen);
      if(ferror(fileToOpen) != 0) {
        printf("Error : can't read file in memory\n");
      } else {
        //we make sure to have an EOF at the end of the buffer
        buf[new_len++] = '\0';
      }

      //divide buf in several segments 
      
      //sizes of our buffers
      int segSize = 1024; //total size of the segment
      int headerSize = 8; //segment ID 
      int chunkSize = 1016; //chunk of data to send

      //create the buffers
      int countdown = new_len;
      char * seg = NULL;
      seg = malloc(sizeof(char) *(segSize+1));
      char chunk[chunkSize];
      char header[headerSize];
      char * receiver_chunk = NULL;
      receiver_chunk = malloc(sizeof(char) *(chunkSize+1));

      //to count the packets
      int count = 0;

      //start fragmentation
      int bytestoCopy;

      while(countdown){
        //choose the amount of bytes to send
        if(countdown > chunkSize){
          bytestoCopy = chunkSize;
        } else {
          bytestoCopy = countdown;
        }
        memcpy(chunk, buf, bytestoCopy);
        buf = buf + bytestoCopy;
        countdown = countdown - bytestoCopy;

        //set an ID for the segment to put in the header
        count ++;
        sprintf(header, "%d", count); //sprintf writes the ID (count) in the buffer header
        
        memcpy(seg, "00000000", headerSize);
        
        if(count<10){
          memcpy(seg+7, header, headerSize);
          //seg+7 -> "0" in the header because count between 1 and 9
        }
        else if(count<100){
          memcpy(seg+6, header, headerSize);
          //seg+6 -> "00" in the header because count between 10 and 99
        }
        else if(count<1000){
          memcpy(seg+5, header, headerSize);          
          //seg+7 -> "000" in the header because count between 100 and 999
        }

        //put the chunk in the segment to send
        memcpy(seg+8, chunk, sizeof(chunk));

        //send the file to the client and wait for an ACK
        sendWaitACK(msg_serv, clientaddr, servaddr, seg, segSize,10);
        printf("Sending file...\n");

        //reset the buffers
        memset(seg, '\0', sizeof(seg));
        memset(chunk, '\0', sizeof(chunk));
        memset(header, '\0', sizeof(header));

        //receiver
        receiver_chunk = seg+8;
        fwrite(receiver_chunk, sizeof(char), chunkSize, fileToReceive);
        memset(receiver_chunk, '\0', sizeof(receiver_chunk));
      }
      printf("Done !\n");
      //Envoi d'un EOF au client 
      int eof;
      char buffEOF[3];
      memcpy(buffEOF, "EOF", 3);
      if ((eof = sendto(msg_serv, buffEOF, sizeof(buffEOF), 0, (struct sockaddr *) &clientaddr, len)) == -1){
        perror("server: EOF sendto failed");
        exit(2);
      }

    }
    fclose(fileToReceive);
    fclose(fileToOpen);
  }
  return 0;
}

int sendWaitACK(int msg_serv, struct sockaddr_in clientaddr, struct sockaddr_in servaddr, char *seg, int segSize, int timeoutUsec){
  
  int receiveSize = 12;
  char receiveACK[receiveSize];
  int n;
  int len = sizeof(clientaddr);

  //file descriptor for select()
  fd_set sock_set;

  //time structures
  struct timeval stop, start, timeout;

  //send segment to the client
  sendto(msg_serv, (char *)seg, segSize, MSG_CONFIRM, (struct sockaddr *) &clientaddr, len);
  printf("Segment sent\n");

  //fd_set set up
  FD_ZERO(&sock_set);
  FD_SET(msg_serv, &sock_set);

  //init timeout (timeout = timeoutUsec (µsec))
  timeout.tv_sec = 0;
  timeout.tv_usec = timeoutUsec;

  //start timer
  gettimeofday(&start, NULL);

  //wait for segment
  if (select(1, &sock_set, NULL, NULL, &timeout) == 0){
    printf("Timeout\n");
  } else {
    gettimeofday(&stop, NULL);
    n = recvfrom(msg_serv, (char *)receiveACK, receiveSize, MSG_WAITALL, (struct sockaddr *) &clientaddr, &len);
    printf("This operation took %lu µs.\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
    receiveACK[n]='\0';
    printf("ACK from client : %s\n", receiveACK);
  }
  //reset the buffer
  memset(receiveACK, '\0', sizeof(receiveSize));
  return 0;
}