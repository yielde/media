#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void send_fd(int socket, int fd_to_send) {
  struct msghdr msg = {0};
  struct cmsghdr *cmsg;
  char buf[CMSG_SPACE(sizeof(fd_to_send))], dup[256];
  struct iovec io = {.iov_base = &dup, .iov_len = sizeof(dup)};

  msg.msg_iov = &io;
  msg.msg_iovlen = 1;

  msg.msg_control = buf;
  msg.msg_controllen = sizeof(buf);

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(fd_to_send));

  *((int *)CMSG_DATA(cmsg)) = fd_to_send;

  if (sendmsg(socket, &msg, 0) < 0) perror("Failed to send message");
}

int recv_fd(int socket) {
  struct msghdr msg = {0};
  struct cmsghdr *cmsg;
  char buf[CMSG_SPACE(sizeof(int))], dup[256];
  struct iovec io = {.iov_base = &dup, .iov_len = sizeof(dup)};
  int received_fd;

  msg.msg_iov = &io;
  msg.msg_iovlen = 1;

  msg.msg_control = buf;
  msg.msg_controllen = sizeof(buf);

  if (recvmsg(socket, &msg, 0) < 0) {
    perror("Failed to receive message");
    exit(EXIT_FAILURE);
  }

  cmsg = CMSG_FIRSTHDR(&msg);

  if (cmsg == NULL || cmsg->cmsg_type != SCM_RIGHTS) {
    fprintf(stderr, "Invalid control message received\n");
    exit(EXIT_FAILURE);
  }

  received_fd = *((int *)CMSG_DATA(cmsg));
  return received_fd;
}

int main() {
  int sv[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
    perror("socketpair");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {  // Child process
    close(sv[0]);  // Close parent's socket

    // Receive the file descriptor
    int fd = recv_fd(sv[1]);
    printf("Child received file descriptor: %d\n", fd);

    // Read from the received file descriptor
    char buf[100];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    buf[n] = '\0';
    printf("Child read: %s\n", buf);

    close(fd);
    close(sv[1]);
  } else {         // Parent process
    close(sv[1]);  // Close child's socket

    // Open a file and send its file descriptor
    int fd = open("testfile.txt", O_RDONLY);
    if (fd == -1) {
      perror("open");
      exit(EXIT_FAILURE);
    }
    send_fd(sv[0], 33);
    close(fd);

    close(sv[0]);
  }

  return 0;
}
