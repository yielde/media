#ifndef _VIDEOPLAYERSERVER_H_
#define _VIDEOPLAYERSERVER_H_

#include <map>

#include "crypto.h"
#include "epoll.h"
#include "httpparser.h"
#include "json.h"
#include "logger.h"
#include "mysqlclient.h"
#include "server.h"

MYSQL_DECLARE_TABLE_CLASS(video_user_mysql, _mysql_table_)
MYSQL_DECLARE_FIELD(TYPE_INT, user_id, NOT_NULL | PRIMARY_KEY | AUTOINCREMENT,
                    "INTEGER", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_VARCHAR, user_qq, NOT_NULL, "VARCHAR", "(15)", "", "")
MYSQL_DECLARE_FIELD(TYPE_VARCHAR, user_phone, DEFAULT, "VARCHAR", "(11)",
                    "'18888888888'", "")  // �ֻ�
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_name, NOT_NULL, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_nick, NOT_NULL, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_wechat, DEFAULT, "TEXT", "", "NULL", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_wechat_id, DEFAULT, "TEXT", "", "NULL", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_address, DEFAULT, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_province, DEFAULT, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_country, DEFAULT, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_age, DEFAULT | CHECK, "INTEGER", "", "18",
                    "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_male, DEFAULT, "BOOL", "", "1", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_flags, DEFAULT, "TEXT", "", "0", "")
MYSQL_DECLARE_FIELD(TYPE_REAL, user_experience, DEFAULT, "REAL", "", "0.0", "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_level, DEFAULT | CHECK, "INTEGER", "", "0",
                    "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_class_priority, DEFAULT, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_REAL, user_time_per_viewer, DEFAULT, "REAL", "", "",
                    "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_career, NONE, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_password, NOT_NULL, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_birthday, NONE, "DATETIME", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_describe, NONE, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_education, NONE, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_register_time, DEFAULT, "DATETIME", "",
                    "LOCALTIME()", "")
MYSQL_DECLARE_TABLE_CLASS_END()

#define ERR_RETURN(ret, errn) \
  if (ret < 0) {              \
    TRACEE("ret: %d", ret);   \
    return errn;              \
  }

#define WARN_CONTINUE(ret)  \
  if (ret < 0) {            \
    TRACEW("ret: %d", ret); \
    continue;               \
  }

class CVideoPlayerServer : public CBusiness {
 public:
  CVideoPlayerServer(unsigned count) : CBusiness() { m_count = count; }
  ~CVideoPlayerServer() {
    if (m_db) {
      CDatabaseClient *db = m_db;
      m_db = NULL;
      db->Close();
      delete db;
    }
    m_epoll.Close();
    m_pool.Close();
    for (auto it : m_mapClients) {
      if (it.second) {
        delete it.second;
      }
    }
    m_mapClients.clear();
  }

  virtual int BusinessProcess(CProcess *proc) {
    using namespace std::placeholders;
    int ret = 0;
    m_db = new CMysqlClient();
    if (m_db == NULL) {
      TRACEE("can't allocate instantiate mysql");
      return -1;
    }
    KeyValue args;
    args["host"] = "127.0.0.1";
    args["user"] = "root";
    args["password"] = "th303th303";
    args["port"] = 3306;
    args["db"] = "video";

    ret = m_db->Connect(args);
    ERR_RETURN(ret, -2);
    video_user_mysql user;
    ret = m_db->Exec(user.Create());
    ERR_RETURN(ret, -3);
    setConnectedCallback(&CVideoPlayerServer::connected, this, _1);
    ERR_RETURN(ret, -4);
    setRecvCallback(&CVideoPlayerServer::received, this, _1, _2);
    ERR_RETURN(ret, -5);
    ret = m_epoll.Create(m_count);
    ERR_RETURN(ret, -6);
    ret = m_pool.Start(m_count);
    ERR_RETURN(ret, -7);
    for (unsigned i = 0; i < m_count; ++i) {
      ret = m_pool.AddTask(&CVideoPlayerServer::ThreadFunc, this);
      ERR_RETURN(ret, -8);
    }

    while (m_epoll != -1) {
      int sock = 0;
      sockaddr_in addrin;
      ret = proc->recvIPSocket(sock, &addrin);
      if ((ret < 0) || (sock == 0)) {
        break;
      }
      CSocketBase *pClient = new CLocalSocket(sock);
      if (pClient == NULL) continue;
      ret = pClient->Init(CSocketParam(&addrin, SOCK_ISIP));

      WARN_CONTINUE(ret);
      int ret = m_epoll.Add(sock, EpollData((void *)pClient));
      if (m_connectedcallback) {
        (*m_connectedcallback)(pClient);
      }
      WARN_CONTINUE(ret);
    }
    return 0;
  }

