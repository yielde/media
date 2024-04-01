#include "vlchelper.h"
#include <QDebug>
#include <QRandomGenerator>

using namespace std::placeholders;

vlchelper::vlchelper(QWidget* widget)
    : m_logo(":/UI/ico/128-128.png")
{
    const char* const args[] = {
        "--sub-filter=logo",
        "--sub-filter=marq"
    };

    // m_instance = libvlc_new(sizeof(args) / sizeof(args[0]), args);

    m_instance = libvlc_new(0, NULL);

    if (m_instance != NULL) {
        m_media = new vlcmedia(m_instance);
    } else {
        m_media = NULL;
        throw QString("没有vlc插件？");
    }
    m_player = NULL;
    m_hwnd = (void*)widget->winId();
    winWidth = widget->frameGeometry().width();
    winHeight = widget->frameGeometry().height();
    m_widget = widget;
    m_ispause = false;
    m_isplaying = false;
    m_volume = 80;
}

vlchelper::~vlchelper()
{
    if (m_player != NULL) {
        stop();
        libvlc_media_player_set_nsobject(m_player, NULL);
        libvlc_media_player_release(m_player);
        m_player = NULL;
    }
    if (m_media != NULL) {
        m_media->free();
        m_media = NULL;
    }
    if (m_instance != NULL) {
        libvlc_release(m_instance);
        m_instance = NULL;
    }
}

QString vlcmedia::path()
{
    return strPath;
}

int vlchelper::prepare(const QString& strPath)
{

    *m_media = strPath;

    if (m_media == NULL) {
        return -1;
    }
    if (m_player != NULL) {
        libvlc_media_player_release(m_player);
    }
    m_player = libvlc_media_player_new_from_media(*m_media);

    if (m_player == NULL) {
        return -2;
    }

    m_duration = libvlc_media_get_duration(*m_media);
    libvlc_media_player_set_nsobject(m_player, m_hwnd);
    libvlc_audio_set_volume(m_player, m_volume);
    libvlc_video_set_aspect_ratio(m_player, "16:9");

    m_ispause = false;
    m_isplaying = false;
    m_issilence = false;
    winHeight = m_widget->frameGeometry().height();
    winWidth = m_widget->frameGeometry().width();
    qDebug() << "--- vlchelper prepare finished ---";
    return 0;
}

void vlchelper::update_text()
{
    // qDebug() << "update text";
}

int vlchelper::play()
{
    if (m_player == NULL) {
        return -1;
    }

    if (m_media->path().size() <= 0) {
        m_ispause = false;
        m_isplaying = false;
        return -2;
    }

    if (m_ispause) {
        int ret = libvlc_media_player_play(m_player);
        if (ret == 0) {
            m_ispause = false;
            m_isplaying = true;
        }
        return ret;
    }
    libvlc_video_set_mouse_input(m_player, 0);
    libvlc_video_set_key_input(m_player, 0);
    libvlc_set_fullscreen(m_player, 1);
    m_isplaying = true;
    return libvlc_media_player_play(m_player);
}
int vlchelper::pause()
{
    if (m_player == NULL) {
        return -1;
    }
    libvlc_media_player_pause(m_player);
    m_isplaying = false;
    m_ispause = true;
    return 0;
}

int vlchelper::stop()
{
    if (m_player == NULL) {
        return -1;
    }
    libvlc_media_player_stop(m_player);
    m_isplaying = false;
    m_ispause = false;
    return 0;
}

int vlchelper::volume(int vol)
{

    if (m_player == NULL) {
        return -1;
    }
    if (vol < 0) {
        return m_volume;
    }
    int ret = libvlc_audio_set_volume(m_player, vol);
    if (ret == 0) {
        m_volume = vol;
        return m_volume;
    }
    return ret;
}

int vlchelper::silence()
{
    if (m_player == NULL) {
        return -1;
    }

    if (m_issilence) {
        libvlc_audio_set_mute(m_player, 0);
        m_issilence = false;
    } else {
        libvlc_audio_set_mute(m_player, 1);
        m_issilence = true;
    }

    return m_issilence;
}

bool vlchelper::isplaying()
{
    return m_isplaying;
}
bool vlchelper::ismute()
{
    if (m_player && m_isplaying) {
        return libvlc_audio_get_mute(m_player) == 1;
    }
    return false;
}

libvlc_time_t vlchelper::gettime()
{
    if (m_player == NULL) {
        return -1;
    }
    return libvlc_media_player_get_time(m_player);
}

int vlchelper::settime(libvlc_time_t time)
{
    if (m_player == NULL) {
        return -1;
    }
    libvlc_media_player_set_time(m_player, time);
    return 0;
}

libvlc_time_t vlchelper::getduration()
{
    if (m_media == NULL) {
        return -1;
    }
    if (m_duration < 0) {
        m_duration = libvlc_media_get_duration(*m_media);
        qDebug() << "getduration: " << m_duration;
    }
    return m_duration;
}

int vlchelper::set_play_rate(float rate)
{
    if (m_media == NULL) {
        return -1;
    }
    return libvlc_media_player_set_rate(m_player, rate);
}

float vlchelper::get_play_rate()
{
    if (m_media == NULL) {
        return -1.0;
    }
    return libvlc_media_player_get_rate(m_player);
}

