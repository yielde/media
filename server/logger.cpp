#include "logger.h"

LogInfo::LogInfo(const char *file, int line, const char *func, pid_t pid,
                 pthread_t tid, int level) {
  isAuto = true;
  char *buf = NULL;
  const char sLevel[][8] = {"INFO", "DEBUG", "WARNING", "ERROR", "FATAL"};
  // 日志头
  int count =
      asprintf(&buf, "%s(%d):[%s][%s]<%d-%ld>(%s) ", file, line, sLevel[level],
               (char *)CLoggerServer::GetTimeStr(), pid, tid, func);
  if (count > 0) {
    m_buf = buf;
    free(buf);
  }
}

LogInfo::LogInfo(const char *file, int line, const char *func, pid_t pid,
                 pthread_t tid, int level, const char *fmt, ...) {
  isAuto = false;
  char *buf = NULL;
  const char sLevel[][8] = {"INFO", "DEBUG", "WARNING", "ERROR", "FATAL"};
  // 日志头
  int count =
      asprintf(&buf, "%s(%d):[%s][%s]<%d-%ld>(%s) ", file, line, sLevel[level],
               (char *)CLoggerServer::GetTimeStr(), pid, tid, func);

  if (count > 0) {
    m_buf = buf;
    free(buf);
  } else {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  count = vasprintf(&buf, fmt, ap);
  if (count > 0) {
    m_buf += buf;
    free(buf);
  }
  m_buf += "\n";
  va_end(ap);
}

LogInfo::LogInfo(const char *file, int line, const char *func, pid_t pid,
                 pthread_t tid, int level, void *pData, size_t nSize) {
  isAuto = false;
  char *buf = NULL;
  const char sLevel[][8] = {"INFO", "DEBUG", "WARNING", "ERROR", "FATAL"};
  // 日志头
  int count =
      asprintf(&buf, "%s(%d):[%s][%s]<%d-%ld>(%s)\n", file, line, sLevel[level],
               (char *)CLoggerServer::GetTimeStr(), pid, tid, func);
  if (count > 0) {
    m_buf = buf;
    free(buf);
  } else
    return;

  size_t i = 0;
  Buffer out;
  char *Data = (char *)pData;

  for (; i < nSize; ++i) {
    char buf[16] = "";
    snprintf(buf, sizeof(buf), "%2X ", Data[i]);
    m_buf += buf;
    if (((i + 1) % 16) == 0) {
      m_buf += "\t; ";
      for (int j = i - 15; j < i; ++j) {
        if (((Data[j] & 0xFF) > 31) && ((Data[j] & 0xFF) < 0x7F)) {
          m_buf += Data[j];
        } else {
          m_buf += ".";
        }
      }
      m_buf += "\n";
    }
  }
  // 十六进制dump尾部
  size_t k = i % 16;
  if (k != 0) {
    for (size_t j = 0; j < 16 - k; j++) m_buf += "   ";
    m_buf += "\t; ";

    for (size_t j = i - k; j <= i; ++j) {
      if (((Data[j] & 0xFF) > 31) && ((Data[j] & 0xFF) < 0x7F)) {
        m_buf += Data[j];
      } else {
        m_buf += ".";
      }
    }
    m_buf += '\n';
  }
}

LogInfo::~LogInfo() {
  if (isAuto) {
    m_buf += "\n";
    CLoggerServer::Trace(*this);
  }
}