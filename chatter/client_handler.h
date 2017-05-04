#include<pthread.h>
#include<stdio.h>
#define MESSAGE_SIZE 128

void *recieve(void *args) {
  client_t *pointer = (client_t *)args;
  char buf[MESSAGE_SIZE];
  char message[MESSAGE_SIZE];
  int recvlen;
  while(1) {
    len = recv(pointer->client_fd, buf, sizeof buf, 0);
    if(len>0) {
      strncpy(message, buf, len);
      message[len]='\0';
      printf("%s\n", message);
    }
  }
  pthread_exit(NULL);
}

void *send(void *args) {
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
  client_t *pointer = (client_t *)args;
  pthread_create(NULL, NULL, recieve, pointer);
  pthread_create(NULL, NULL, send, pointer);
  pthread_join(NULL, NULL);
  pthread_exit(NULL);
}
