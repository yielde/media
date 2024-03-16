#include "recordfile.h"

#include <QFile>
// RecordFile::RecordFile(const QString& path)
// {
//     QFile file(path);
//     m_path = path;
//     qDebug() << m_path;
//     do {
//         if (!file.open(QIODevice::ReadOnly)) {
//             break;
//         }
//         QByteArray data = file.readAll();
//         if (data.size() <= 0) {
//             break;
//         }
//         data = tool.rsaDecode(data);
//         qDebug() << __FILE__ << "(" << __LINE__ << "):" << data;
//         int i = 0;
//         for (; i < data.size(); ++i) {
//             if ((int)data[i] >=)
//         }
//     }
// }

RecordFile::RecordFile(const QString& path)
{
    QJsonValue value = QJsonValue();
    m_config.insert("user", value);
    m_config.insert("password", value);
    m_config.insert("auto", false); // 自动登录
    m_config.insert("remember", false); // 记住密码
    m_config.insert("date", "2021-04-01 10:10:10"); //
}

QJsonObject& RecordFile::config()
{
    return m_config;
}

RecordFile::~RecordFile()
{
    save();
}

bool RecordFile::save()
{
    return true;
}
