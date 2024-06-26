#include <mysql/mysql.h>

#include "databasehelper.h"
#include "logger.h"
#include "publiclib.h"

class CMysqlClient : public CDatabaseClient {
 public:
  CMysqlClient(const CMysqlClient&) = delete;
  CMysqlClient& operator=(const CMysqlClient&) = delete;

  CMysqlClient() {
    isInit = false;
    bzero(&m_db, sizeof(m_db));
  }
  virtual ~CMysqlClient() { Close(); }

 public:
  virtual int Connect(const KeyValue& args);
  virtual int Exec(const Buffer& sql);
  virtual int Exec(const Buffer& sql, Result& result, const _Table_& table);
  virtual int StartTransaction();
  virtual int CommitTransaction();
  virtual int RollbackTransaction();
  virtual int Close();
  virtual bool IsConnected();

 private:
  MYSQL m_db;
  bool isInit;

 private:
  class ExecParam {
   public:
    ExecParam(CMysqlClient* obj, Result& result, const _Table_& table)
        : obj(obj), result(result), table(table) {}
    CMysqlClient* obj;
    Result& result;
    const _Table_& table;
  };
};

class _mysql_table_ : public _Table_ {
 public:
  _mysql_table_() : _Table_() {}
  virtual ~_mysql_table_() {}

  _mysql_table_(const _mysql_table_& table);
  // 返回创建表的sql语句
  virtual Buffer Create();
  // 返回创建表的sql语句
  virtual Buffer Drop();
  // 增删改查条目
  virtual Buffer Insert(const _Table_& values);
  virtual Buffer Delete(const _Table_& values);
  virtual Buffer Modify(const _Table_& values);
  virtual Buffer Query();
  virtual void ClearConditionUsed();

  // 创建表对象
  virtual PTable Copy() const;
  // 获取表的名称
  virtual operator Buffer() const;
};

class _mysql_field_ : public _Field_ {
 public:
  _mysql_field_();
  _mysql_field_(int ntype, const Buffer& name, unsigned attr,
                const Buffer& type, const Buffer& size, const Buffer& default_,
                const Buffer& check);
  virtual ~_mysql_field_();
  _mysql_field_(const _mysql_field_& field);
  virtual Buffer Create();
  virtual void LoadFromStr(const Buffer& str);
  virtual Buffer toEqualExp() const;
  virtual Buffer toSqlStr() const;
  virtual operator const Buffer() const;

 private:
  Buffer Str2Hex(const Buffer& data) const;

  union {
    bool Bool;
    int Integer;
    double Double;
    Buffer* String;
  } Value;

  int nType;
};

#define MYSQL_DECLARE_TABLE_CLASS(name, base)                       \
  class name : public base {                                        \
   public:                                                          \
    virtual PTable Copy() const { return PTable(new name(*this)); } \
    name() : base() {                                               \
      Name = #name; /*将name 转换为 "name" */
#define MYSQL_DECLARE_FIELD(ntype, name, attr, type, size, default_, check)  \
  {                                                                          \
    PField field(                                                            \
        new _mysql_field_(ntype, #name, attr, type, size, default_, check)); \
    FieldDefine.push_back(field);                                            \
    Fields[#name] = field;                                                   \
  }
#define MYSQL_DECLARE_TABLE_CLASS_END() \
  }                                     \
  }                                     \
  ;
