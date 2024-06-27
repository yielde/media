#ifndef _HTTPPARSER_H_
#define _HTTPPARSER_H_
#include <map>

#include "http_parser.h"
#include "socket.h"

class CHttpParser {
 public:
  CHttpParser();
  CHttpParser(const CHttpParser& http);
  CHttpParser& operator=(const CHttpParser& http);

  size_t Parser(const Buffer& data);
  unsigned Method() const { return m_parser.method; }
  const std::map<Buffer, Buffer>& Headers() { return m_HeaderValues; }
  const Buffer& Status() const { return m_status; }
  const Buffer& Url() const { return m_url; }
  const Buffer& Body() const { return m_body; }
  unsigned Errno() const { return m_parser.http_errno; }

 protected:
  static int OnMessageBegin(http_parser* parser);
  static int OnUrl(http_parser* parser, const char* at, size_t length);
  static int OnStatus(http_parser* parser, const char* at, size_t length);
  static int OnHeaderFiled(http_parser* parser, const char* at, size_t length);
  static int OnHeaderValue(http_parser* parser, const char* at, size_t length);
  static int OnHeadersComplete(http_parser* parser);
  static int OnBody(http_parser* parser, const char* at, size_t length);
  static int OnMessageComplete(http_parser* parser);

  int OnMessageBegin();
  int OnUrl(const char* at, size_t length);
  int OnStatus(const char* at, size_t length);
  int OnHeaderFiled(const char* at, size_t length);
  int OnHeaderValue(const char* at, size_t length);
  int OnHeadersComplete();
  int OnBody(const char* at, size_t length);
  int OnMessageComplete();

 private:
  http_parser m_parser;
  http_parser_settings m_settings;
  std::map<Buffer, Buffer> m_HeaderValues;
  Buffer m_status;
  Buffer m_url;
  Buffer m_body;
  bool m_complete;
  Buffer m_lastFiled;
};

class UrlParser {
 public:
  UrlParser(const Buffer& url);
  ~UrlParser() {}
  int Parser();
  Buffer operator[](const Buffer& name) const;
  Buffer Protocol() const { return m_protocol; }
  Buffer Host() const { return m_host; }
  int Port() const { return m_port; }
  void SetUrl(const Buffer& url);
  const Buffer Uri() const { return m_uri; }

 private:
  Buffer m_url;
  Buffer m_uri;
  Buffer m_protocol;
  Buffer m_host;
  int m_port;
  std::map<Buffer, Buffer> m_values;
};
#endif