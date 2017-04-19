#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<errno.h>

#define MAX_CLIENTS 50
#define PATH_LEN 64
#define MSG_LEN 65536
#define BYTES 65536

char PORT[6];
char *ROOT;
int listener;
int clients[MAX_CLIENTS];

void startServer();
void respond(int);
void clearClients();

int main(int argc, char * const *argv) {
  clearClients();
  strcpy(PORT, "3003");
  ROOT = getenv("PWD");
  char c;
  while((c=getopt(argc, argv, "p:r:"))!=-1) {
    switch (c) {
      case 'r':
        ROOT = malloc(strlen(optarg));
        strcpy(ROOT, optarg);
        break;

      case 'p':
        strcpy(PORT, optarg);
        break;

      default:
        fprintf(stderr, "usage: http -p PORT -r DIR\n");
        exit(1);
    }
  }
  startServer();
  printf("Server running on port %s serving directory %s\n", PORT, ROOT);
  int slot=0;
  struct sockaddr_storage client_addr;
  socklen_t addrlen;
  while(1) {
    addrlen = sizeof client_addr;
    clients[slot] = accept(listener, (struct sockaddr *)&client_addr, &addrlen);

    if(clients[slot]==-1) {
      perror("accept");
      continue;
    }

    if(fork()==0) {
      respond(slot);
      exit(0);
    }
    while(clients[slot]!=-1) slot = (slot+1)%MAX_CLIENTS;
  }

  return 0;
}

void startServer() {
  struct addrinfo hints, *res, *p;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  int rv, yes=1;

  if((rv=getaddrinfo(NULL, PORT, &hints, &res))!=0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(2);
  }

  for(p=res; p!=NULL; p=p->ai_next) {
    if((listener=socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0) {
      continue;
    }

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if(bind(listener, p->ai_addr, p->ai_addrlen)==-1) {
      close(listener);
      continue;
    }

    break;
  }
  if(p==NULL) {
    fprintf(stderr, "bind: failed\n");
    exit(3);
  }
  if((listen(listener, 50))==-1) {
    perror("listen");
    exit(4);
  }
  freeaddrinfo(res);
}

void respond(int slot) {
  int clientfd = clients[slot];
  char message[MSG_LEN], *reqline[3], data[BYTES], path[PATH_LEN];
  int nbytes, fd, recvd;
  memset(message, '\0', MSG_LEN);
  recvd = recv(clientfd, message, MSG_LEN, 0);

  if(recvd<0) {
    perror("recv");
    return;
  }
  else if(recvd==0) {
    printf("Connection closed by socket: %d\n", clientfd);
    clients[slot] = -1;
    return;
  }
  else {
    printf("Client %d: %s\n", clientfd, message);
    reqline[0] = strtok(message, " \t\n");
    if(strncmp(reqline[0], "GET\0", 4)==0) {
      reqline[1] = strtok(NULL, " \t");
      reqline[2] = strtok(NULL, " \t\n");

      if(strncmp(reqline[2], "HTTP/1.0", 8)!=0 && strncmp(reqline[2], "HTTP/1.1", 8)!=0) {
        write(clientfd, "HTTP/1.0 400 Bad Request\n", 25);
      }
      else {
        if(strncmp(reqline[1], "/\0", 2)==0) {
          reqline[1] = "/index.html";
        }
        strcpy(path, ROOT);
        strcpy(&path[strlen(ROOT)], reqline[1]);
        printf("File: %s\n", path);

        if((fd=open(path, O_RDONLY))==-1) {
          write(clientfd, "Cannot GET required file\n", 25);

        }
        else {
          write(clientfd, "HTTP/1.0 200 OK\n\n", 17);
          while(nbytes=read(fd, data, BYTES)) {
            write(clientfd, data, nbytes);
          }
        }
      }
    }
  }

  shutdown (clientfd, SHUT_RDWR);
  close(clientfd);
  clients[slot]=-1;
}

void clearClients() {
  int i=0;
  for(i=0;i<MAX_CLIENTS;i++)
    clients[i]=-1;
}
