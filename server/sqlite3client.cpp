#include "sqlite3client.h"

#include "logger.h"

int CSqlite3Client::Connect(const KeyValue& args) {
  auto it = args.find("host");
  if (it == args.end()) return -1;
  if (m_db != NULL) return -2;
  // 连接sqlite
  int ret = sqlite3_open(it->second, &m_db);
  if (ret != SQLITE_OK) {
    TRACEE("connect failed: %d [%s]", ret, sqlite3_errmsg(m_db));
    return -3;
  }
  return 0;
}

int CSqlite3Client::Exec(const Buffer& sql) {
  if (m_db == NULL) return -1;
  int ret = sqlite3_exec(m_db, sql, NULL, this, NULL);
  if (ret != SQLITE_OK) {
    TRACEE("sql{%s}", BUFFER_TO_CHAR(sql));
    TRACEE("connect failed: %d [%s]", ret, sqlite3_errmsg(m_db));
    return -2;
  }
  return 0;
}

int CSqlite3Client::Exec(const Buffer& sql, Result& result,
                         const _Table_& table) {
  if (m_db == NULL) return -1;
  ExecParam param(this, result, table);
  char* errmsg = NULL;
  // sqlite3_exec是阻塞执行的， param用局部变量即可
  int ret = sqlite3_exec(m_db, sql, &CSqlite3Client::ExecCallback,
                         (void*)&param, &errmsg);
  if (ret != SQLITE_OK) {
    TRACEE("sql{%s}", BUFFER_TO_CHAR(sql));
    TRACEE("connect failed: %d [%s]", ret, errmsg);
    if (errmsg) sqlite3_free(errmsg);
    return -2;
  }
  if (errmsg) sqlite3_free(errmsg);
  return 0;
}

int CSqlite3Client::StartTransaction() {
  if (m_db == NULL) return -1;
  int ret = sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, NULL);
  if (ret != SQLITE_OK) {
    TRACEE("sql{BEGIN TRANSACTION}");
    TRACEE("BEGIN TRANSACTION failed: %d [%s]", ret, sqlite3_errmsg(m_db));
    return -2;
  }
  return 0;
}

int CSqlite3Client::CommitTransaction() {
  if (m_db == NULL) return -1;
  int ret = sqlite3_exec(m_db, "COMMIT TRANSACTION", 0, 0, NULL);
  if (ret != SQLITE_OK) {
    TRACEE("sql{COMMIT TRANSACTION}");
    TRACEE("COMMIT TRANSACTION failed: %d [%s]", ret, sqlite3_errmsg(m_db));
    return -2;
  }
  return 0;
}

int CSqlite3Client::RollbackTransaction() {
  if (m_db == NULL) return -1;
  int ret = sqlite3_exec(m_db, "ROLLBACK TRANSACTION", 0, 0, NULL);
  if (ret != SQLITE_OK) {
    TRACEE("sql{ROLLBACK TRANSACTION}");
    TRACEE("ROLLBACK TRANSACTION failed: %d [%s]", ret, sqlite3_errmsg(m_db));
    return -2;
  }
  return 0;
}

int CSqlite3Client::Close() {
  if (m_db == NULL) return -1;

  int ret = sqlite3_close(m_db);
  if (ret != SQLITE_OK) {
    TRACEE("Close failed: %d [%s]", ret, sqlite3_errmsg(m_db));
    return -2;
  }
  m_db = NULL;
  return 0;
}

bool CSqlite3Client::IsConnected() { return m_db != NULL; }

int CSqlite3Client::ExecCallback(void* arg, int count, char** names,
                                 char** values) {
  ExecParam* param = (ExecParam*)arg;
  return param->obj->ExecCallback(param->result, param->table, count, names,
                                  values);
}

int CSqlite3Client::ExecCallback(Result& result, const _Table_& table,
                                 int count, char** values, char** names) {
  PTable pTable = table.Copy();
  if (pTable == nullptr) {
    TRACEE("table %s", (Buffer)table);
    return -1;
  }
  for (int i = 0; i < count; ++i) {
    Buffer name = names[i];
    auto it = pTable->Fields.find(name);
    if (it != pTable->Fields.end()) {
      if (values[i] != NULL) it->second->LoadFromStr(values[i]);
    }
  }
  result.push_back(pTable);
  return 0;
}

_sqlite3_table_::_sqlite3_table_(const _sqlite3_table_& table) {
  Database = table.Database;
  Name = table.Name;
  for (unsigned i = 0; i < table.FieldDefine.size(); ++i) {
    PField field = PField(
        new _sqlite3_field_(*(_sqlite3_field_*)table.FieldDefine[i].get()));
    FieldDefine.push_back(field);
    Fields[field->Name] = field;
  }
}

