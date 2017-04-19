#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#define BUF_SIZE 256
#define MY_PORT "3003"

void *get_in_addr(struct sockaddr *sa) {
  if(sa->sa_family==AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  else
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char const *argv[]) {
  fd_set master, read_fds;
  int listener, new_fd;
  int fdmax;

  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;

  char buf[BUF_SIZE];
  int nbytes;

  char remoteIP[INET6_ADDRSTRLEN];

  int yes=1;
  int i, j, rv;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  struct addrinfo hints, *res, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if((rv=getaddrinfo(NULL, MY_PORT, &hints, &res))!=0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  for(p=res;p!=NULL;p=p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(listener<0){
      continue;
    }

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if(bind(listener, p->ai_addr, p->ai_addrlen)<0) {
      close(listener);
      continue;
    }

    break;
  }

  if(p==NULL) {
    fprintf(stderr, "bind: failed\n");
    exit(2);
  }

  inet_ntop(p->ai_family, get_in_addr(p->ai_addr), remoteIP, sizeof(remoteIP));
  printf("myIP: %s\n", remoteIP);

  freeaddrinfo(res);

  if(listen(listener, 10)==-1) {
    perror("listen");
    exit(3);
  }

  FD_SET(listener, &master);

  fdmax = listener;

  for(;;) {
    read_fds = master;
    if(select(fdmax+1, &read_fds, NULL, NULL, NULL)==-1) {
      perror("select");
      exit(4);
    }
    for(i=0;i<=fdmax;i++) {
      printf("%d\n", i);
      if(FD_ISSET(i, &read_fds)) {
        addrlen = sizeof(remoteaddr);
        new_fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
        if(new_fd==-1) {
          perror("accept");
        }
        else {
          FD_SET(new_fd, &master);
          if(new_fd>fdmax) {
            fdmax = new_fd;
          }
          printf("selectserver: new connection from: %s on socket %d\n",
                  inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr),
                  remoteIP, sizeof(remoteIP)), new_fd);
        }
      }
      else {
        if((nbytes=recv(i, buf, sizeof(buf), 0))<=0) {
          if(nbytes==0) {
            printf("socket closed by client %d\n", i);
          }
          else {
            perror("recv");
          }
          close(i);
          FD_CLR(i, &master);
        }
        else {
          for(j=0;j<=fdmax;j++) {
            if(FD_ISSET(j, &master)) {
              if(j!=listener && j!=i) {
                if(send(j, buf, nbytes, 0)==-1) {
                  perror("send");
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
