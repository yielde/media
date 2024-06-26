#include <fcntl.h>
#include <memory.h>
#include <netinet/in.h>
#include <signal.h>
// #include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <functional>
#include <iostream>

#include "epoll.h"
#include "httpparser.h"
#include "logger.h"
#include "mysqlclient.h"
#include "process.h"
#include "server.h"
#include "sqlite3client.h"
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

int test_thread() {
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

int test_business_server() {
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

int test_http_parser() {
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

DECLARE_TABLE_CLASS(user_test, _sqlite3_table_)
DECLARE_FIELD(TYPE_INT, user_id, NOT_NULL | PRIMARY_KEY | AUTOINCREMENT,
              "INTEGER", "", "", "")
DECLARE_FIELD(TYPE_VARCHAR, user_qq, 0, "VARCHAR", "(15)", "", "")
DECLARE_FIELD(TYPE_VARCHAR, user_phone, NOT_NULL | DEFAULT, "VARCHAR", "(11)",
              "18888888888", "")
DECLARE_FIELD(TYPE_TEXT, user_name, 0, "TEXT", "", "", "")
DECLARE_TABLE_CLASS_END()

int sqlite3_test() {
  // 使用
  // user_test test;
  // printf("create: %s\n", (char *)test.Create());
  // user_test values;
  // values.Fields["user_qq"]->LoadFromStr("937013596");
  // values.Fields["user_qq"]->Condition = SQL_INSERT;
  // printf("insert: %s\n", (char *)test.Insert(values));
  // values.Fields["user_qq"]->LoadFromStr("93701359666");
  // values.Fields["user_qq"]->Condition = SQL_MODIFY;
  // values.Fields["user_id"]->LoadFromStr("0");
  // values.Fields["user_id"]->Condition = SQL_CONDITION;
  // values.Fields["user_phone"]->LoadFromStr("18888888888");
  // values.Fields["user_phone"]->Condition = SQL_CONDITION;
  // printf("modify: %s\n", (char *)test.Modify(values));
  // printf("query: %s\n", (char *)test.Query());
  // test.Fields["user_phone"]->LoadFromStr("18888888888");
  // test.Fields["user_phone"]->Condition = SQL_CONDITION;
  // test.Fields["user_id"]->LoadFromStr("0");
  // test.Fields["user_id"]->Condition = SQL_CONDITION;
  // printf("delete: %s\n", (char *)test.Delete(test))
  // printf("drop: %s\n", (char *)test.Drop());

  getchar();
  printf("next~~~\n\n");
  CDatabaseClient *pClient = new CSqlite3Client();
  KeyValue args;
  user_test test;
  user_test value;
  int ret = -1;
  args["host"] = "test.db";
  ret = pClient->Connect(args);
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
  ret = pClient->Exec(test.Create());
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  value.Fields["user_qq"]->LoadFromStr("937013596");
  value.Fields["user_qq"]->Condition = SQL_INSERT;
  ret = pClient->Exec(test.Insert(value));
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  value.Fields["user_qq"]->LoadFromStr("93701359666");
  value.Fields["user_qq"]->Condition = SQL_MODIFY;

  ret = pClient->Exec(test.Modify(value));
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  Result result;
  ret = pClient->Exec(test.Query(), result, test);
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  ret = pClient->Exec(test.Delete(value));
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  ret = pClient->Exec(test.Drop());
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  ret = pClient->Close();
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  return 0;
}

MYSQL_DECLARE_TABLE_CLASS(muser_test, _mysql_table_)
MYSQL_DECLARE_FIELD(TYPE_INT, user_id, NOT_NULL | PRIMARY_KEY | AUTOINCREMENT,
                    "INTEGER", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_VARCHAR, user_qq, 0, "VARCHAR", "(15)", "", "")
MYSQL_DECLARE_FIELD(TYPE_VARCHAR, user_phone, NOT_NULL | DEFAULT, "VARCHAR",
                    "(11)", "18888888888", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_name, 0, "TEXT", "", "小孩哥", "")
MYSQL_DECLARE_TABLE_CLASS_END()

int mysql_test() {  // 使用
  // muser_test test;
  // printf("create: %s\n", (char *)test.Create());
  // user_test values;
  // values.Fields["user_qq"]->LoadFromStr("937013596");
  // values.Fields["user_qq"]->Condition = SQL_INSERT;
  // printf("insert: %s\n", (char *)test.Insert(values));
  // values.Fields["user_qq"]->LoadFromStr("93701359666");
  // values.Fields["user_qq"]->Condition = SQL_MODIFY;
  // values.Fields["user_id"]->LoadFromStr("0");
  // values.Fields["user_id"]->Condition = SQL_CONDITION;
  // values.Fields["user_phone"]->LoadFromStr("18888888888");
  // values.Fields["user_phone"]->Condition = SQL_CONDITION;
  // printf("modify: %s\n", (char *)test.Modify(values));
  // printf("query: %s\n", (char *)test.Query());
  // test.Fields["user_phone"]->LoadFromStr("18888888888");
  // test.Fields["user_phone"]->Condition = SQL_CONDITION;
  // test.Fields["user_id"]->LoadFromStr("0");
  // test.Fields["user_id"]->Condition = SQL_CONDITION;
  // printf("delete: %s\n", (char *)test.Delete(test));
  // printf("drop: %s\n", (char *)test.Drop());

  getchar();
  printf("next~~~\n\n");
  CDatabaseClient *pClient = new CMysqlClient();
  KeyValue args;
  muser_test test;
  muser_test value;
  int ret = -1;
  args["host"] = "localhost";
  args["password"] = "th303th303";
  args["port"] = 3306;
  args["user"] = "root";
  args["db"] = "video";
  ret = pClient->Connect(args);
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
  ret = pClient->Exec(test.Create());
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  value.Fields["user_qq"]->LoadFromStr("937013596");
  value.Fields["user_qq"]->Condition = SQL_INSERT;
  ret = pClient->Exec(test.Insert(value));
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  ret = pClient->Exec(test.Insert(value));
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  value.Fields["user_qq"]->LoadFromStr("93701359666");
  value.Fields["user_qq"]->Condition = SQL_MODIFY;

  ret = pClient->Exec(test.Modify(value));
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  Result result;
  ret = pClient->Exec(test.Query(), result, test);
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
  value.ClearConditionUsed();
  value.Fields["user_id"]->LoadFromStr("1");
  value.Fields["user_id"]->Condition = SQL_CONDITION;

  ret = pClient->Exec(test.Delete(value));
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  // ret = pClient->Exec(test.Drop());
  // printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  ret = pClient->Close();
  printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

  return 0;
}

int main() {
  CProcess proclog;
  proclog.setEntryFunction(createLogServer, &proclog);
  proclog.CreateSubProcess();
  sleep(1);
  mysql_test();
  getchar();
  return 0;
}