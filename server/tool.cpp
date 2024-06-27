#include "mysqlclient.h"

MYSQL_DECLARE_TABLE_CLASS(video_user_mysql, _mysql_table_)
MYSQL_DECLARE_FIELD(TYPE_INT, user_id, NOT_NULL | PRIMARY_KEY | AUTOINCREMENT,
                    "INTEGER", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_VARCHAR, user_qq, NOT_NULL, "VARCHAR", "(15)", "", "")
MYSQL_DECLARE_FIELD(TYPE_VARCHAR, user_phone, DEFAULT, "VARCHAR", "(11)",
                    "'18888888888'", "")  // �ֻ�
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_name, NOT_NULL, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_nick, NOT_NULL, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_wechat, DEFAULT, "TEXT", "", "NULL", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_wechat_id, DEFAULT, "TEXT", "", "NULL", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_address, DEFAULT, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_province, DEFAULT, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_country, DEFAULT, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_age, DEFAULT | CHECK, "INTEGER", "", "18",
                    "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_male, DEFAULT, "BOOL", "", "1", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_flags, DEFAULT, "TEXT", "", "0", "")
MYSQL_DECLARE_FIELD(TYPE_REAL, user_experience, DEFAULT, "REAL", "", "0.0", "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_level, DEFAULT | CHECK, "INTEGER", "", "0",
                    "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_class_priority, DEFAULT, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_REAL, user_time_per_viewer, DEFAULT, "REAL", "", "",
                    "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_career, NONE, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_password, NOT_NULL, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_birthday, NONE, "DATETIME", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_describe, NONE, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_TEXT, user_education, NONE, "TEXT", "", "", "")
MYSQL_DECLARE_FIELD(TYPE_INT, user_register_time, DEFAULT, "DATETIME", "",
                    "LOCALTIME()", "")
MYSQL_DECLARE_TABLE_CLASS_END()

int main() {
  video_user_mysql value;
  value.Fields["user_qq"]->LoadFromStr("937013596");
  value.Fields["user_name"]->LoadFromStr("galen");
  value.Fields["user_nick"]->LoadFromStr("galent");
  value.Fields["user_password"]->LoadFromStr("123123");
  value.Fields["user_qq"]->Condition = SQL_INSERT;
  value.Fields["user_name"]->Condition = SQL_INSERT;
  value.Fields["user_nick"]->Condition = SQL_INSERT;
  value.Fields["user_password"]->Condition = SQL_INSERT;

  video_user_mysql user_1;

  CMysqlClient* pClient = new CMysqlClient();
  KeyValue args;
  args["host"] = "127.0.0.1";
  args["user"] = "root";
  args["password"] = "th303th303";
  args["port"] = 3306;
  args["db"] = "video";

  pClient->Connect(args);
  pClient->Exec(user_1.Insert(value));
  pClient->Close();
  return 0;
}
//g++ -o tool tool.cpp mysqlclient.cpp logger.cpp  -lmysqlclient