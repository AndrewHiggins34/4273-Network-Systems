/* Proxy application called webproxy */

// Copy past http server includes, remove unnecessary
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

#define MAXBUF  8192  /* max text line length */
#define LISTENQ  1024  /* second argument to listen() */
#define ERRBUFSIZE 1024
#define HEAPBUF 32768 // 2^15

int open_listenfd(int port);
void webProxy(int connfd);
void *thread(void *vargp);
int connect2Client(char* ip, char* port);
void sendErrResponse(char* errBUF, int connfd);
int forwardClientReq(char* buffer, char* host, int clientfd);


int main(int argc, char **argv)
{
  int timeout = 0;
  /* Check if the function was called correctly */
  /* Store arguments provided (port number & cache timeout) */
  if(argc < 2  || argc > 3){
    fprintf(stderr, "Incorrect arguments, Usage: ./<executableFile> <port#> <timeout(optional)>\n");
    exit(0);
  }
  if(argc == 3)
    timeout = atoi(argv[2]);

  int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in);
  pthread_t tid;
  struct sockaddr_in clientaddr;

  port = atoi(argv[1]); // store the port number
  listenfd = open_listenfd(port); // create persistant TCP socket for client HTTP requests

  /* Create,Bind,Listen SOCKET and multithread */
  while (1) {
    connfdp = malloc(sizeof(int));  // pointer to pass the socket
    *connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
    if(*connfdp<0)
      printf("There is an error accepting the connection with the client");
    pthread_create(&tid, NULL, thread, connfdp);  // call thread function with tid
  }
}

/* thread routine */
void * thread(void * vargp)
{
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    webProxy(connfd);
    // close(connfd);
    return NULL;
}

void webProxy(int connfd)
{
  size_t n;
  char buffer[MAXBUF];  // pointer to pass the socket
  char errorBuf[ERRBUFSIZE];
  bzero(errorBuf, ERRBUFSIZE);
  bzero(buffer, MAXBUF);
  char httpmsg[]="HTTP/1.1 200 Document Follows\r\nContent-Type:text/html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>";
  char hostname[50];

  n = read(connfd, buffer, MAXBUF);  // read the requst up to maxbuf sizeof
  printf("server received the following request:\n%s\n",buffer);


  /* parse the request */
  char requestType[50], fullPath[50];
  bzero(requestType, 50);
  bzero(fullPath, 50);
  sscanf(buffer, "%s %s", requestType, fullPath);
  printf("requestType = %s, and fullPath= %s\n", requestType, fullPath);
  /* I tried a million different methods and libary suggestions but this is the only
  method that I found to easily parse the information I wanted to input */
  // sscanf(fullPath, "http://www.%511[^/\n]", hostname);
  sscanf(fullPath, "http://%511[^/\n]", hostname);
  printf("hostname is = %s\n", hostname);


  /* support only GET requests */
  if(strcmp(requestType, "GET") != 0)
  {
    printf("Proxy received a request that was not a 'GET' Request, sending 400 Bad Request response\n");
    sendErrResponse(errorBuf, connfd);
  }
  // // /* support only HTTP/1.1 */
  // else if(strcmp(Type, "HTTP/1.1") != 0)
  // {
  //   printf("Proxy received a request that was not an HTTP/1.1 version, sending 400 Bad Request response");
  //   // I think the error message below is 80, but I could be wrong. Need to double check or use strlen function
  //   sprintf(errorBuf, "HTTP/1.1 400 Bad Request\r\nContent-Type:text/html\r\nContent-Length: %d\r\n\r\n",80);
  //   write(connfd, errorBuf, strlen(errorBuf));
  //   bzero(errorBuf, ERRBUFSIZE);
  // }

  /* parse and verify the hostname/server */
  struct hostent *host = gethostbyname(hostname);
  if (host == NULL) {
      fprintf(stderr,"ERROR, no such host as %s\n", hostname);
      sendErrResponse(errorBuf, connfd);
      exit(0);
  }
  else{
    /* Forward request to HTTP server */
    forwardClientReq(buffer, hostname, connfd);

    /* Relay data from server to client */
  }


}

int forwardClientReq(char* buffer, char* hostname, int clientfd)
{
  char* serverResponse = malloc(HEAPBUF);  // create a buffer for receiving the response from the server
  bzero(serverResponse, HEAPBUF); // zeroize the buffer

  /* open a socket with the server */
  struct sockaddr_in serveraddr;
  int serverSock, optval =1;

  /* Create a socket descriptor */
  if(-1 == (serverSock = socket(AF_INET, SOCK_STREAM, 0)))  // yoda condition...
  {
    printf("Error: Unable to create socket in 'open_listenfd' function");
    return -1;
  }
  /* Eliminates "Address already in use" error from bind. */
  if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &optval , sizeof(int)) < 0){
    printf("Error in setsockopt in forwardClientReq function");
    return -1;
  }

  struct hostent* host = gethostbyname(hostname);

  serveraddr.sin_family = AF_INET;
  memcpy(&serveraddr.sin_addr, host->h_addr_list[0], host->h_length);
  serveraddr.sin_port = htons(80); //(you should pick the correct remote port or use the default 80 port if noneis specified).

  socklen_t addrlen = sizeof(serveraddr); // The addrlen argument specifies the size of serveraddr.
  int serverfd = connect(serverSock, (struct sockaddr*) &serveraddr, (socklen_t)sizeof(serveraddr));
  if(serverfd < 0)
    printf("improper connection when trying to forward client's request\n");


  /* send the client message to the server */
  int numBytesSent = 0;
  printf("buffer before send = %s\n", buffer);
  strcat(buffer, "\r\n\r\n");
  numBytesSent = send(serverSock, buffer, strlen(buffer), 0);
  if(numBytesSent == -1){
    printf("Oh dear, something went wrong with send()! %s\n", strerror(errno));
    return -1;
  }
  printf("Sent %d to the server\n", numBytesSent);

  /* store the server's response */
  int numBytesRead = 0;
  numBytesRead = read(serverSock, serverResponse, HEAPBUF);
  printf("Read %d to the proxy-buffer\n", numBytesRead);

  /* send the response to the client. This is a moment here the proxy can cache the page,
  inspect the data, and do all kinds of cool proxy-level things */
  numBytesSent = write(clientfd, serverResponse, strlen(serverResponse));
  printf("Sent %d back to the client\n", numBytesSent);

  free(serverResponse); // free malloc'd space
}

int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("Error: Unable to create socket in 'open_listenfd' function");
      return -1;
    }
    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval , sizeof(int)) < 0)
        return -1;
    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    /* bind: associate the parent socket with a port */
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */

void sendErrResponse(char* errorBuf, int connfd)
{
  // I think the error message below is 80, but I could be wrong. Need to double check or use strlen function
  sprintf(errorBuf, "HTTP/1.1 400 Bad Request\r\nContent-Type:text/html\r\nContent-Length: %d\r\n\r\n",80);
  write(connfd, errorBuf, strlen(errorBuf));
  bzero(errorBuf, ERRBUFSIZE);
}
