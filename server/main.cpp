#include <fcntl.h>
#include <memory.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <functional>
#include <iostream>

#include "epoll.h"
#include "logger.h"
#include "process.h"
#include "server.h"
#include "threadpool.h"

int LogTest() {
  char buffer[] = "galen! new new new say 设置 test\n";
  TRACEI("here is log test: %d %c %f %g %s 你好 %s", 10, 'A', 1.2f, 2.3, buffer,
         "晖晖");
  // DUMPD(buffer, sizeof(buffer));
  // LOGI << 100 << " " << 's' << " " << 0.123f << " " << 2.3 << " " << buffer
  //      << "中国";

  return 0;
}

int LogTest1() {
  char buffer[] = "hello hello galen! 设置 test1 \n";
  DUMPD(buffer, sizeof(buffer));

  return 0;
}

int LogTest2() {
  char buffer[] = "galen! 设置 test2\n";
  DUMPD(buffer, sizeof(buffer));

  return 0;
}

int createLogServer(CProcess* proc) {
  CLoggerServer server;  // m_server_1
  int ret = server.Start();
  if (ret != 0) {
    printf("[%d]create server error!: errno:%d\n", getpid(), ret);
    return ret;
  }
  int fd = 2;
  while (true) {
    proc->recvFD(fd);
    if (fd <= 0) {
      printf("received fd, %d, will close server!\n", fd);
      break;
    }
  }
  server.Close();
  return 0;
}

int createClientServer(CProcess* proc) {
  printf("[%s:%d] <%s>(%d) : creat client server\n", __FILE__, __LINE__,
         __FUNCTION__, getpid());
  int fd = -1;

  int ret = proc->recvFD(fd);
  printf("[%s:%d] <%s>(%d) : client receve fd %d\n", __FILE__, __LINE__,
         __FUNCTION__, getpid(), fd);
  char buf[10];
  memset(buf, '\0', 10);
  lseek(fd, 0, SEEK_SET);
  read(fd, buf, 10);
  printf("[%s:%d] <%s>(%d) : client receve buf %s\n", __FILE__, __LINE__,
         __FUNCTION__, getpid(), buf);
  close(fd);
  return 0;
}

int main() {
  printf("start server~\n");
  // CProcess::switchDeamon();

  CProcess proclog;
  int ret = 0;
  proclog.setEntryFunction(createLogServer, &proclog);
  ret = proclog.CreateSubProcess();
  if (ret != 0) return -1;
  sleep(3);

  LogTest1();
  LogTest1();
  LogTest1();

  LOGI << "start thread1";
  CThread thread1(LogTest);
  thread1.Start();

  CThreadPool pool;
  ret = pool.Start(4);
  LOGI << "start thread pool" << " ret: " << ret;
  if (ret != 0)
    TRACEI("error: ret = %d, error = %d, %s", ret, errno, strerror(errno));
  ret = pool.AddTask(LogTest2);
  if (ret != 0)
    TRACEI("error: ret = %d, error = %d, %s", ret, errno, strerror(errno));
  ret = pool.AddTask(LogTest2);
  if (ret != 0)
    TRACEI("error: ret = %d, error = %d, %s", ret, errno, strerror(errno));
  ret = pool.AddTask(LogTest2);
  if (ret != 0)
    TRACEI("error: ret = %d, error = %d, %s", ret, errno, strerror(errno));
  ret = pool.AddTask(LogTest2);
  if (ret != 0)
    TRACEI("error: ret = %d, error = %d, %s", ret, errno, strerror(errno));
  (void)getchar();
  // sleep(5);
  proclog.sendFD(-1);
  TRACEI("main process send fd !\n");
  (void)getchar();
  // sleep(5);
  printf("over!\n");
  return 0;
}

// int main() {
//   printf("start main: \n");
//   CProcess::switchDeamon();

//   CProcess proclog, procclient;
//   proclog.setEntryFunction(createLogServer, &proclog);
//   int ret = proclog.CreateSubProcess();

//   if (ret != 0) {
//     return -1;
//   }

//   procclient.setEntryFunction(createClientServer, &procclient);
//   ret = procclient.CreateSubProcess();
//   if (ret != 0) {
//     return -2;
//   }

//   int fd = open("./test.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);

//   printf("[%s:%d] <%s>(%d): main fd %d\n", __FILE__, __LINE__, __FUNCTION__,
//          getpid(), fd);

//   if (fd == -1) {
//     return -3;
//   }

//   ret = procclient.sendFD(fd);

//   if (ret != 0) {
//     printf("[%s:%d] <%s>(%d): main send failed\n", __FILE__, __LINE__,
//            __FUNCTION__, getpid());
//     return ret;
//   }

//   ret = write(fd, "galen\n", 6);
//   if (ret == -1) {
//     printf("[%s:%d] <%s>(%d): main write failed, (%d)%s\n", __FILE__,
//     __LINE__,
//            __FUNCTION__, getpid(), errno, strerror(errno));
//   }
//   printf("[%s:%d] <%s>(%d): main write success\n", __FILE__, __LINE__,
//          __FUNCTION__, getpid());

//   close(fd);
//   sleep(10);
//   proclog.sendFD(-1);
//   printf("main process send fd !\n");
//   return 0;
// }
