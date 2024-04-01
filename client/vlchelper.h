#ifndef VLCHELPER_H
#define VLCHELPER_H

// #include "libvlc/vlc/vlc.h"
#include "mediamp4.h"
#include "vlc/vlc.h"

#include <QFile>
#include <QWidget>
#include <fstream>

class vlcmedia {
public:
    vlcmedia(libvlc_instance_t* instance);
    ~vlcmedia();
    static int open(void* t, void** infile, u_int64_t* fsize);
    static ssize_t read(void* t, uint8_t* buffer, size_t length);
    static int seek(void* t, uint64_t offset);
    static void close(void* t);
    vlcmedia& operator=(const QString& str);
    operator libvlc_media_t*();
    void free();
    QString path();

private:
    int open(void** infile, uint64_t* fsize);
    ssize_t read(uint8_t* buffer, size_t length);
    int seek(uint64_t offset);
    void close();

private:
    QString strPath;
    std::ifstream infile;
    libvlc_instance_t* instance;
    libvlc_media_t* media;
    uint64_t media_size;
    MediaBase* media_instance;
};

class vlchelper {
public:
    vlchelper(QWidget* widget);
    ~vlchelper();
    int prepare(const QString& strPath = "");
    int play();
    int pause();
    int stop();
    int volume(int vol = -1);
    int silence();
    bool isplaying();
    bool ispause() const
    {
        return m_ispause;
    }
    bool ismute();
    libvlc_time_t gettime();
    libvlc_time_t getduration();
    int settime(libvlc_time_t time);
    int set_play_rate(float rate);
    float get_play_rate();
    void init_logo();
    void update_logo();
    bool is_logo_enable();
    void init_text(const QString& text);
    void update_text();
    bool is_text_enable();
    bool has_media_player();

private:
    void set_float_text();

private:
    QFile m_logo;
    libvlc_instance_t* m_instance;
    libvlc_media_player_t* m_player;
    int winHeight;
    int winWidth;
    libvlc_time_t m_duration;
    vlcmedia* m_media;
    void* m_hwnd;
    bool m_ispause;
    bool m_isplaying;
    bool m_issilence;
    int m_volume;
    QString text;
    QWidget* m_widget;
};

#endif // VLCHELPER_H
