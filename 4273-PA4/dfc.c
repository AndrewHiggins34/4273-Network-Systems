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
#define FILENAME 50
#define SA struct sockaddr

// some basic helper functions...
void displayOptions()
{
	printf("User, please enter your username. Then you may execute the following commands:\n");
	printf("GET [file_name]\n PUT [file_name]\n LIST\n exit\n");
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
    perror("socket creation failed in connect2Server...\n");
    exit(0);
  }
  else{
    printf("Socket created successfully for port number %d\n", port);
    bzero(&servaddr, sizeof(servaddr));
  }
  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);
  memset(servaddr.sin_zero, '\0', sizeof(servaddr.sin_zero));

  // connect the client socket to server socket
  if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0)
      printf("connection with the server failed...\n");

  else{
    printf("connected to the server...\n");
    return sockfd;
  }
  return -1;
}



int main(int argc, char **argv)
{
  /* verify the arguments */
  if(argc != 2){
    printf("Incorrect call, Usage: # dfc.conf");
    exit(0);
  }
  /* display options and read user input */
  displayOptions();
  char username[20];
  scanf("%s",&username);

  /* create connections with the 4 servers & forward username */
  int sockfd[4], n;
  int portno[4] = {10001, 10002, 10003, 10004};
  for(int i=0;i<4;i++){
    sockfd[i] = connect2Server(portno[i]);
    if((n = send(sockfd[i], username, strlen(username), 0))<0)
      printf("ERROR in send");
  }
  /* create & zero the buffers */
  char buf[BUFSIZE], command[5], file[FILENAME];
  bzero(buf, BUFSIZE); bzero(command, 5); bzero(file, FILENAME);
  int serverlen;
  struct sockaddr_in serveraddr;

  while(1){
    getUserInput(buf);
    sscanf(buf, "%s %s", command, file);
    printf("Stored: %s and file: %s", command, file);
    if(strcmp(command, "LIST") == 0)
    {
      for(int i = 0;i<4;i++){
        if((n = send(sockfd[i], command, strlen(command), 0))<0)
          printf("ERROR in send");
        /* zero the buf and read the servers response in */
        bzero(buf, BUFSIZE);
        n = read(sockfd[i], buf, BUFSIZE);
        /* Print what the server sends */
        printf("Server directory includes the following documents:\n");
        printf("%s", buf);
        printf("--------------End of list-----------------------\n");
        bzero(buf, BUFSIZE);
      }
    }else
    if(strcmp(command, "GET") ==0)
    {

    }else

    if(strcmp(command, "PUT") == 0)
    {

    }else
    if(strcmp(command, "exit") == 0)
    {
      for(int y=0;y<4;y++){
        close(sockfd[y]);
        exit(0);
      }
    }

    else
    {
      printf("Command not recognized, please use commands posted in display");
    }

  }
}
