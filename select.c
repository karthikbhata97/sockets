#include<stdio.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/types.h>
#define STDIN 0
int main(int argc, char const *argv[]) {
  struct timeval tv;
  fd_set readfds;
  char c;

  tv.tv_sec = 5;
  tv.tv_usec = 500000;

  FD_ZERO(&readfds);
  FD_SET(STDIN, &readfds);

  select(STDIN+1, &readfds, NULL, NULL, &tv);
  if(FD_ISSET(STDIN, &readfds)) {
    printf("Key was pressed\n");
    scanf("%c\n", &c);
  }
  else {
    printf("Time out\n");
  }
  return 0;
}
