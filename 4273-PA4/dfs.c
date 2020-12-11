/*
 * Distributed File System Server (DFS)
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
#define MAXFILESIZE 40   /* Expected max filesize */

int open_listenfd(int port);
void doServerStuff(int connfd);
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
    doServerStuff(connfd);
    return NULL;
}

void doServerStuff(int connfd)
  size_t n;
  int fileSize;
  char buf[MAXBUF]; bzero(buf, MAXBUF);

  /* read the request */
  n = read(connfd,buf, MAXBUF);
  printf("server received the following request:\n%s\n",buf);

  /* parse the request */
  char word1[5];
  char file[MAXFILESIZE];
  sscanf(buf, "%s %s", word1, file);
