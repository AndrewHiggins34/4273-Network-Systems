/*
 * tcpechosrv.c - A concurrent TCP echo server using threads
 */

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

#define MAXBUF  8192  /* max text line length */
#define LISTENQ  1024  /* second argument to listen() */
#define PARSELNGTH 50 /* parse sentence length */


int open_listenfd(int port);
void echo(int connfd);
void *thread(void *vargp);

int main(int argc, char **argv)
{
    int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;

    if (argc != 2) {
    	fprintf(stderr, "usage: %s <port>\n", argv[0]);
    	exit(0);
    }


    port = atoi(argv[1]);
    listenfd = open_listenfd(port);

    while (1) {
    	connfdp = malloc(sizeof(int));  // pointer to pass the socket
    	*connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
    	pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* thread routine */
void * thread(void * vargp)
{
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self()); // what is the pthread detach
    free(vargp);
    echo(connfd);
    close(connfd);  // we may need to look further into this close statement, and use a timer.
    return NULL;
}
/*
 * echo - read and echo text lines until client closes connection
 */
  void echo(int connfd)
  {
    size_t n;
    int fileSize;
    char buf[MAXBUF], fileBuf[MAXBUF];
    bzero(buf, (MAXBUF));
    bzero(fileBuf, MAXBUF);
    char fileType[8];
    bzero(fileType, 8);
    char httpmsg[]="HTTP/1.1 200 Document Follows\r\nContent-Type:text/html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>";
    char* indexPage = "www/index.html";

    n = read(connfd, buf, MAXBUF);
    printf("server received the following request:\n%s\n",buf);

    /* parse the request */
    char requestType[PARSELNGTH];
    char fullPath[PARSELNGTH];
    char pathAddOn[4+PARSELNGTH] = "www";
    char contentType[22];
    sscanf(buf, "%s %s", requestType, fullPath);
    // printf("fullPath= %s\n\n",  fullPath);
      /* extract the last 4 characters of the path to get the file type */
      int size = strlen(fullPath);
      FILE* fp;
      /* get the accurate relative path of the file */
      strncat(pathAddOn, fullPath, size);
      printf("\npath after pathaddon is %s\n\n", pathAddOn);
      /* open the file to be read */
      fp = fopen(pathAddOn,"rb");
      if(fp == NULL){
        printf("File was empy or didn't open");
        exit(0);
      }
      /* taken & modified from my uftp code. */
      fseek(fp, 0, SEEK_END);
      n = ftell(fp);
      fileSize = n;
      fseek(fp, 0, SEEK_SET);


      if(strstr(fullPath, ".html")){
        sprintf(buf, "HTTP/1.1 200 Document Follows\r\nContent-Type:text/html\r\nContent-Length: %d\r\n\r\n",fileSize);
        write(connfd, buf, strlen(buf));
        printf("Server sent the following header information\n%s\n",buf);
        bzero(buf, MAXBUF);
      }

      else if(strstr(fullPath, ".txt")){

        sprintf(buf, "HTTP/1.1 200 Document Follows\r\nContent-Type:text/plain\r\nContent-Length: %d\r\n\r\n",fileSize);
        write(connfd, buf, strlen(buf));
        printf("Server sent the following header information\n%s\n",buf);
        bzero(buf, MAXBUF);
      }

      else if(strstr(fullPath, ".png")){
        sprintf(buf, "HTTP/1.1 200 Document Follows\r\nContent-Type:image/png\r\nContent-Length: %d\r\n\r\n",fileSize);
        write(connfd, buf, strlen(buf));
        printf("Server sent the following header information\n%s\n",buf);
        bzero(buf, MAXBUF);
      }

      else if(strstr(fullPath, ".gif")){
        sprintf(buf, "HTTP/1.1 200 Document Follows\r\nContent-Type:image/gif\r\nContent-Length: %d\r\n\r\n",fileSize);
        write(connfd, buf, strlen(buf));
        printf("Server sent the following header information\n%s\n",buf);
        bzero(buf, MAXBUF);
      }

      else if(strstr(fullPath, ".jpg")){
        sprintf(buf, "HTTP/1.1 200 Document Follows\r\nContent-Type:image/jpg\r\nContent-Length: %d\r\n\r\n",fileSize);
        write(connfd, buf, strlen(buf));
        printf("Server sent the following header information\n%s\n",buf);
        bzero(buf, MAXBUF);

      }

      else if(strstr(fullPath, ".css")){
        sprintf(buf, "HTTP/1.1 200 Document Follows\r\nContent-Type:text/css\r\nContent-Length: %d\r\n\r\n",fileSize);
        write(connfd, buf, strlen(buf));
        printf("Server sent the following header information\n%s\n",buf);
        bzero(buf, MAXBUF);

      }

      else if(strstr(fullPath, ".js")){
        sprintf(buf, "HTTP/1.1 200 Document Follows\r\nContent-Type:application/javascript\r\nContent-Length: %d\r\n\r\n",fileSize);
        write(connfd, buf, strlen(buf));
        printf("Server sent the following header information\n%s\n",buf);
        bzero(buf, MAXBUF);
      }

      else{
        // /* display the index.html page. */
        fp = fopen(indexPage,"rb");
        if(fp == NULL){
          printf("File was empy or didn't open");
          exit(0);
        }

        /* taken & modified from my uftp code. */
        fseek(fp, 0, SEEK_END);
        n = ftell(fp);
        fileSize = n;
        fseek(fp, 0, SEEK_SET);
        if(fread(fileBuf, 1, n, fp) <=0)
          printf("Unsuccessful copying of file contents into file buffer\n");
            fclose(fp);
            // I hate sprintf...
            sprintf(buf, "HTTP/1.1 200 Document Follows\r\nContent-Type:text/html\r\nContent-Length: %d\r\n\r\n", fileSize);
            strcat(buf,fileBuf);
            printf("server returning a http message with the following content.\n%s\n",buf);
            write(connfd, buf,strlen(buf));
            bzero(buf, MAXBUF);
            return;
      }


      if(fileSize < 8192)
      {
        printf("Filesize is < 8192\n");
        if(fread(fileBuf, 1, n, fp) <=0)
          printf("Unsuccessful copying of file contents into file buffer\n");
        else{
          printf("server returning a http message with the following content.\n%s\n\n\n",fileBuf);
          write(connfd, fileBuf,strlen(fileBuf));
          fclose(fp);
          return;
        }
      }

      else{
        printf("\n Filesize for: %s is > 8192\n", pathAddOn);
        while(fileSize > 0){
          printf("Currently inside of fileBuf we have: \n%s\n\n\n",fileBuf);
          printf("filesize is now %d\n",fileSize);
          if(fileSize >= 8192){
            if(fread(fileBuf, 1, MAXBUF, fp) <=0){
              printf("Unsuccessful copying of file contents into file buffer, exiting unsuccessfully\n");
              return;
            }
              n = write(connfd, fileBuf,MAXBUF);
              fileSize = fileSize - MAXBUF;
              printf("server returning a http message with the following content.\n%s\n\n\n",fileBuf);
              printf("Filesize is now: %d\n", fileSize);
          }
          else{
            if(fread(fileBuf, 1, fileSize, fp) <=0){
              printf("Unsuccessful copying of file contents into file buffer, exiting unsuccessfully\n");
              return;
            }
              n = write(connfd, fileBuf,fileSize);
              fileSize = fileSize - fileSize;
              n=n-n;
              printf("server returning a http message with the following content.\n%s\n\n\n",fileBuf);
              if(fileSize == 0)
                return;
          }
          bzero(fileBuf, MAXBUF);
        }
      }
  return;
}

/*
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure
 */
int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */
