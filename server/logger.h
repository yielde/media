#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include <iostream>
#include <map>
#include <sstream>

#include "epoll.h"
#include "socket.h"
#include "thread.h"

enum LogLevel { LOG_INFO, LOG_DEBUG, LOG_WARNING, LOG_ERROR, LOG_FATAL };

class LogInfo {
 public:
  LogInfo(const char *file, int line, const char *func, pid_t pid,
          pthread_t tid, int level, const char *fmt, ...);

  LogInfo(const char *file, int line, const char *func, pid_t pid,
          pthread_t tid, int level);

  LogInfo(const char *file, int line, const char *func, pid_t pid,
          pthread_t tid, int level, void *pData, size_t nSize);
  ~LogInfo();

 public:
  operator Buffer() const { return m_buf; }
  template <typename T>
  LogInfo &operator<<(const T &data) {
    std::stringstream stream;
    stream << data;
    m_buf += stream.str();
    return *this;
  }

 private:
  bool isAuto;
  Buffer m_buf;
};

class CLoggerServer {
 public:
  CLoggerServer() : m_thread(&CLoggerServer::ThreadFunc, this) {
    m_server = NULL;
    m_path = "./log/" + GetTimeStr() + ".log";
  }
  ~CLoggerServer() { Close(); }

  CLoggerServer(const CLoggerServer &) = delete;
  CLoggerServer &operator=(const CLoggerServer &) = delete;

 public:
  int Start() {
    if (m_server != NULL) return -1;
    if (access("log", W_OK | R_OK) != 0) {
      mkdir("log", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }
    m_file = fopen(m_path, "w+");
    if (m_file == NULL) return -2;
    int ret = m_epoll.Create(1);
    if (ret != 0) return -3;

    m_server = new CLocalSocket();
    if (m_server == NULL) {
      Close();
      return -4;
    }

    ret = m_server->Init(
        CSocketParam("./log/server.sock", (int)SOCK_ISSERVER | SOCK_ISREUSE));
    if (ret != 0) {
      Close();
      return -5;
    }
    ret =
        m_epoll.Add(*m_server, EpollData((void *)m_server), EPOLLIN | EPOLLERR);
    if (ret != 0) {
      Close();
      return -6;
    }
    ret = m_thread.Start();

    if (ret != 0) {
      Close();
      return -7;
    }
    return 0;
  }

  int Close() {
    if (m_server != NULL) {
      CSocketBase *p = m_server;
      m_server = NULL;
      delete p;
    }
    m_epoll.Close();
    m_thread.Stop();
    return 0;
  }

  static void Trace(const LogInfo &info) {
    static thread_local CLocalSocket client;
    int ret = 0;
    if (client == -1) {
      ret = client.Init(CSocketParam("./log/server.sock", (int)0));
      if (ret != 0) {
        return;
      }
      ret = client.Link();
      if (ret != 0) {
        return;
      }
    }
    client.Send(info);
  }

  // static Buffer GetTimeStr() {
  //   Buffer result(128);
  //   timeb tmb;
  //   ftime(&tmb);
  //   tm *pTm = localtime(&tmb.time);
  //   int nSize =
  //       snprintf(result, result.size(), "%04d-%02d-%02d_%02d-%02d-%02d.%03d",
  //                pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
  //                pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tmb.millitm);
  //   result.resize(nSize);
  //   return result;
  // }
  static Buffer GetTimeStr() {
    Buffer result(128);
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tm *pTm = localtime(&tv.tv_sec);
    int nSize =
        snprintf(result, result.size(), "%04d-%02d-%02d_%02d-%02d-%02d.%03ld",
                 pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
                 pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tv.tv_usec);
    result.resize(nSize);
    return result;
  }

 private:
  int ThreadFunc() {
    EPEvents events;
    std::map<int, CSocketBase *> mapClients;
    while (m_thread.isValid() && (m_epoll != -1) && (m_server != NULL)) {
      ssize_t ret = m_epoll.WaitEvent(events, 1);

      if (ret < 0) break;
      if (ret > 0) {
        ssize_t i = 0;
        for (; i < ret; ++i) {
          if (events[i].events & EPOLLERR) {
            break;
          } else if (events[i].events & EPOLLIN) {
            if (events[i].data.ptr == m_server) {
              CSocketBase *pClient = NULL;
              int r = m_server->Link(&pClient);
              if (r < 0) continue;
              r = m_epoll.Add(*pClient, EpollData((void *)pClient),
                              EPOLLIN | EPOLLERR);
              if (r < 0) {
                delete pClient;
                continue;
              }
              auto it = mapClients.find(*pClient);
              if (it != mapClients.end()) {
                if (it->second) delete it->second;
              }
              mapClients[*pClient] = pClient;
            } else {
              CSocketBase *pClient = (CSocketBase *)events[i].data.ptr;

              if (pClient != NULL) {
                Buffer data(1024 * 1024);
                int r = pClient->Recv(data);
                if (r <= 0) {
                  mapClients[*pClient] = NULL;
                  delete pClient;
                } else {
                  WriteLog(data);
                }
              } else {
                printf("pClient is NULL !\n");
              }
            }
          }
        }

        if (i != ret) {
          break;
        }
      }
    }
    for (auto it = mapClients.begin(); it != mapClients.end(); ++it) {
      if (it->second) {
        delete it->second;
      }
    }
    mapClients.clear();
    return 0;
  }

  void WriteLog(const Buffer &data) {
    if (m_file != NULL) {
      FILE *pFile = m_file;
      fwrite((char *)data, 1, data.size(), pFile);
      fflush(pFile);
    }
  }

 private:
  CThread m_thread;

  CEpoll m_epoll;
  CSocketBase *m_server;
  Buffer m_path;
  FILE *m_file;
};

#ifndef TRACE
// 内存
#define DUMPI(data, size)                                                     \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(),    \
                               gettid(), LOG_INFO, static_cast<void *>(data), \
                               size))
#define DUMPD(data, size)                                                      \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(),     \
                               gettid(), LOG_DEBUG, static_cast<void *>(data), \
                               size))
#define DUMPW(data, size)                                                  \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), \
                               gettid(), LOG_WARNING,                      \
                               static_cast<void *>(data), size))
#define DUMPE(data, size)                                                      \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(),     \
                               gettid(), LOG_ERROR, static_cast<void *>(data), \
                               size))
#define DUMPF(data, size)                                                      \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(),     \
                               gettid(), LOG_FATAL, static_cast<void *>(data), \
                               size))

// 直接记录
#define LOGI \
  LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), LOG_INFO)
#define LOGD \
  LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), LOG_DEBUG)
#define LOGW \
  LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), LOG_WARNING)
#define LOGE \
  LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), LOG_ERROR)
#define LOGF \
  LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), LOG_FATAL)

// 格式化输出
#define TRACEI(...)                                                        \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), \
                               gettid(), LOG_INFO, __VA_ARGS__))
#define TRACED(...)                                                        \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), \
                               gettid(), LOG_DEBUG, __VA_ARGS__))
#define TRACEW(...)                                                        \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), \
                               gettid(), LOG_WARNING, __VA_ARGS__))
#define TRACEE(...)                                                        \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), \
                               gettid(), LOG_ERROR, __VA_ARGS__))
#define TRACEF(...)                                                        \
  CLoggerServer::Trace(LogInfo(__FILE__, __LINE__, __FUNCTION__, getpid(), \
                               gettid(), LOG_FATAL, __VA_ARGS__))

#endif
#endif