void vlchelper::init_logo()
{
    // libvlc_video_set_logo_int(m_player, libvlc_logo_file, m_logo.handle());
    libvlc_video_set_logo_string(m_player, libvlc_logo_file, ":UI/ico/128-128.png"); // Logo 文件名
    libvlc_video_set_logo_int(m_player, libvlc_logo_x, 0); // logo的 X 坐标。
    // libvlc_video_set_logo_int(m_player, libvlc_logo_y, 0); // logo的 Y 坐标。
    libvlc_video_set_logo_int(m_player, libvlc_logo_delay, 100); // 标志的间隔图像时间为毫秒,图像显示间隔时间 0 - 60000 毫秒。
    libvlc_video_set_logo_int(m_player, libvlc_logo_repeat, -1); // 标志logo的循环,  标志动画的循环数量。-1 = 继续, 0 = 关闭
    libvlc_video_set_logo_int(m_player, libvlc_logo_opacity, 122);
    // logo 透明度 (数值介于 0(完全透明) 与 255(完全不透明)
    libvlc_video_set_logo_int(m_player, libvlc_logo_position, 5);
    // 1 (左), 2 (右), 4 (顶部), 8 (底部), 5 (左上), 6 (右上), 9 (左下), 10 (右下),您也可以混合使用这些值，例如 6=4+2    表示右上)。
    libvlc_video_set_logo_int(m_player, libvlc_logo_enable, 1); // 设置允许添加logo
}
void vlchelper::update_logo()
{
    static int alpha = 0;
    // static int pos[] = {1, 5, 4, 6, 2, 10, 8, 9};
    int height = QRandomGenerator::global()->bounded(20, winHeight - 20);
    libvlc_video_set_logo_int(m_player, libvlc_logo_y, height); // logo的 Y 坐标。
    int width = QRandomGenerator::global()->bounded(20, winWidth - 20);
    libvlc_video_set_logo_int(m_player, libvlc_logo_x, width); // logo的 X 坐标。
    libvlc_video_set_logo_int(m_player, libvlc_logo_opacity, (alpha++) % 80 + 20); // 透明度
}
bool vlchelper::is_logo_enable()
{
    if (m_player == NULL) {
        return -1;
    }
    return libvlc_video_get_logo_int(m_player, libvlc_logo_enable) == 1;
}

void vlchelper::init_text(const QString& text)
{
    this->text = text;
}

bool vlchelper::is_text_enable()
{
    // like logo
}
bool vlchelper::has_media_player()
{
    return (m_media != NULL && m_player != NULL);
}

// vlcmedia

vlcmedia::vlcmedia(libvlc_instance_t* ins)
    : instance(ins)
{
    media = NULL;
    media_instance = new MediaMp4();
}

void vlcmedia::free()
{
    if (media != NULL) {
        libvlc_media_release(media);
    }
}

vlcmedia::~vlcmedia()
{
    if (media) {
        free();
    }
    if (media_instance) {
        delete media_instance;
        media_instance = NULL;
    }
}

int vlcmedia::open(void* thiz, void** infile, uint64_t* fsize)
{
    vlcmedia* _this = (vlcmedia*)thiz;
    return _this->open(infile, fsize);
}

ssize_t vlcmedia::read(void* thiz, uint8_t* buffer, size_t length)
{
    vlcmedia* _this = (vlcmedia*)thiz;
    return _this->read(buffer, length);
}

int vlcmedia::seek(void* thiz, uint64_t offset)
{
    vlcmedia* _this = (vlcmedia*)thiz;
    return _this->seek(offset);
}

void vlcmedia::close(void* thiz)
{
    vlcmedia* _this = (vlcmedia*)thiz;
    _this->close();
}

int vlcmedia::open(void** infile, uint64_t* fsize)
{
    //"file:///"
    if (media_instance) {
        *infile = this;
        int ret = media_instance->open(strPath, fsize);
        media_size = *fsize;
        return ret;
    }
    this->infile.open(strPath.toStdString().c_str() + 8, std::ios::binary | std::ios::in);
    this->infile.seekg(0, std::ios::end);
    *fsize = (uint64_t)this->infile.tellg();
    media_size = *fsize;
    this->infile.seekg(0);
    *infile = this;
    return 0;
}

ssize_t vlcmedia::read(uint8_t* buffer, size_t length)
{
    if (media_instance) {
        return media_instance->read(buffer, length);
    }
    // qDebug() << __FUNCTION__ << " length:" << length;
    uint64_t pos = (uint64_t)infile.tellg();
    // qDebug() << __FUNCTION__ << " positon:" << pos;
    if (media_size - pos < length) {
        length = media_size - pos;
    }
    infile.read((char*)buffer, length);
    return infile.gcount();
}

int vlcmedia::seek(uint64_t offset)
{
    if (media_instance) {
        return media_instance->seek(offset);
    }
    // qDebug() << __FUNCTION__ << ":" << offset;
    infile.clear();
    infile.seekg(offset);
    return 0;
}

void vlcmedia::close()
{
    if (media_instance) {
        return media_instance->close();
    }
    // qDebug() << __FUNCTION__;
    infile.close();
}

vlcmedia::operator libvlc_media_t*()
{
    return media; // 隐式类型转换
}

vlcmedia& vlcmedia::operator=(const QString& str)
{
    if (media) {
        free();
    }
    strPath = str;
    media = libvlc_media_new_callbacks(
        instance,
        &vlcmedia::open,
        &vlcmedia::read,
        &vlcmedia::seek,
        &vlcmedia::close,
        this);
    return *this;
}
