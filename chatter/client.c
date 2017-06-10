#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<pthread.h>
#define IP_SIZE 40
#define MESSAGE_SIZE 128
char PORT[10];
char serverip[IP_SIZE];


void *sender(void *args) {
  int fd = *(int *)args;
  int len;
  char message[MESSAGE_SIZE];
  while(1) {
    scanf("%s", message);
    len = send(fd, message, sizeof(message), 0);
    if(len==-1) {
      printf("failed sending %s\n", message);
    }
  }
  pthread_exit(NULL);
}

void *reciever(void *args) {
  int fd = *(int *)args;
  char message[MESSAGE_SIZE];
  int len;
  len=recv(fd, message, sizeof(message), 0);
  message[len]='\0';
  printf("%s", message);
  while(1) {
    len=recv(fd, message, sizeof(message), 0);
    if(len==0) {
      printf("server shutdown\n");
      pthread_exit(NULL);
    }
    message[len]='\0';
    printf("%s\n", message);
  }
  pthread_exit(NULL);
}


int main(int argc, char const *argv[]) {
  if(argc!=3) {
    fprintf(stderr, "usage: %s serverip port\n", argv[0]);
    exit(1);
  }
  strncpy(serverip, argv[1], strlen(argv[1]));
  strncpy(PORT, argv[2], strlen(argv[2]));
  struct addrinfo hints, *res, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  int aires, sockfd, yes=1;
  pthread_t sendt, recvt;

  if((aires=getaddrinfo(serverip, PORT, &hints, &res))!=0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(aires));
    exit(2);
  }

  for(p=res;p!=NULL;p=p->ai_next) {
    sockfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(sockfd<0) {
      continue;
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(connect(sockfd, p->ai_addr, p->ai_addrlen)) {
      continue;
    }
    break;
  }

  if(p==NULL) {
    fprintf(stderr, "failed to connect to the server\n");
    exit(3);
  }

  printf("connection eshtablished\n");
  pthread_create(&sendt, NULL, sender, &sockfd);
  pthread_create(&recvt, NULL, reciever, &sockfd);
  pthread_join(sendt, NULL);
  pthread_join(recvt, NULL);
  return 0;
}
