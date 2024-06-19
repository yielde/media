#ifndef _VIDEOPLAYERSERVER_H_
#define _VIDEOPLAYERSERVER_H_

#include <map>

#include "epoll.h"
#include "logger.h"
#include "server.h"

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
    int ret = -1;
    setConnectedCallback(&CVideoPlayerServer::connected, this, _1);
    ERR_RETURN(ret, -1);
    setRecvCallback(&CVideoPlayerServer::received, this, _1, _2);
    ERR_RETURN(ret, -2);
    ret = m_epoll.Create(m_count);
    ERR_RETURN(ret, -3);
    ret = m_pool.Start(m_count);
    ERR_RETURN(ret, -4);
    for (unsigned i = 0; i < m_count; ++i) {
      ret = m_pool.AddTask(&CVideoPlayerServer::ThreadFunc, this);
    }
    while (m_epoll != -1) {
      int sock = 0;
      sockaddr_in addrin;
      ret = proc->recvIPSocket(sock, &addrin);
      if ((ret < 0) || (sock == 0)) break;
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

  int connected(CSocketBase *pClient) { return 0; }
  int received(CSocketBase *pClient, const Buffer &data) { return 0; }

 private:
  CEpoll m_epoll;
  CThreadPool m_pool;
  std::map<int, CSocketBase *> m_mapClients;
  unsigned m_count;
};
#endif