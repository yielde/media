#include "qmediaplayer.h"

QMediaPlayer::QMediaPlayer(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f)
    , timer(NULL)
    , vlc(NULL)
{
    init_member();
}

QMediaPlayer::QMediaPlayer(const QString& text, QWidget* parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
    , timer(NULL)
    , vlc(NULL)
{
    init_member();
}

QMediaPlayer::~QMediaPlayer()
{
    if (timer) {
        delete timer;
        timer = NULL;
    }
    if (vlc) {
        delete vlc;
        vlc = NULL;
    }
    stat = MP_DESTROY;
}

void QMediaPlayer::handleTimer()
{
    // TODO: 感觉此处有问题
    static unsigned char tick = 0;
    tick++;
    if (last != stat) {
        emit media_status(stat);
        last = stat;
    }
    if (stat == MP_PLAY) {
        if (vlc) {
            double pos = vlc->gettime();
            pos /= vlc->getduration();
            emit position(pos);
        } else {
            stat = MP_MEDIA_INIT_FAILED;
        }
    }
    if (tick % 10 == 0) {
        tick = 0;
        vlc->update_text();
    }
}

bool QMediaPlayer::is_playing()
{
    if (vlc) {
        return vlc->isplaying();
    }
    return false;
}

bool QMediaPlayer::is_paused()
{
    if (vlc) {
        return vlc->ispause();
    }
    return false;
}

bool QMediaPlayer::has_media_player()
{
    if (vlc) {
        return vlc->has_media_player();
    }
    return false;
}

bool QMediaPlayer::is_mute()
{
    return vlc->ismute();
}

int64_t QMediaPlayer::get_duration()
{
    return vlc->getduration();
}

void QMediaPlayer::set_float_text(const QString& nick)
{
}

void QMediaPlayer::open(const QUrl& path)
{
    if (vlc) {
        vlc->stop();
        vlc->prepare(path.toString());
        stat = MP_OPEN;
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::play()
{
    if (vlc) {
        if (vlc->play() >= 0) {
            stat = MP_PLAY;

        } else {
            stat = MP_OPERATOR_FAILED;
        }
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::pause()
{
    if (vlc) {
        if (vlc->pause() >= 0) {
            stat = MP_PAUSE;
        } else {
            stat = MP_OPERATOR_FAILED;
        }
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::stop()
{
    if (vlc) {
        vlc->stop();
        stat = MP_STOP;
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::close()
{
    if (vlc) {
        vlc->stop();
        stat = MP_NONE;
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::seek(double pos)
{
    if (vlc) {
        vlc->settime(vlc->getduration() * pos);
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::set_position(int64_t pos)
{
    if (vlc) {
        vlc->settime(pos);
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::set_position(int hour, int minute, int second, int millisecond)
{
    int64_t pos = (hour * 3600 + minute * 60 + second) * 1000 + millisecond;
    set_position(pos);
}

void QMediaPlayer::set_scale(float scale)
{
    if (vlc) {
        vlc->set_play_rate(scale);
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::pick_frame(QImage&, int64_t) { }

void QMediaPlayer::set_size(const QSize& sz)
{
    this->resize(sz);
}

void QMediaPlayer::volume(int vol)
{
    if (vlc) {
        vlc->volume(vol);
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::silence()
{
    if (vlc) {
        vlc->silence();
    } else {
        stat = MP_MEDIA_INIT_FAILED;
    }
}

void QMediaPlayer::init_member()
{
    timer = new QTimer(this);
    if (timer == NULL) {
        stat = MP_TIMER_INIT_FAILED;
        return;
    }
    try {
        vlc = new vlchelper(this);
    } catch (const QString& msg) {
        qDebug() << msg;
    } catch (...) {
        if (vlc == NULL) {

            stat = MP_MEDIA_LOAD_FAILED;
            return;
        }
    }
    QLabel::connect(timer, &QTimer::timeout, this, &QMediaPlayer::handleTimer);
    timer->start(500);
    stat = MP_NONE;
    last = MP_NONE;
}