Buffer _sqlite3_table_::Create() {
  // CREATE TABLE IF NOT EXISTS 数据库.表明 (字段定义);
  Buffer sql = "CREATE TABLE IF NOT EXISTS " + (Buffer)(*this) + "(\r\n";
  for (unsigned i = 0; i < FieldDefine.size(); ++i) {
    sql += FieldDefine[i]->Create();
    if (i < FieldDefine.size() - 1) sql += ",";
  }
  sql += ");";
  TRACEI("sql = %s", BUFFER_TO_CHAR(sql));
  return sql;
}

Buffer _sqlite3_table_::Drop() {
  Buffer sql = "DROP TABLE " + (Buffer)(*this) + ";";
  TRACEI("sql = %s", BUFFER_TO_CHAR(sql));
  return sql;
}

Buffer _sqlite3_table_::Insert(const _Table_& values) {
  // INSERT INTO 表名 (字段1，字段2，。。。) VALUES(字段1值，字段2值，。。。)
  Buffer sql = "INSERT INTO " + (Buffer)(*this) + "(";
  bool isfirst = true;
  for (size_t i = 0; i < values.FieldDefine.size(); ++i) {
    if (values.FieldDefine[i]->Condition & SQL_INSERT) {
      if (!isfirst)
        sql += ",";
      else
        isfirst = false;
      sql += (Buffer)(*values.FieldDefine[i]);
    }
  }
  sql += ") VALUES(";
  isfirst = true;
  for (size_t i = 0; i < values.FieldDefine.size(); ++i) {
    if (values.FieldDefine[i]->Condition & SQL_INSERT) {
      if (!isfirst)
        sql += ",";
      else
        isfirst = false;
      sql += values.FieldDefine[i]->toSqlStr();
    }
  }
  sql += ");";
  TRACEI("sql = %s", BUFFER_TO_CHAR(sql));
  return sql;
}

Buffer _sqlite3_table_::Delete(const _Table_& values) {
  // DELETE FROM 表名 WHERE 条件
  Buffer sql = "DELETE FROM " + (Buffer)(*this);
  Buffer Where = "";
  bool isfirst = true;
  for (size_t i = 0; i < values.FieldDefine.size(); ++i) {
    if (values.FieldDefine[i]->Condition & SQL_CONDITION) {
      if (!isfirst)
        Where += " AND ";
      else {
        sql += " ";
        isfirst = false;
      }
      Where += (Buffer)(*values.FieldDefine[i]) + "=" +
               values.FieldDefine[i]->toSqlStr();
    }
  }
  if (Where.size() > 0) sql += " WHERE " + Where;
  sql += ";";
  TRACEI("sql = %s", BUFFER_TO_CHAR(sql));
  return sql;
}

Buffer _sqlite3_table_::Modify(const _Table_& values) {
  // UPDATE 表名 SET 字段1=字段1值, 字段2=字段2值, ... [WHERE 条件];
  Buffer sql = "UPDATE " + (Buffer)(*this) + " SET";
  bool isfirst = true;
  for (size_t i = 0; i < values.FieldDefine.size(); ++i) {
    if (values.FieldDefine[i]->Condition & SQL_MODIFY) {
      if (!isfirst)
        sql += ",";
      else {
        sql += " ";
        isfirst = false;
      }
      sql += (Buffer)(*values.FieldDefine[i]) + "=" +
             values.FieldDefine[i]->toSqlStr();
    }
  }
  isfirst = true;
  Buffer Where = "";
  for (size_t i = 0; i < values.FieldDefine.size(); ++i) {
    if (values.FieldDefine[i]->Condition & SQL_CONDITION) {
      if (!isfirst)
        Where += " AND ";
      else {
        isfirst = false;
      }
      Where += (Buffer)*values.FieldDefine[i] + "=" +
               values.FieldDefine[i]->toSqlStr();
    }
  }
  if (Where.size() > 0) sql += " WHERE " + Where;
  sql += ";";
  TRACEI("sql = %s", BUFFER_TO_CHAR(sql));
  return sql;
}

Buffer _sqlite3_table_::Query() {
  // SELECT 字段1，字段2，。。。 FROM 表全名; （暂时不做WHERE查询）
  Buffer sql = "SELECT ";
  for (size_t i = 0; i < FieldDefine.size(); ++i) {
    if (i > 0) sql += ',';
    sql += '"' + FieldDefine[i]->Name + '"' + ' ';
  }
  sql += " FROM " + (Buffer)(*this) + ';';
  TRACEI("sql = %s", BUFFER_TO_CHAR(sql));
  return sql;
}

