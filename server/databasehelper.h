#ifndef _DATABASEHELPTER_H_
#define _DATABASEHELPTER_H_

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "publiclib.h"
class _Table_;
using PTable = std::shared_ptr<_Table_>;

using KeyValue = std::map<Buffer, Buffer>;
using Result = std::list<PTable>;

class CDatabaseClient {
 public:
  CDatabaseClient(const CDatabaseClient&) = delete;
  CDatabaseClient& operator=(const CDatabaseClient&) = delete;

  CDatabaseClient() {}
  virtual ~CDatabaseClient() {}

 public:
  virtual int Connect(const KeyValue& args) = 0;
  virtual int Exec(const Buffer& sql) = 0;
  virtual int Exec(const Buffer& sql, Result& result, const _Table_& table) = 0;
  virtual int StartTransaction() = 0;
  virtual int CommitTransaction() = 0;
  virtual int RollbackTransaction() = 0;
  virtual int Close() = 0;
  virtual bool IsConnected() = 0;
};

enum { SQL_INSERT = 1, SQL_MODIFY = 2, SQL_CONDITION = 4 };
enum {
  NOT_NULL = 1,
  DEFAULT = 2,
  UNIQUE = 4,
  PRIMARY_KEY = 8,
  CHECK = 16,
  AUTOINCREMENT = 32
};
using SqlType = enum {
  TYPE_NULL = 0,
  TYPE_BOOL = 1,

  TYPE_INT = 2,
  TYPE_DATETIME = 4,

  TYPE_REAL = 8,

  TYPE_VARCHAR = 16,
  TYPE_TEXT = 32,

  TYPE_BLOB = 64
};

class _Field_ {
 public:
  _Field_() {}
  _Field_(const _Field_& field);
  _Field_& operator=(const _Field_& field);
  virtual ~_Field_() {}

  virtual Buffer Create() = 0;
  virtual void LoadFromStr(const Buffer& str) = 0;
  virtual Buffer toEqualExp() const = 0;
  virtual Buffer toSqlStr() const = 0;
  virtual operator const Buffer() const = 0;

 public:
  Buffer Name;
  Buffer Type;
  Buffer Size;
  unsigned Attr;
  Buffer Default;
  Buffer Check;
  unsigned Condition;
};

using PField = std::shared_ptr<_Field_>;
using FiledArray = std::vector<PField>;
using FiledMap = std::map<Buffer, PField>;

class _Table_ {
 public:
  _Table_() {}
  virtual ~_Table_() {}
  // 返回创建表的sql语句
  virtual Buffer Create() = 0;
  // 返回创建表的sql语句
  virtual Buffer Drop() = 0;
  // 增删改查条目
  virtual Buffer Insert(const _Table_& values) = 0;
  virtual Buffer Delete(const _Table_& values) = 0;
  virtual Buffer Modify(const _Table_& values) = 0;
  virtual Buffer Query() = 0;
  // 清理使用状态
  virtual void ClearConditionUsed() = 0;
  // 创建表对象
  virtual PTable Copy() const = 0;
  // 获取表的名称
  virtual operator Buffer() const = 0;

 public:
  // 数据库名
  Buffer Database;
  // 表名
  Buffer Name;
  // 表字段
  FiledArray FieldDefine;
  FiledMap Fields;
};

class CDatabaseHelper {};
#endif