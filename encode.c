#include<stdio.h>
#include <arpa/inet.h>

int main() {
  int a, b;
  scanf("%d", &b);
  printf("%d\n", htons(b)&(0x00FF));
  printf("%d\n", htons(b)&(0xFF00));
  printf("%lu\n", sizeof(htons(b)));
}
