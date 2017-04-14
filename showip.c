#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>

int main(int argc, char *argv[]) {
  if(argc!=2) {
    fprintf(stderr, "Invalid arguments!\nUsage: showip hostname\n");
    return(1);
  }
  struct addrinfo hints, *res, *p;
  memset(&hints, 0, sizeof(hints));
  int status;
  char ipstr[INET6_ADDRSTRLEN];
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if((status=getaddrinfo(argv[1], NULL, &hints, &res))!=0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return(2);
  }

  printf("IP address for %s\n", argv[1]);

  for(p=res; p!=NULL; p=(p->ai_next)) {
    void *addr;
    char *ipver;
    if(p->ai_family == AF_INET) {
      struct sockaddr_in *inet4 = (struct sockaddr_in *)p->ai_addr;
      ipver = "IPv4";
      addr = &(inet4->sin_addr);
    }
    else {
      struct sockaddr_in6 *inet6 = (struct sockaddr_in6 *)p->ai_addr;
      ipver = "IPv6";
      addr = &(inet6->sin6_addr);
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    printf("%s: %s\n", ipver, ipstr);
  }

  freeaddrinfo(res);

  return 0;
}
