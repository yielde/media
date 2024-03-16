#ifndef RECORDFILE_H
#define RECORDFILE_H
#include <QJsonObject>
#include <QString>

// #include "ssltool.h"

class RecordFile {
public:
    RecordFile(const QString& path);
    ~RecordFile();
    QJsonObject& config();
    bool save();

private:
    QJsonObject m_config;
    QString m_path;
    // SslTool tool;
};

#endif // RECORDFILE_H
