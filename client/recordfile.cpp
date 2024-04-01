#include "recordfile.h"

#include <QFile>
#include <QJsonParseError>
RecordFile::RecordFile(const QString& path)
{
    QFile file(path);
    m_path = path;
    do {
        if (!file.open(QIODevice::ReadOnly)) {
            break;
        }
        QByteArray data = file.readAll();
        if (data.size() <= 0) {
            break;
        }
        data = tool.rsaDecode(data);

        int i = 0;
        for (; i < data.size(); ++i) {
            if ((int)data[i] >= (int)0x7F || (int)data[i] < (int)0x0A) {
                data.resize(i);
                break;
            }
        }
        QJsonParseError json_error;
        QJsonDocument document = QJsonDocument::fromJson(data, &json_error);
        if (json_error.error == QJsonParseError::NoError) {
            if (document.isObject()) {
                m_config = document.object();
                return;
            }
        } else {
            qDebug() << __FILE__ << "(" << __LINE__ << "):" << json_error.errorString() << json_error.error;
        }
    } while (false);

    file.close();
    QJsonValue value = QJsonValue();
    m_config.insert("user", value);
    m_config.insert("password", value);
    m_config.insert("auto", false);
    m_config.insert("remember", false);
    m_config.insert("data", "2024-01-01 10:24:00");
    return;
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
    QFile file(m_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate) == false) {
        return false;
    }
    QJsonDocument document = QJsonDocument(m_config);
    file.write(tool.rsaEncode(document.toJson()));
    file.close();
    return true;
}
