#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#define MYPORT "3003"
#define BACKLOG 10

int main(int argc, char const *argv[]) {
  void *addr;
  socklen_t client_addr_size;
  char ipstr[INET6_ADDRSTRLEN];
  int addrinfo_status, sockfd, client_sfd;
  struct sockaddr_storage client_addr;
  struct addrinfo hints, *res, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if((addrinfo_status=getaddrinfo(NULL, MYPORT, &hints, &res))!=0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrinfo_status));
    return 1;
  }

  if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1) {
    fprintf(stderr, "socketfd failed with: %d\n", sockfd);
    return 2;
  }

  if(bind(sockfd, res->ai_addr, res->ai_addrlen)==-1) {
    fprintf(stderr, "Bind failed\n");
    return 3;
  }

  if(listen(sockfd, BACKLOG)==-1) {
    fprintf(stderr, "Couldn't listen\n");
    return 4;
  }

  client_addr_size = sizeof(client_addr);
  if((client_sfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_size))==-1) {
    fprintf(stderr, "Couldn't accept\n");
    return 5;
  }

  if(client_addr.ss_family==AF_INET) {
    struct sockaddr_in *inet4 = (struct sockaddr_in *)&client_addr;
    addr = &(inet4->sin_addr);
  }
  else {
    struct sockaddr_in6 *inet6 = (struct sockaddr_in6 *)&client_addr;
    addr = &(inet6->sin6_addr);
  }

  char *msg = "HelloWorld\n";
  if(send(client_sfd, msg, strlen(msg), 0)!=strlen(msg)) {
    fprintf(stderr, "Couldn't send message properly\n");
    return 7;
  }

  char income[15];
  recv(client_sfd, income, 15, 0);
  printf("%s\n", income);

  inet_ntop(client_addr.ss_family, addr, ipstr, sizeof(ipstr));
  printf("Client address is %s\n", ipstr);
  printf("Client socket descriptor: %d\n", client_sfd);
  close(client_sfd);

/*
  Kill the bad boy hanging around
  begin
*/
  int yes = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))==-1) {
    fprintf(stderr, "Failed to kill the beast\n");
    return 6;
  }
/*
  end
  That's simple
*/

  return 0;
}
