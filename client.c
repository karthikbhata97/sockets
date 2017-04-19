#include <unistd.h>
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<errno.h>

int main(int argc, char const *argv[]) {
  if(argc!=3) {
    printf("Usage: client destination port\n");
    return 1;
  }
  int aistatus, sockfd;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if((aistatus=getaddrinfo(argv[1], argv[2], &hints, &res))!=0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(aistatus));
    return 2;
  }

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  connect(sockfd, res->ai_addr, res->ai_addrlen);
  char *msg = "HelloWorld";
  send(sockfd, msg, strlen(msg), 0);
  char income[15];
  recv(sockfd, income, 15, 0);
  printf("%s\n", income);
  close(sockfd);
  return 0;
}
