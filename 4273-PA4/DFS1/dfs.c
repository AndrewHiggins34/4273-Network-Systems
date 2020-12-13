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
#define HEAPBUF 32768 // 2^15


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

int verifyCredential(char* userN, char* passN)
{
  /* store the dfc.conf usernames/passwords *///////////////////////////////////
  FILE *config = fopen("dfc.conf", "r");  // just giving it read rights for now.
  if(config == NULL){
    printf("Invalid conf file passed, can't verify passwords, terminating connection\n");
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
    printf("checking user: %s pass:%s\n", userN, passN);
    sscanf(tempBuf, "%s" "%s", user, pass);
		memcpy(userDB.username[datacnt], user, 20);
    memcpy(userDB.password[datacnt], pass, 20);
		printf("captured user: %s pass: %s\n", userDB.username[datacnt], userDB.password[datacnt]);

    if(strcmp(userN, userDB.username[datacnt]) == 0 && strcmp(passN, userDB.password[datacnt]) == 0){
      printf("Credentials verified\n");
      fclose(config);
			return 1;
		}
		bzero(tempBuf, 40); bzero(user, 20); bzero(pass, 20);
		datacnt++;
	}
  fclose(config);
	return -1;
}

void doServerStuff(int connfd)
{
  size_t n;
  int fileSize;
  char buf[MAXBUF]; bzero(buf, MAXBUF);
  char buffer[MAXBUF];

  /* read the request */
  n = read(connfd,buffer, MAXBUF);
  printf("server received the following request:\n%s\n",buffer);

  /* parse the request */
  char word1[5];
  char file[MAXFILESIZE];
  sscanf(buffer, "%s %s", word1, file);
  printf("Server recieved the following request: %s, for %s\n", word1, file);


  //////////////////////////////////////////////////////////////////////////////


int botherUser()
{
  // /* Prompt user for username and password */
  // if((n = send(connfd, "Enter username(enter)\n Then password(enter)\n", 46, 0)) < 0){
  //   perror("Message failed to send (LIST)\n");
  //   return -1;
  // }
  /* parse & verify username and password */
  bzero(buf, MAXBUF);
  char userN[20], passN[20];
  read(connfd, buf, MAXBUF);
  sscanf(buf, "%s %s",userN, passN);
  if(verifyCredential(userN, passN) != 1){
    printf("Invalid credentials\n");
    return -1;;
  }
  bzero(buf, MAXBUF);
}

  /*prepare the switch *////////////////////////////////////////////////////////
  int value = 0;
  if(strcmp(word1, "LIST") == 0) value=1;
  else if (strcmp(word1, "GET") == 0) value=2;
  else if (strcmp(word1, "PUT") == 0) value=3;
  switch(value)
  {
    case 1: // "LIST" case
      if(botherUser() == -1)
        break;
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
      if(botherUser() == -1)
        break;
        /* read the file into the buffer. */
        int n1,n2;
        char c1, c2;  // store the last character
        int i1, i2; // to find the length of the buffer
        char* buf4Files1 = malloc(HEAPBUF); bzero(buf4Files1, HEAPBUF);
        char* buf4Files2 = malloc(HEAPBUF); bzero(buf4Files2, HEAPBUF);
        n1 = read(connfd, buf4Files1, HEAPBUF);
        n2 = read(connfd, buf4Files2, HEAPBUF);
        i1 = strlen(buf4Files1);
        i2 = strlen(buf4Files2);
        c1 = buf4Files1[(i1-1)];
        c2 = buf4Files2[(i2-1)];
        printf("\n c1 = %c\n", c1);
        printf("\n c2 = %c\n", c2);

        FILE* file1 = fopen(file, "w+");
        if(fwrite(buf4Files1, n1, 1, file1) < 0)
          printf("ERROR1234\n");
        else
          printf("File: %s should be successfully uploaded\n", file);
        strcat(file, "2");
        FILE* file2 = fopen(file, "w+");
        if(fwrite(buf4Files1, n2, 1, file2) < 0)
          printf("ERROR4321\n");
        else
          printf("File: %s should be successfully uploaded\n", file);
          printf("\n\n %s \n\n", buf4Files1);
          printf("\n\n %s \n\n", buf4Files2);

        fclose(file2);
        fclose(file1);
        free(buf4Files1);
        free(buf4Files2);
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
