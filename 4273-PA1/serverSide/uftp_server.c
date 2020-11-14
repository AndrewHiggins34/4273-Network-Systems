/*
 * udpserver.c - A simple UDP echo server
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h> // included for 'ls' call and 'readdir'/dirent struct
//sys/socket.h : The header file socket.h includes a number of definitions of structures needed for sockets.
//o netinet/in.h : The header file in.h contains constants and structures needed for internet domain addresses.
// netdb.h : This header file contains definitions for network database operations.
// arpa/inet.h : The header file contains definitions for internet operations.
#define BUFSIZE 1024
#define MAXFILESIZE 5120

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}
// referenced from https://www.zentut.com/c-tutorial/c-file-exists/
// return 1 if file is found, otherwise, 0
int fileFound(char* filename)
{
  printf("entered the fileFound function \n");
  FILE *fp;
  if((fp = fopen(filename, "r")) != NULL)
  {
    fclose(fp);
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /*
   * check command line arguments
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /*
   * socket: create the parent socket
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /*
   * bind: associate the parent socket with a port
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr,
	   sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  /*
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /*
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
			                 sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");

    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
    printf("server received %ld/%d bytes: %s\n", strlen(buf), n, buf);

    /* parse the buffer sent by client into command and file_name */
    char command[7], file_name[40];
    sscanf(buf, "%s %s", command, file_name);


    if(strcmp(command, "delete") == 0)
    {
      // code from tutorialspoint.com/c-program-to-dleete-a-file
      if(fileFound(file_name) == 0)
        printf("File: %s was not found, could not delete\n", file_name);
      else
      {
        printf("file was found to exist\n");
        int del = remove(file_name);
        if(!del)
        {
          printf("The file %s has been deleted successfully\n", file_name);
          char dl[100] = "The file has been deleted successfully\n";
          sendto(sockfd, dl, 100, 0, (struct sockaddr*)&clientaddr,clientlen);
        }

        else if(del == -1)
          printf("The file %s has not been deleted successfully\n", file_name);
        else
          printf("Something unusual happened in the file deletion");
      }
    }

    else if(strcmp(command, "get") == 0)
    {
      FILE *fileRequested;

      if((fileRequested = fopen(file_name, "r")) == NULL) // rb?
      { /* inform client screen that the file wasn't found. */
          printf("File requested not found on server.\n");
          // char message[40] = "File requested not found on server.";
          // n = sendto(sockfd, message, strlen(message), 0,
          //      (struct sockaddr *) &clientaddr, clientlen);
      }
      else
      {
    // referencing http://www.fundza.com/c4serious/fileIO_reading_all/index.html
        char net_buf[MAXFILESIZE]; // creating a 5kb Buffer
        bzero(net_buf, MAXFILESIZE);
        // get the number of bytes
        fseek(fileRequested, 0, SEEK_END);
        size_t fileSize = ftell(fileRequested);

        printf("fileSize is equal to %ld\n", fileSize);
        // reset to the beginning of files
        fseek(fileRequested, 0, SEEK_SET);
        /* copy requested file into buffer to be sent
    copy filesize bytes, 1 byte at a time, from fileRequested into net_buf */
        if(fread(net_buf, 1, fileSize, fileRequested) <=0)
          printf("Unsuccessful copying of file contents into file buffer\n");

        /* send the requested file to the client
        send filesize bytes, from netbuf to client */
        else if(sendto(sockfd, net_buf, fileSize, 0, (struct sockaddr*)&clientaddr,
                                                           clientlen) >= 0)
            printf("Successfully sent file: %s to client\n", file_name);
        else
            printf("Error in the transfer of the file to client");
      }
      fclose(fileRequested);
    }
// this code comes from the client side of the "get" case
    else if(strcmp(command, "put") == 0)
    {
      char net_buf[MAXFILESIZE]; // creating a 5kb Buffer
      bzero(net_buf, MAXFILESIZE);
      n = recvfrom(sockfd, net_buf, MAXFILESIZE, 0,
              (struct sockaddr*)&clientaddr, &clientlen);
      if (n < 0)
        error("ERROR in recvfrom\n");
      else
      {
        FILE *fileFromClient;
        fileFromClient = fopen(file_name, "w+");
        if(fwrite(net_buf, n, 1, fileFromClient) < 0)
          printf("Error copying contents of file from server to client file\n");
        else
          printf("File: %s should be successfully uploaded\n", file_name);

        fclose(fileFromClient);
      }
    }

    else if(strcmp(command, "ls") == 0)
    {
//https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
      char tempBuf[BUFSIZE];
      struct dirent *currDirectory;
      DIR *dirp = opendir(".");
      if(dirp == NULL)
        printf("Somehow can't access the current directory");

      else
      {
        int i = 0;
        printf("---------The following directories are being passed--------\n");
        bzero(buf, BUFSIZE);
        while((currDirectory = readdir(dirp)) != NULL)
        {
          strcat(buf, currDirectory->d_name);
          strcat(buf, "\n");
        }
          printf("%s\n", buf);
      }

      closedir(dirp);
      free(currDirectory);
    }

    else if(strcmp(command, "exit") == 0)
    {
      /* clean up the buffer space */
      bzero(buf, BUFSIZE);
      printf("Server self-destruction\n");
      // close(sockfd);
      exit(0);
    }

    /*
     * sendto: echo the input back to the client
     */
    n = sendto(sockfd, buf, strlen(buf), 0,
	       (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0)
      error("ERROR in sendto");
    /* clear all of the buffers */
    bzero(buf, BUFSIZE);
    bzero(command, 7);
    bzero(file_name, 40);
  }
}
