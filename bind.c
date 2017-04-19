#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>

int main(int argc, char *argv[]) {
  char ipstr[INET6_ADDRSTRLEN];
  int status;
  int sockfd;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if((status=getaddrinfo(NULL, "3000", &hints, &res))!=0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 1;
  }

  char *ipver;
  void *addr;

  if(res->ai_family==AF_INET) {
    ipver="IPv4";
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    addr = &(ipv4->sin_addr);
  }
  else {
    ipver = "IPv6";
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
    addr = &(ipv6->sin6_addr);
  }

  inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
  printf("%s: %s\n", ipver, ipstr);

  printf("%d: %d: %d\n", res->ai_family, res->ai_socktype, res->ai_protocol);
  printf("%d\n", sockfd);

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  bind(sockfd, res->ai_addr, res->ai_addrlen);

  return 0;
}
