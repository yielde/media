#include "databasehelper.h"
#include "publiclib.h"
#include "sqlite3.h"

class CSqlite3Client : public CDatabaseClient {
 public:
  CSqlite3Client(const CSqlite3Client&) = delete;
  CSqlite3Client& operator=(const CSqlite3Client&) = delete;

  CSqlite3Client() { m_db = NULL, m_stmt = NULL; }
  virtual ~CSqlite3Client() { Close(); }

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
  static int ExecCallback(void* arg, int count, char** names, char** values);
  int ExecCallback(Result& result, const _Table_& table, int count,
                   char** names, char** values);

 private:
  sqlite3_stmt* m_stmt;
  sqlite3* m_db;

 private:
  class ExecParam {
   public:
    ExecParam(CSqlite3Client* obj, Result& result, const _Table_& table)
        : obj(obj), result(result), table(table) {}
    CSqlite3Client* obj;
    Result& result;
    const _Table_& table;
  };
};

class _sqlite3_table_ : public _Table_ {
 public:
  _sqlite3_table_() : _Table_() {}
  virtual ~_sqlite3_table_() {}

  _sqlite3_table_(const _sqlite3_table_& table);
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

class _sqlite3_field_ : public _Field_ {
 public:
  _sqlite3_field_();
  _sqlite3_field_(int ntype, const Buffer& name, unsigned attr,
                  const Buffer& type, const Buffer& size,
                  const Buffer& default_, const Buffer& check);
  virtual ~_sqlite3_field_();
  _sqlite3_field_(const _sqlite3_field_& field);
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

#define DECLARE_TABLE_CLASS(name, base)                             \
  class name : public base {                                        \
   public:                                                          \
    virtual PTable Copy() const { return PTable(new name(*this)); } \
    name() : base() {                                               \
      Name = #name; /*将name 转换为 "name" */
#define DECLARE_FIELD(ntype, name, attr, type, size, default_, check)          \
  {                                                                            \
    PField field(                                                              \
        new _sqlite3_field_(ntype, #name, attr, type, size, default_, check)); \
    FieldDefine.push_back(field);                                              \
    Fields[#name] = field;                                                     \
  }

#define DECLARE_TABLE_CLASS_END() \
  }                               \
  }                               \
  ;
