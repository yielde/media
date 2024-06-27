#ifndef _PROCESS_H_
#define _PROCESS_H_
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>

#include "function.h"

class CProcess {
 public:
  CProcess() {
    m_func = NULL;
    memset(pipes, 0, sizeof(pipes));
  }
  ~CProcess() {
    if (m_func != NULL) {
      delete m_func;
      m_func = NULL;
      close(pipes[0]);
      close(pipes[1]);
    }
  }

  template <typename _FUNCTION_, typename... _ARGS_>
  int setEntryFunction(_FUNCTION_ func, _ARGS_... args) {
    m_func = new CFunction<_FUNCTION_, _ARGS_...>(func, args...);
    return 0;
  }

  int CreateSubProcess() {
    if (m_func == NULL) return -1;

    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pipes);
    pid_t pid = fork();
    if (ret == -1) {
      std::cout << "create socketpair failed!" << std::endl;
      return -2;
    }

    if (pid < 0) {
      std::cerr << "fork failed!" << std::endl;
      return -3;
    }
    if (pid == 0) {
      close(pipes[1]);
      pipes[1] = 0;
      ret = (*m_func)();

      exit(0);
    }
    close(pipes[0]);
    pipes[0] = 0;
    m_pid = pid;
    return 0;
  }

  int sendFD(int fd) {
    msghdr msg;
    bzero(&msg, sizeof(msg));
    struct iovec iov[1];
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr* cmsg = (cmsghdr*)calloc(1, CMSG_LEN(sizeof(int)));
    if (cmsg == NULL) {
      std::cerr << "cmsg has no memory!" << std::endl;
      return -1;
    }

    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    *(int*)CMSG_DATA(cmsg) = fd;

    msg.msg_control = cmsg;
    msg.msg_controllen = CMSG_LEN(sizeof(int));
    printf("UNIX will send msg %d pipes %d\n", fd, pipes[1]);
    ssize_t ret = sendmsg(pipes[1], &msg, 0);
    printf("send msg %d", *(int*)CMSG_DATA(cmsg));
    if (ret == -1) {
      printf("send fd failed! error: %d %s\n", errno, strerror(errno));
      return -2;
    }

    printf("[%s:%d] <%s>(%d) : send fd %d", __FILE__, __LINE__, __FUNCTION__,
           getpid(), *(int*)CMSG_DATA(cmsg));
    free(cmsg);
    return 0;
  }

  int sendIPSocket(int fd, sockaddr_in* addrin) {
    struct msghdr msg;
    iovec iov;
    char buf[20] = "";
    bzero(&msg, sizeof(msg));
    memcpy(buf, addrin, sizeof(sockaddr_in));
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    cmsghdr* cmsg = (cmsghdr*)calloc(1, CMSG_LEN(sizeof(int)));
    if (cmsg == NULL) return -1;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    *(int*)CMSG_DATA(cmsg) = fd;
    msg.msg_control = cmsg;
    msg.msg_controllen = cmsg->cmsg_len;

    ssize_t ret = sendmsg(pipes[1], &msg, 0);
    free(cmsg);
    if (ret == -1) {
      printf("********errno %d msg:%s\n", errno, strerror(errno));
      return -2;
    }
    return 0;
  }
  int recvIPSocket(int& fd, sockaddr_in* addrin) {
    msghdr msg;
    iovec iov;
    char buf[20] = "";
    bzero(&msg, sizeof(msg));
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    cmsghdr* cmsg = (cmsghdr*)calloc(1, CMSG_LEN(sizeof(int)));
    if (cmsg == NULL) return -1;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    msg.msg_control = cmsg;
    msg.msg_controllen = CMSG_LEN(sizeof(int));
    ssize_t ret = recvmsg(pipes[0], &msg, 0);
    if (ret == -1) {
      free(cmsg);
      return -2;
    }
    memcpy(addrin, buf, sizeof(sockaddr_in));
    fd = *(int*)CMSG_DATA(cmsg);
    printf("received fd : %d, received buff : %s\n", fd, buf);
    free(cmsg);
    return 0;
  }

  int recvFD(int& fd) {
    msghdr msg;
    bzero(&msg, sizeof(msg));
    struct iovec iov[1];
    char buf[0];
    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr* cmsg = (cmsghdr*)calloc(1, CMSG_LEN(sizeof(int)));
    if (cmsg == NULL) {
      std::cerr << "cmsg has no memory!" << std::endl;
      return -1;
    }

    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    msg.msg_control = cmsg;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    ssize_t ret = recvmsg(pipes[0], &msg, 0);
    if (ret == -1) {
      printf("receive fd failed!, error: %d %s\n", errno, strerror(errno));
      free(cmsg);
      return -2;
    }

    fd = *(int*)CMSG_DATA(cmsg);

    free(cmsg);
    return 0;
  }

  static int switchDeamon() {
    int ret = fork();
    if (ret < 0) return -1;
    if (ret > 0) exit(0);
    ret = setsid();
    if (ret == -1) {
      return -2;
    }
    ret = fork();
    if (ret < 0) return -3;
    if (ret > 0) exit(0);
    int nullFd = open("/dev/null", O_RDWR);
    dup2(nullFd, STDOUT_FILENO);
    dup2(nullFd, STDIN_FILENO);
    dup2(nullFd, STDERR_FILENO);

    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);

    umask(0);
    signal(SIGCHLD, SIG_IGN);
    return 0;
  }

 private:
  CFunctionBase* m_func;
  pid_t m_pid;
  int pipes[2];
};

#endif