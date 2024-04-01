#ifndef MEDIAMP4_H
#define MEDIAMP4_H
#include "mediabase.h"

class MediaMp4 : public MediaBase {
public:
    MediaMp4();
    virtual ~MediaMp4();
    virtual int open(const QUrl& url, uint64_t* fsize);
    virtual sssize_t read(uint8_t* buffer, size_t length);
    virtual int seek(uint64_t offset);
    virtual void close();

private:
    QByteArray key;
    QFile* file;
    uint16_t pos;
};

#endif // MEDIAMP4_H
