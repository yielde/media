#pragma once
#include <errno.h>
#include <memory.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <vector>

#define MAX_EVENTS 1024
using EPEvents = std::vector<epoll_event>;

class EpollData {
 public:
  EpollData() { m_data.u64 = 0; }
  explicit EpollData(uint64_t u64) { m_data.u64 = u64; }
  explicit EpollData(uint32_t u32) { m_data.u32 = u32; }
  explicit EpollData(int fd) { m_data.fd = fd; }
  EpollData(void *ptr) { m_data.ptr = ptr; }
  EpollData(const EpollData &data) { m_data.u64 = data.m_data.u64; }

 public:
  EpollData &operator=(const EpollData &data) {
    if (&data != this) {
      m_data.u64 = data.m_data.u64;
    }
    return *this;
  }

  EpollData &operator=(void *data) {
    m_data.ptr = data;
    return *this;
  }

  EpollData &operator=(uint32_t u32) {
    m_data.u32 = u32;
    return *this;
  }

  EpollData &operator=(uint64_t u64) {
    m_data.u64 = u64;
    return *this;
  }

  EpollData &operator=(int fd) {
    m_data.fd = fd;
    return *this;
  }

  operator epoll_data_t() { return m_data; }
  operator epoll_data_t() const { return m_data; }
  operator epoll_data_t *() { return &m_data; }
  operator const epoll_data_t *() const { return &m_data; }

 private:
  epoll_data_t m_data;
};

class CEpoll {
 public:
  CEpoll() { m_epoll = -1; }
  ~CEpoll() { Close(); }
  CEpoll(const CEpoll &) = delete;
  CEpoll &operator=(const CEpoll &) = delete;
  operator int() const { return m_epoll; }

 public:
  int Create(unsigned count) {
    if (m_epoll != -1) return -1;
    m_epoll = epoll_create(count);
    if (m_epoll == -1) return -2;
    return 0;
  }

  ssize_t WaitEvent(EPEvents &events, int timeout = 10) {
    if (m_epoll == -1) return -1;
    EPEvents evs(MAX_EVENTS);
    int ret = epoll_wait(m_epoll, evs.data(), MAX_EVENTS, timeout);
    if (ret == -1) {
      if ((errno == EINTR) || (errno == EAGAIN)) {
        return 0;
      }
      return -2;
    }
    if (ret > (int)events.size()) {
      events.resize(ret);
    }
    memcpy(events.data(), evs.data(), sizeof(epoll_event) * ret);
    return ret;
  }

  int Add(int fd, const EpollData &data = EpollData((void *)0),
          uint32_t events = EPOLLIN) {
    epoll_event evs;
    evs.data = data;
    evs.events = events;
    int ret = epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, &evs);
    if (ret == -1) return -2;
    return 0;
  }

  int Modify(int fd, uint32_t events,
             const EpollData &data = EpollData((void *)0)) {
    epoll_event evs;
    evs = {events, data};
    int ret = epoll_ctl(m_epoll, EPOLL_CTL_MOD, fd, &evs);
    if (ret == -1) return -2;
    return 0;
  }

  int Del(int fd) {
    int ret = epoll_ctl(m_epoll, EPOLL_CTL_DEL, fd, NULL);
    if (ret == -1) return -2;
    return 0;
  }

  void Close() {
    if (m_epoll != -1) {
      int fd = m_epoll;
      m_epoll = -1;
      close(fd);
    }
  }

 private:
  int m_epoll;
};