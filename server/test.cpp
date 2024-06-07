#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

void *threadFunc(void *) {
  printf("pid: %d , thread: %ld, %d\n", getpid(), pthread_self(), gettid());
}

int main() {
  pthread_t thread;
  pthread_create(&thread, NULL, threadFunc, NULL);

  printf("main: pid %d, tid: %d\n", getpid(), gettid());
  char A[] = "设";
  printf("%d\n", sizeof(A));
  printf("%X\n", A[0]);
  printf("%X\n", A[1]);
  printf("%X\n", A[2]);
  printf("%X\n", A[4]);

  return 0;
}