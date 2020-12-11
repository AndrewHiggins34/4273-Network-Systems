/* Trying to redo my Distributed File System in record time... */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <sys/socket.h>  /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>


#define BUFSIZE 8192
#define LISTENQ 1024

// some basic helper functions...
void displayOptions()
{
	printf(" User, you are being prompted to type any of the following commands. \n");
	printf(" GET [file_name]\n PUT [file_name]\n LIST\n exit\n");
}

char* getUserInput(char* message)
{
  bzero(message, BUFSIZE);
  fgets(message, BUFSIZE, stdin);
  return message;
}

int connect2Server(int port)
{
//https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
  int sockfd, connfd;
  struct sockaddr_in servaddr, cli;
  // create and verify socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("socket creation failed in connect2Server...\n");
    exit(0);
  }
  else{
    printf("Socket created successfully for port number %d", port);
    bzero(&servaddr, sizeof(servaddr));
  }
  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1:%d", port);
  servaddr.sin_port = htons(PORT);

  // connect the client socket to server socket
  if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
      printf("connection with the server failed...\n");
      exit(0);
  }
  else{
    printf("connected to the server..\n");
    return sockfd;
  }
}

int main(int argc, char **argv)
{


}
