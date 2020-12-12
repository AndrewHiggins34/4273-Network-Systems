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
#include <dirent.h> // included for 'ls' call and 'readdir'/dirent struct



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
    char username[20];

    if (argc != 2) {
    	fprintf(stderr, "usage: %s <port>\n", argv[0]);
    	exit(0);
    }

    port = atoi(argv[1]);
    printf("This is the server for port %d speaking...\n", port);
    listenfd = open_listenfd(port);

    fp

    if()
    while (1) {
    	connfdp = malloc(sizeof(int));  // pointer to pass the socket
    	*connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
    	pthread_create(&tid, NULL, thread, connfdp);
    }
}

int credentialVerify(char )

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
{
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
  printf("Server recieved the following request: %s, for %s\n", word1, file);

  /* store the dfc.conf usernames/passwords *///////////////////////////////////
  FILE *config = fopen("dfc.conf", "r");  // just giving it read rights for now.
  if(config == NULL){
    printf("Invalid conf file passed, can't verify passwords, terminating connection\n");
    close(connfd);
    exit(0);
  }
  struct userData { // only storing 4 here bc my conf file only has 4 user/passwords
    char username[4][20];
    char password[4][20];
  };
  char tempBuf[40], user[20], pass[20];
  int datacnt = 0;
  struct userData userDB;
  while (fgets(tempBuf, 161, config) != NULL)
  {
    sscanf(tempBuf, "%s" "%s", user, pass);
    memcpy(userDB.username[datacnt], user, 20);
    memcpy(userDB.password[datacnt], pass, 20);
    // printf("captured user: %s pass: %s\n", userDB.username[datacnt], userDB.password[datacnt]);
    bzero(tempBuf, 40); bzero(user, 20); bzero(pass, 20);
    datacnt++;
  }
  //////////////////////////////////////////////////////////////////////////////

  /*prepare the switch *////////////////////////////////////////////////////////
  int value = 0;
  if(strcmp(word1, "LIST") == 0) value=1;
  else if (strcmp(word1, "GET") == 0) value=2;
  else if (strcmp(word1, "PUT") == 0) value=3;
  switch(value)
  {
    case 1: // "LIST" case

      bzero(buf, MAXBUF);
      struct dirent *currDirectory;
      DIR *dirp = opendir(".");
      if(dirp == NULL)
        printf("Somehow can't access the current directory");
      else
      {
        int i = 0;
        printf("---------The following directories are being passed--------\n");
        while((currDirectory = readdir(dirp)) != NULL)
        {
          strcat(buf, currDirectory->d_name);
          strcat(buf, "\n");
        }
          printf("%s\n", buf);
      }
      closedir(dirp);
      free(currDirectory);
      n = send(connfd, buf, strlen(buf), 0);
      break;

    case 2: // "GET case
      break;

    case 3: // "PUT" case
      break;

    default:
      printf("Server recieved an invalid command, terminating connection");
      exit(0);
  } /* end of switch(value) */
  //////////////////////////////////////////////////////////////////////////////
} /* end of doServerStuff */

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