void _sqlite3_table_::ClearConditionUsed() {
  for (size_t i = 0; i < FieldDefine.size(); ++i) {
    FieldDefine[i]->Condition = 0;
  }
}

PTable _sqlite3_table_::Copy() const {
  return PTable(new _sqlite3_table_(*this));
}

_sqlite3_table_::operator Buffer() const {
  Buffer Head;
  if (Database.size()) {
    Head = '"' + Database + "\".";
  }
  return Head + '"' + Name + '"';
}

_sqlite3_field_::_sqlite3_field_() : _Field_() { nType = TYPE_NULL; }

_sqlite3_field_::_sqlite3_field_(int ntype, const Buffer& name, unsigned attr,
                                 const Buffer& type, const Buffer& size,
                                 const Buffer& default_, const Buffer& check) {
  Condition = 0;
  nType = ntype;
  switch (ntype) {
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
      Value.String = new Buffer();
      break;
  }
  Name = name;
  Attr = attr;
  Type = type;
  Size = size;
  Default = default_;
  Check = check;
}

_sqlite3_field_::~_sqlite3_field_() {
  switch (nType) {
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
      if (Value.String) {
        Buffer* p = Value.String;
        Value.String = NULL;
        delete p;
      }
      break;
  }
}

_sqlite3_field_::_sqlite3_field_(const _sqlite3_field_& field) {
  nType = field.nType;
  switch (nType) {
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
      Value.String = new Buffer();
      *Value.String = *field.Value.String;
      break;
  }
  Name = field.Name;
  Type = field.Type;
  Attr = field.Attr;
  Default = field.Default;
  Check = field.Check;
}

Buffer _sqlite3_field_::Create() {
  // "名称" 类型 属性
  Buffer sql = '"' + Name + "\" " + Type + " ";
  if (Attr & NOT_NULL) sql += " NOT NULL ";
  if (Attr & DEFAULT) sql += " DEFAULT " + Default + " ";
  if (Attr & UNIQUE) sql += " UNIQUE ";
  if (Attr & PRIMARY_KEY) sql += " PRIMARY KEY";
  if (Attr & CHECK) sql += " CHECK( " + Check + ") ";
  if (Attr & AUTOINCREMENT) sql += " AUTOINCREMENT ";
  TRACEI("Create sql {%s}", BUFFER_TO_CHAR(sql));
  return sql;
}

void _sqlite3_field_::LoadFromStr(const Buffer& str) {
  switch (nType) {
    case TYPE_NULL:
      break;
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
      Value.Integer = atoi(str);
      break;
    case TYPE_REAL:
      Value.Double = atof(str);
      break;
    case TYPE_VARCHAR:
    case TYPE_TEXT:
      *Value.String = str;
      break;
    case TYPE_BLOB:
      *Value.String = Str2Hex(str);
      break;
    default:
      TRACEW("type=%d", nType);
      break;
  }
}

Buffer _sqlite3_field_::toEqualExp() const {
  Buffer sql = (Buffer)(*this) + " = ";
  std::stringstream ss;
  switch (nType) {
    case TYPE_NULL:
      sql += "NULL ";
      break;
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
      ss << Value.Integer;
      sql += ss.str() + " ";
      break;
    case TYPE_REAL:
      sql += "NULL ";
      ss << Value.Double;
      sql += ss.str() + " ";
      break;
    case TYPE_TEXT:
    case TYPE_VARCHAR:
    case TYPE_BLOB:
      sql += '"' + *Value.String + '"' + " ";
      break;
    default:
      TRACEW("type=%d", nType);
      break;
  }
  return sql;
}

Buffer _sqlite3_field_::toSqlStr() const {
  Buffer sql = "";
  std::stringstream ss;
  switch (nType) {
    case TYPE_NULL:
      sql += " NULL ";
      break;
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
      ss << Value.Integer;
      sql += ss.str();
      break;
    case TYPE_REAL:
      sql += "NULL ";
      ss << Value.Double;
      sql += ss.str();
      break;
    case TYPE_TEXT:
    case TYPE_VARCHAR:
    case TYPE_BLOB:
      sql += '"' + *Value.String + '"';
      break;
    default:
      TRACEW("type=%d", nType);
      break;
  }
  return sql;
}
_sqlite3_field_::operator const Buffer() const { return '"' + Name + '"'; }

Buffer _sqlite3_field_::Str2Hex(const Buffer& data) const {
  const char* hex = "0123456789ABCDEF";
  std::stringstream ss;
  for (auto ch : data) {
    ss << hex[(unsigned char)ch >> 4] << hex[(unsigned char)ch & 0xF];
  }
  return ss.str();
}