 private:
  int ThreadFunc() {
    int ret = 0;
    while (m_epoll != -1) {
      EPEvents evs;
      ssize_t size = m_epoll.WaitEvent(evs);
      if (size < 0) break;
      if (size > 0) {
        for (ssize_t i = 0; i < size; ++i) {
          if (evs[i].events & EPOLLERR) {
            break;
          } else if (evs[i].events & EPOLLIN) {
            CSocketBase *pClient = (CSocketBase *)evs[i].data.ptr;
            if (pClient) {
              Buffer data;
              ret = pClient->Recv(data);
              if (ret <= 0) {
                TRACEW("ret=%d errno=%d msg=[%s]", ret, errno, strerror(errno));
                m_epoll.Del(*pClient);
                continue;
              }
              WARN_CONTINUE(ret);
              if (m_recvcallback) {
                (*m_recvcallback)(pClient, data);
              }
            }
          }
        }
      }
    }
    return 0;
  }

  int connected(CSocketBase *pClient) {
    sockaddr_in *paddr = *pClient;
    TRACEI("address: %s, port: %d", inet_ntoa(paddr->sin_addr),
           paddr->sin_port);
    return 0;
  }

  int received(CSocketBase *pClient, const Buffer &data) {
    int ret = 0;
    Buffer response = "";
    ret = HttpParser(data);
    if (ret != 0) {
      TRACEE("http parser failed, %d\n", ret);
    }
    response = MakeResponse(ret);
    ret = pClient->Send(response);
    if (ret != 0)
      TRACEE("%d, http response failed, %s", ret, response);
    else
      TRACEI("%d, http response success", ret);
    return 0;
  }

  Buffer MakeResponse(int ret) {
    Json::Value root;
    root["status"] = ret;
    if (ret != 0) {
      root["message"] = "用户名密码错误";
    } else {
      root["message"] = "成功";
    }
    Buffer json = root.toStyledString();
    Buffer result = "HTTP/1.1 200 OK\r\n";
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tm *pTm = localtime(&tv.tv_sec);
    char temp[64] = "";
    strftime(temp, sizeof(temp), "%a, %d %b %G %T GMT\r\n", pTm);
    Buffer Date = Buffer("Date: ") + temp;
    Buffer Server =
        "Server: Gvideo/1.0\r\nContent-Type: text/html; "
        "charset=utf-8\r\nX-Frame-Options: DENY\r\n";
    snprintf(temp, sizeof(temp), "%ld", json.size());
    Buffer Length = Buffer("Content-Length: ") + temp + "\r\n";
    Buffer Stub =
        "X-Content-Type-Options: nosniff\r\nReferrer-Policy: "
        "same-origin\r\n\r\n";
    result += Date + Server + Length + Stub + json;
    TRACEI("response: %s", (char *)result);
    return result;
  }

  int HttpParser(const Buffer &data) {
    CHttpParser parser;
    size_t size = parser.Parser(data);
    if (size == 0 || parser.Errno() != 0) {
      TRACEE("HttpParser: %ld %d", size, parser.Errno());
      return -1;
    }
    if (parser.Method() == HTTP_GET) {
      UrlParser url("http://10.211.55.3" + parser.Url());
      int ret = url.Parser();
      if (ret != 0) {
        TRACEE("ret = %d url[%s]", ret, "http://10.211.55.3" + parser.Url());
        return -2;
      }
      Buffer uri = url.Uri();
      TRACEI("**** uri = %s", (char *)uri);
      if (uri == "login") {
        Buffer time = url["time"];
        Buffer salt = url["salt"];
        Buffer user = url["user"];
        Buffer sign = url["sign"];
        printf("time %s salt %s user %s sign %s\n", (char *)time, (char *)salt,
               (char *)user, (char *)sign);
        video_user_mysql dbuser;
        Result result;
        Buffer sql = dbuser.Query("user_name=\"" + user + "\"");
        ret = m_db->Exec(sql, result, dbuser);
        if (ret != 0) {
          TRACEE("ret=%d, sql:{%s}", ret, sql);
          return -3;
        }
        if (result.size() == 0) {
          TRACEE("ret=%d, no result sql:{%s}", ret, sql);
          return -4;
        }
        if (result.size() != 1) {
          TRACEE("ret=%d, repeat user sql{%s}", ret, sql);
          return -5;
        }
        auto user1 = result.front();
        Buffer pwd = *user1->Fields["user_password"]->Value.String;
        TRACEI("password=%s", pwd);
        const char *MD5_KEY = "abcdefghijklmnopqrstuvwxyz";
        Buffer md5str = time + MD5_KEY + pwd + salt;
        Buffer md5 = Crypto::MD5(md5str);
        TRACEI("md5=%s", (char *)md5);
        printf("md5=%s\n", (char *)md5);
        if (md5 == sign) {
          return 0;
        }
        return -6;
      }
    } else if (parser.Method() == HTTP_POST) {
    }
    return 0;
  }

 private:
  CEpoll m_epoll;
  CThreadPool m_pool;
  std::map<int, CSocketBase *> m_mapClients;
  unsigned m_count;
  CDatabaseClient *m_db;
};
#endif