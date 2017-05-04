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
#define MAX_CLIENTS 100
#define MESSAGE_SIZE 128


char PORT[10];
int total_clients = 0;
int clientid_next = 0;

typedef struct myclient {
  struct sockaddr client_addr;
  int client_fd;
  int client_id;
  char name[32];
} client_t;

client_t *client_list[MAX_CLIENTS];

void *recieve_data(void *args) {
  client_t *pointer = (client_t *)args;
  char buf[MESSAGE_SIZE];
  char message[MESSAGE_SIZE];
  int recvlen;
  while(1) {
    recvlen = recv(pointer->client_fd, buf, sizeof buf, 0);
    if(recvlen>0) {
      strncpy(message, buf, recvlen);
      message[recvlen]='\0';
      printf("%s\n", message);
    }
  }
  pthread_exit(NULL);
}

void *send_data(void *args) {
  char buf[MESSAGE_SIZE];
  int i;
  client_t *pointer = (client_t *)args;
  while (1) {
    scanf("%s", buf);
    for(i=0;i<total_clients;i++) {
      if(client_list[i]->client_fd!=pointer->client_fd)
        send(client_list[i]->client_fd, buf, strlen(buf), 0);
    }
  }
  pthread_exit(NULL);
}

void *handle_clients(void *args) {
  pthread_t recvt, sendt;
  client_t *pointer = (client_t *)args;
  pthread_create(&recvt, NULL, recieve_data, pointer);
  pthread_create(&sendt, NULL, send_data, pointer);
  pthread_join(sendt, NULL);
  pthread_join(recvt, NULL);
  pthread_exit(NULL);
}


void *get_addr(struct sockaddr *sa) {
  if(sa->sa_family==AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  else {
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }
}

client_t *save_client(struct sockaddr *temp, int *fd) {
  client_t *pointer = (client_t *)malloc(sizeof(client_t));
  memcpy(&(pointer->client_addr), temp, sizeof(*temp));
  pointer->client_id = clientid_next;
  pointer->client_fd = *fd;
  char myip[IP_SIZE];
  inet_ntop(pointer->client_addr.sa_family, get_addr(&(pointer->client_addr)), myip, sizeof myip);
  printf("client ip: %s\n", myip);
  return pointer;
}


int main(int argc, char const *argv[]) {
  if(argc!=2) {
    fprintf(stderr, "usage %s [port_number]\n", argv[0]);
    exit(1);
  }
  strncpy(PORT, argv[1], strlen(argv[1]));
  pthread_t tid[MAX_CLIENTS];
  struct addrinfo hints, *res, *p;
  struct sockaddr temp;
  socklen_t addrlen = sizeof temp;
  int aires, yes=1;
  int listener, new_client;
  char message[20]="Helloworld";
  char serverip[IP_SIZE];
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if((aires=getaddrinfo(NULL, PORT, &hints, &res))!=0) {
    fprintf(stderr, "getaddrinfo: failed\n");
    exit(2);
  }

  for(p=res;p!=NULL;p=p->ai_next) {
    listener=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(listener<0) {
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
    fprintf(stderr, "unable to bind\n");
    exit(3);
  }

  inet_ntop(p->ai_family, get_addr(p->ai_addr), serverip, sizeof serverip);
  printf("Server running on %s:%s\n", serverip, PORT);

  freeaddrinfo(res);

  if(listen(listener, 10)==-1) {
    fprintf(stderr, "couldnot listen\n");
    exit(4);
  }

  while(1) {
    new_client = accept(listener, &temp, &addrlen);
    if(new_client<0) {
      continue;
    }
    printf("new connection\n");
    client_list[total_clients] = save_client(&temp, &new_client);
    pthread_create(&tid[total_clients], NULL, handle_clients, client_list[total_clients]);
    total_clients++;
    clientid_next++;
    send(new_client, message, strlen(message), 0);
  }
  return 0;
}
