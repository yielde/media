
#ifndef _PUBLICLIB_H_
#define _PUBLICLIB_H_
#include <string.h>

#include <string>

class Buffer : public std::string {
 public:
  Buffer() : std::string() {}
  Buffer(size_t size) : std::string() { resize(size); }
  Buffer(const std::string &str) : std::string(str) {}
  Buffer(const char *str) : std::string(str) {}
  Buffer(const char *str, size_t length) : std::string() {
    resize(length);
    memcpy((char *)c_str(), str, length);
  }
  Buffer(const char *begin, const char *end) : std::string() {
    int len = end - begin;
    if (len > 0) {
      resize(len);
      memcpy((char *)c_str(), begin, len);
    }
  }
  operator char *() { return (char *)c_str(); }
  operator char *() const { return (char *)c_str(); }
  operator const char *() const { return c_str(); }
};

#define BUFFER_TO_CHAR(buffer) (char *)(buffer)
#endif