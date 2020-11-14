/*
 * uftp_client.c - A slightly less simple UDP client?
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
//The header file socket.h includes a number of definitions of structures needed for sockets.*/
//The header file in.h contains constants and structures needed for internet domain addresses.
//This header file contains definitions for network database operations.
#define BUFSIZE 1024
#define MAXFILESIZE 24000

/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

void displayOptions()
{
	printf(" User, you are being prompted to type any of the following commands. \n");
	printf(" get [file_name]\n put [file_name]\n delete [file_name]\n ls\n exit\n");
}

char* getUserInput(char* message)
{
  bzero(message, BUFSIZE);
  fgets(message, BUFSIZE, stdin);
  return message;
}

int main(int argc, char **argv) {
  int sockfd, portno, n;
  int serverlen;
  struct sockaddr_in serveraddr;
  struct hostent *server;
  char *hostname;
  char buf[BUFSIZE];
  void serverReply()
  {
    /* print the server's reply */
    char tempBuf[200];
    int nBytes = recvfrom(sockfd, tempBuf, 50, 0, (struct sockaddr *)&serveraddr, &serverlen);
    if (nBytes < 0)
      error("ERROR in recvfrom");
    printf("Echo from server: %s", tempBuf);
  }
  void sendToServer(char* buf)
  {
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
    if (n < 0)
      error("ERROR in sendto");
  }
  /* check command line arguments */
  if (argc != 3) {
     fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
     exit(0);
  }
  hostname = argv[1];
  portno = atoi(argv[2]);

	displayOptions();

  /* socket: create the socket */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
      error("ERROR opening socket");

  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(hostname);
  if (server == NULL) {
      fprintf(stderr,"ERROR, no such host as %s\n", hostname);
      exit(0);
  }

  /* build the server's Internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(portno);


  /* start the command loop between client-server. */
  while (1){
    /* get the client terminal input and send to server */
    getUserInput(buf);

    /* seperating the command from the file name */
    char command[7], file_name[40];
    sscanf(buf, "%s %s", command, file_name);

    /* address each of the commands */
    if(strcmp(command, "delete") == 0)
    {
      sendToServer(buf);
      bzero(buf, BUFSIZE);
      serverReply();
    }

// https://www.geeksforgeeks.org/c-program-for-file-transfer-using-udp/
    else if(strcmp(command, "get") == 0)
    {
      sendToServer(buf); // send the request to the server to get the file
      char fileBuffer[MAXFILESIZE]; //create a buffer to store file contents
      n = recvfrom(sockfd, fileBuffer, MAXFILESIZE, 0, (struct sockaddr*)
                                                &serveraddr, &serverlen);
      if (n < 0)
        error("ERROR in recvfrom\n");
      /* Create the file received from the server */
      else
      {
        FILE *fileFromServer;
        fileFromServer = fopen(file_name, "w+");
        if(fwrite(fileBuffer, n, 1, fileFromServer) < 0)
          printf("Error copying contents of file from server to client file\n");
        else
          printf("File: %s should be successfully uploaded\n", file_name);

        fclose(fileFromServer);
      }
      serverReply();
    }

// this code comes from the server side of the "get" command.
    else if(strcmp(command, "put") == 0)
    {
      sendToServer(buf); // send the file transfer information
      FILE* fileToTransfer;

      if((fileToTransfer = fopen(file_name, "r")) == NULL)
        printf("File name: '%s' was not found.\n", file_name);

      else
      {
        //https://www.geeksforgeeks.org/c-program-for-file-transfer-using-udp/
        char transfer_buf[MAXFILESIZE];
        bzero(transfer_buf, MAXFILESIZE);
        fseek(fileToTransfer, 0, SEEK_END);
        size_t fileSize = ftell(fileToTransfer);
        printf("fileSize is equal to %ld\n", fileSize);

        fseek(fileToTransfer, 0, SEEK_SET);

        if(fread(transfer_buf, 1, fileSize, fileToTransfer) <=0)
          printf("Unsuccessful copying of file contents into file buffer\n");

        else if(sendto(sockfd, transfer_buf, fileSize, 0,
          (struct sockaddr *)&serveraddr, serverlen) >=0)
          printf("Successfully sent file: %s to client\n", file_name);
        else
          printf("Error in the transfer of the file to client");

      }
      fclose(fileToTransfer);
    }

    else if(strcmp(command, "ls") == 0)
    {

      sendToServer(buf);
      bzero(buf, BUFSIZE);
      n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)
                                                &serveraddr, &serverlen);
      if (n < 0)
        error("ERROR in recvfrom\n");
      else
        printf("Server directory includes the following documents:\n");
        printf("%s", buf);
        printf("--------------End of list-----------------------\n");
    }
    else if(strcmp(command, "exit") == 0)
    {
      sendToServer(buf);
      /* clean up the buffer space */
      bzero(buf, BUFSIZE);
      bzero(command, 7);
      bzero(file_name, 40);
      // close(sockfd);
      exit(0);
    }
    else
      printf("Invalid entry, enter: 'get', 'put', 'delete', 'ls', or 'exit'\n");


    /* clean up the buffer space */
    bzero(buf, BUFSIZE);
    bzero(command, 7);
    bzero(file_name, 40);

  }

  return 0;
}
