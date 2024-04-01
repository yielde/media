#include "mediamp4.h"
#include <QDebug>

MediaMp4::MediaMp4()
{
    QFile f(":/UI/ico/128-128.png");
    if (f.open(QFile::ReadOnly)) {
        key = f.readAll();
        f.close();
    }
    file = NULL;
}

MediaMp4::~MediaMp4()
{
    if (file) {
        file->close();
        delete file;
    }
    key.clear();
}

int MediaMp4::open(const QUrl& url, uint64_t* fsize)
{
    if (file) {
        file->close();
        delete file;
    }
    file = new QFile("/" + url.path().right(url.path().size() - 1));
    if (file == NULL) {
        return QFile::FatalError;
    }
    if (file->error() != QFile::NoError) {

        int ret = file->error();
        qDebug() << "file: ret:" << ret;
        delete file;
        file = NULL;
        return ret;
    }
    if (file->open(QFile::ReadOnly)) {
        *fsize = file->size();
        qDebug() << "file: " << *fsize;
        pos = uint16_t(*fsize & 0xFFFF);
        return QFile::NoError;
    }
    return file->error();
}

sssize_t MediaMp4::read(uint8_t* buffer, size_t length)
{
    if (file == NULL) {
        return QFile::FatalError;
    }
    qint64 positon = file->pos();
    qint64 len = file->read((char*)buffer, length);
    if (len > 0) {
        for (qint64 i = 0; i < len; ++i) {
            buffer[i] = buffer[i] ^ key.at((pos + positon + i) % key.size()); // TOGO
        }
        return len;
    }
    return QFile::ReadError;
}

int MediaMp4::seek(uint64_t offset)
{
    if (file == NULL) {
        return QFile::FatalError;
    }
    file->seek(offset);
    return file->error();
}

void MediaMp4::close()
{
    if (file) {
        file->close();
    }
}
