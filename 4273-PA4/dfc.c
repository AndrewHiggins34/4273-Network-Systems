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
#include <openssl/md5.h>


#define BUFSIZE 8192
#define FILENAME 50
#define SA struct sockaddr

// some basic helper functions...

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
  /* create connections with the 4 servers & forward username */
  int sockfd[4], n;
  int portno[4] = {10001, 10002, 10003, 10004};
  for(int i=0;i<4;i++)
    sockfd[i] = connect2Server(portno[i]);

	printf("You may execute the following commands:\nGET [file_name]\n PUT [file_name]\n LIST\n exit\n");

  /* create & zero the buffers */
  char buf[BUFSIZE], servBuf[BUFSIZE], command[5], file[FILENAME], initialReq[20], passN[20];
  bzero(buf, BUFSIZE); bzero(servBuf, BUFSIZE), bzero(command, 5); bzero(file, FILENAME);
	bzero(initialReq, 20); bzero(passN, 20);
  int serverlen;
  struct sockaddr_in serveraddr;

void forwardCredentials()
{
  printf("Enter username(enter) and password(enter)\n");
  bzero(buf, BUFSIZE);
  scanf("%s ", buf);
  scanf(" %s", passN);
  strcat(buf, " ");
  strcat(buf, passN);
  for(int i = 0;i<4;i++){
    if((n = send(sockfd[i], initialReq, strlen(initialReq), 0))<0)
      printf("ERROR in send");
  }
  for(int i = 0;i<4;i++){
    send(sockfd[i], buf, strlen(buf), 0);
  }
    /* zero the buf and read the servers response in */
  	bzero(buf, BUFSIZE); bzero(initialReq, 20);
}

  while(1){
    printf("Enter your command and file is necessary\n");
    getUserInput(initialReq);
    sscanf(initialReq, "%s %s", command, file);
    printf("Stored: %s and file: %s\n", command, file);
    if(strcmp(command, "LIST") == 0)
    {
      forwardCredentials();
      /* Print what the server sends */
      for(int i = 0;i<4;i++){
				read(sockfd[i], servBuf, BUFSIZE);
        printf("Server directory includes the following documents for server %d:\n", i);
        printf("%s", servBuf);
        printf("--------------End of list-----------------------\n");
        bzero(servBuf, BUFSIZE);
      }
      bzero(initialReq, 20);
    }
    else if(strcmp(command, "GET") ==0)
    {

    }
    else if(strcmp(command, "PUT") == 0)
    {
      forwardCredentials();
      FILE* putFile = fopen(file, "rb+");
      if(putFile==NULL){
        printf("Cannot find/open filename entered. Try again.");
        bzero(initialReq, 20);
        break;
      }
      else{
        /* get the size of the file and store it */
        /* taken & modified from my uftp code. */
        bzero(buf, BUFSIZE); // not taking any chances...
        fseek(putFile, 0, SEEK_END);
        int n = ftell(putFile);
        fseek(putFile, 0, SEEK_SET);
        if(fread(buf, 1, n, putFile) <=0)
          printf("Unsuccessful copying of file contents into file buffer\n");
        ////////////////////////////////////////////////////////////////////////
        /* Get the MD5 of the file */
        /* Taken from linux man page for md5_init*/
        /* The following functions may be used if the message is not completely stored in memory: */
        // unsigned char *MD5(const unsigned char *d, unsigned long n, unsigned char *md);
        unsigned char *d; unsigned long hashN; unsigned char *md; MD5_CTX md5;
        unsigned char hashy[MD5_DIGEST_LENGTH];
        MD5(buf, n, hashy);
        // printf("hashy = %x\n", hashy);

        /* cast hash to int IOT mod by 4 */
        unsigned int hashInt;
        for(int i =0; i<MD5_DIGEST_LENGTH;i++)
          hashInt += (unsigned int)hashy[i];

          hashInt = hashInt%4;
          int mod = n%4;
          printf("hashint = %d\n", hashInt);
          printf("mod = %d\n", mod);

        /* divide & store the file into 4 parts */
        int part[4], quarter = n/4;
        unsigned int qp = quarter+hashInt+1;
        char DFCBuf[3][quarter-1];
        char PartDBuff[quarter+hashInt+1]; // get the remainder + 1 included


        fseek(putFile, 0, SEEK_SET);
        for(int i=0;i<4;i++){
          if(i==0)
          {
            fread(DFCBuf[i], 1, quarter ,putFile);
            strcat(DFCBuf[i], "a");
            printf("DFCBuf at %d is: %s\n", (i+1), DFCBuf[i]);
          }

          else if(i==1)
          {
            fread(DFCBuf[i], 1, (quarter-5) ,putFile);
            strcat(DFCBuf[i], "b");
            printf("DFCBuf at %d is: %s\n", (i+1), DFCBuf[i]);

          }

          else if(i==2)
          {
            fread(DFCBuf[i], 1, quarter ,putFile);
            strcat(DFCBuf[i], "c");
            printf("DFCBuf at %d is: %s\n", (i+1), DFCBuf[i]);

          }
          else
          {
            fread(PartDBuff, 1, qp ,putFile);
            strcat(PartDBuff, "d");
            printf("DFCBuf at %d is: %s\n", (i+1), PartDBuff);

          }
        }
        fclose(putFile);


        switch(hashInt)
        {
          // x value:0, DFS1 (1,2), DFS2(2,3), DFS3(3,4) DFS4(4,1)
          case 0:
              write(sockfd[0], DFCBuf[0], strlen(DFCBuf[0])); // 1->DFS1
              write(sockfd[0], DFCBuf[1], strlen(DFCBuf[0])); // 2->DFS1
              write(sockfd[1], DFCBuf[1], strlen(DFCBuf[0])); // 2->DFS2
              write(sockfd[1], DFCBuf[2], strlen(DFCBuf[0])); // 3->DFS2
              write(sockfd[2], DFCBuf[2], strlen(DFCBuf[0])); // 3->DFS3
              write(sockfd[2], PartDBuff, strlen(DFCBuf[0])); // 4->DFS3
              write(sockfd[3], PartDBuff, strlen(DFCBuf[0])); // 4->DFS4
              write(sockfd[3], DFCBuf[0], strlen(DFCBuf[0])); // 1->DFS4
            break;

          // x value:1, DFS1 (4,1), DFS2(1,2), DFS3(2,3) DFS4(3,4)
          case 1:
            write(sockfd[0], PartDBuff, strlen(DFCBuf[0])); // 4->DFS1
            write(sockfd[0], DFCBuf[0], strlen(DFCBuf[0])); // 1->DFS1
            write(sockfd[1], DFCBuf[0], strlen(DFCBuf[0])); // 1->DFS2
            write(sockfd[1], DFCBuf[1], strlen(DFCBuf[0])); // 2->DFS2
            write(sockfd[2], DFCBuf[1], strlen(DFCBuf[0])); // 2->DFS3
            write(sockfd[2], DFCBuf[2], strlen(DFCBuf[0])); // 3->DFS3
            write(sockfd[3], DFCBuf[2], strlen(DFCBuf[0])); // 3->DFS4
            write(sockfd[3], PartDBuff, strlen(DFCBuf[0])); // 4->DFS4
          break;

          // x value:2, DFS1 (3,4), DFS2(4,1), DFS3(1,2) DFS4(2,3)
          case 2:
            write(sockfd[0], DFCBuf[2], strlen(DFCBuf[0])); // 3->DFS1
            write(sockfd[0], PartDBuff, strlen(DFCBuf[0])); // 4->DFS1
            write(sockfd[1], PartDBuff, strlen(DFCBuf[0])); // 4->DFS2
            write(sockfd[1], DFCBuf[0], strlen(DFCBuf[0])); // 1->DFS2
            write(sockfd[2], DFCBuf[0], strlen(DFCBuf[0])); // 1->DFS3
            write(sockfd[2], DFCBuf[1], strlen(DFCBuf[0])); // 2->DFS3
            write(sockfd[3], DFCBuf[1], strlen(DFCBuf[0])); // 2->DFS4
            write(sockfd[3], DFCBuf[2], strlen(DFCBuf[0])); // 3->DFS4
          break;

          // x value:2, DFS1 (2,3), DFS2(3,4), DFS3(4,1) DFS4(1,2)
          case 3:
            write(sockfd[0], DFCBuf[1], strlen(DFCBuf[0])); // 2->DFS1
            write(sockfd[0], DFCBuf[2], strlen(DFCBuf[0])); // 3->DFS1
            write(sockfd[1], DFCBuf[2], strlen(DFCBuf[0])); // 3->DFS2
            write(sockfd[1], PartDBuff, strlen(DFCBuf[0])); // 4->DFS2
            write(sockfd[2], PartDBuff, strlen(DFCBuf[0])); // 4->DFS3
            write(sockfd[2], DFCBuf[0], strlen(DFCBuf[0])); // 1->DFS3
            write(sockfd[3], DFCBuf[0], strlen(DFCBuf[0])); // 1->DFS4
            write(sockfd[3], DFCBuf[1], strlen(DFCBuf[0])); // 2->DFS4
          break;

          default:
            printf("System is VERY broken");
        }
        bzero(initialReq, 20);
      }
    }
    else if(strcmp(command, "exit") == 0)
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
