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
#include "httpparser.h"
#include "logger.h"
#include "process.h"
#include "server.h"
#include "threadpool.h"
#include "videoplayerserver.h"

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

int createLogServer(CProcess *proc) {
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

int createClientServer(CProcess *proc) {
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

int old_main1() {
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

int old_main2() {
  int ret = 0;
  CProcess proclog;
  ret = proclog.setEntryFunction(createLogServer, &proclog);
  ERR_RETURN(ret, -1);
  ret = proclog.CreateSubProcess();
  ERR_RETURN(ret, -2);
  sleep(3);
  LOGI << "Start server";
  CVideoPlayerServer business(5);
  CServer server;
  ret = server.Init(&business);
  ERR_RETURN(ret, -3);
  ret = server.Run();
  ERR_RETURN(ret, -4);

  (void)getchar();
  LOGI << "kill process";
  proclog.sendFD(-1);
  return 0;
}

int main() {
  int ret = 0;
  // test httpparser
  CHttpParser parser;
  printf("request header ---------------------------------\n");
  Buffer str =
      "GET /favicon.ico HTTP/1.1\r\n"
      "Host: 0.0.0.0=5000\r\n"
      "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) "
      "Gecko/2008061015 Firefox/3.0\r\n"
      "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*; q = "
      "0.8\r\n"
      "Accept-Language: en-us,en;q=0.5\r\n"
      "Accept-Encoding: gzip,deflate\r\n"
      "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
      "Keep-Alive: 300\r\n"
      "Connection: keep-alive\r\n"
      "\r\n";

  size_t size = parser.Parser(str);
  if (parser.Errno() != 0) {
    printf("errno: %d\n", parser.Errno());
    return -1;
  }
  if (size != 368) {
    printf("size error: %ld\n", size);
    return -2;
  }
  printf("method %d url %s\n", parser.Method(), (char *)parser.Url());

  str =
      "GET /favicon.ico HTTP/1.1\r\n"
      "Host: 0.0.0.0=5000\r\n"
      "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) "
      "Gecko/2008061015 Firefox/3.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";

  size = parser.Parser(str);
  printf("errno %d size %ld\n", parser.Errno(), size);

  if (parser.Errno() != 0x7F) {
    return -3;
  }
  if (size != 0) {
    return -4;
  }

  // test url parser
  printf("url1 ---------------------------------\n");

  UrlParser url1(
      "https://www.baidu.com/s?ie=utf8&oe=utf8&wd=httplib&tn=98010089_dg&ch=3");

  ret = url1.Parser();
  if (ret != 0) {
    printf("urlparser url1 error: %d\n", ret);
    return -5;
  }
  // printf("port: %d, host: %s\n", url1.Port(), (char *)url1.Host());

  printf("ie = %s \n", (char *)url1["ie"]);
  printf("oe = %s\n", (char *)url1["oe"]);
  printf("wd = %s \n", (char *)url1["wd"]);
  printf("tn = %s \n", (char *)url1["tn"]);
  printf("ch = %s \n", (char *)url1["ch"]);

  printf("url2 ---------------------------------\n");

  UrlParser url2(
      "http://127.0.0.1:19811/"
      "?time=144000&salt=9527&user=test&sign=1234567890abcdef");
  ret = url2.Parser();
  if (ret != 0) {
    printf("urlparser url2 error: %d\n", ret);
    return -6;
  }

  printf("port: %d, host: %s\n", url2.Port(), (char *)url2.Host());

  printf("time = %s \n", (char *)url2["time"]);
  printf("salt = %s\n", (char *)url2["salt"]);
  printf("user = %s \n", (char *)url2["user"]);
  printf("sign = %s \n", (char *)url2["sign"]);

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
