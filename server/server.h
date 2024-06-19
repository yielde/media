#ifndef _SERVER_H_
#define _SERVER_H_
#include "epoll.h"
#include "logger.h"
#include "process.h"
#include "socket.h"
#include "threadpool.h"

template <typename _FUNCTION_, typename... _ARGS_>
class CConnectedFunction : public CFunctionBase {
 public:
  CConnectedFunction(_FUNCTION_ func, _ARGS_... args)
      : m_binder(std::forward<_FUNCTION_>(func),
                 std::forward<_ARGS_>(args)...) {}
  virtual ~CConnectedFunction() {}
  virtual int operator()(CSocketBase* pClient) { return m_binder(pClient); }

  typename std::_Bindres_helper<int, _FUNCTION_, _ARGS_...>::type m_binder;
};

template <typename _FUNCTION_, typename... _ARGS_>
class CReceivedFunction : public CFunctionBase {
 public:
  CReceivedFunction(_FUNCTION_ func, _ARGS_... args)
      : m_binder(std::forward<_FUNCTION_>(func),
                 std::forward<_ARGS_>(args)...) {}
  virtual ~CReceivedFunction() {}
  virtual int operator()(CSocketBase* pClient, const Buffer& data) {
    return m_binder(pClient, data);
  }

  typename std::_Bindres_helper<int, _FUNCTION_, _ARGS_...>::type m_binder;
};

class CBusiness {
 public:
  CBusiness() : m_connectedcallback(NULL), m_recvcallback(NULL) {}
  virtual int BusinessProcess(CProcess*) = 0;

  template <typename _FUNCTION_, typename... _ARGS_>
  int setConnectedCallback(_FUNCTION_ func, _ARGS_... args) {
    m_connectedcallback =
        new CConnectedFunction<_FUNCTION_, _ARGS_...>(func, args...);
    if (m_connectedcallback == NULL) return -1;
    return 0;
  }

  template <typename _FUNCTION_, typename... _ARGS_>
  int setRecvCallback(_FUNCTION_ func, _ARGS_... args) {
    m_recvcallback =
        new CReceivedFunction<_FUNCTION_, _ARGS_...>(func, args...);
    if (m_recvcallback == NULL) return -1;
    return 0;
  }

 protected:
  CFunctionBase* m_connectedcallback;
  CFunctionBase* m_recvcallback;
};

class CServer {
 public:
  CServer() {
    m_server = NULL;
    m_business = NULL;
  }

  ~CServer() { Close(); }
  CServer(const CServer&) = delete;
  CServer& operator=(const CServer&) = delete;

  int Init(CBusiness* business, const Buffer& ip = "127.0.0.1",
           short port = 9999) {
    m_business = business;
    if (m_business == NULL) return -1;
    int ret = 0;
    ret = m_process.setEntryFunction(&CBusiness::BusinessProcess, m_business,
                                     &m_process);
    if (ret != 0) return -2;
    ret = m_process.CreateSubProcess();
    if (ret != 0) return -3;
    ret = m_pool.Start(5);
    if (ret != 0) return -4;
    ret = m_epoll.Create(5);
    if (ret != 0) return -5;
    m_server = new CLocalSocket();
    if (m_server == NULL) return -6;
    ret = m_server->Init(CSocketParam(ip, port, SOCK_ISSERVER | SOCK_ISIP));
    if (ret != 0) return -7;
    m_epoll.Add(*m_server, EpollData((void*)m_server));
    if (ret != 0) return -8;
    for (size_t i = 0; i < m_pool.Size(); ++i) {
      ret = m_pool.AddTask(&CServer::ThreadFunc, this);
      if (ret != 0) return -9;
    }

    return 0;
  }

  int Run() {
    while (m_server != NULL) {
      usleep(10);
    }
    return 0;
  }

  int Close() {
    if (m_server) {
      CSocketBase* sock = m_server;
      m_server = NULL;
      m_epoll.Del(*m_server);
      delete sock;
    }
    m_epoll.Close();
    m_process.sendFD(-1);
    m_pool.Close();
    return 0;
  }

 private:
  int ThreadFunc() {
    int ret = 0;
    EPEvents evs;
    while ((m_epoll != -1) && (m_server != NULL)) {
      ssize_t size = m_epoll.WaitEvent(evs);
      if (size < 0) break;
      if (size > 0) {
        for (ssize_t i = 0; i < size; i++) {
          if (evs[i].events & EPOLLERR) {
            break;
          } else if (evs[i].events & EPOLLIN) {
            if (m_server) {
              CSocketBase* pClient = NULL;
              ret = m_server->Link(&pClient);
              if (ret != 0) continue;
              ret = m_process.sendIPSocket(*pClient, *pClient);
              delete pClient;
              if (ret != 0) {
                TRACEE("send client %d failed!", (int)*pClient);
                continue;
              }
            }
          }
        }
      }
    }
    return 0;
  }

 private:
  CThreadPool m_pool;
  CSocketBase* m_server;
  CEpoll m_epoll;
  CProcess m_process;
  CBusiness* m_business;
};

#endif