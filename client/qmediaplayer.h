#ifndef QMEDIAPLAYER_H
#define QMEDIAPLAYER_H
#include "vlchelper.h"
#include <QLabel>
#include <QTimer>
class QMediaPlayer : public QLabel {
    Q_OBJECT
public:
    enum PlayerStatus {
        MP_NONE,
        MP_OPEN,
        MP_PLAY,
        MP_PAUSE,
        MP_STOP,
        MP_CLOSE,
        MP_DESTROY,
        MP_MEDIA_INIT_FAILED = -1,
        MP_OPERATOR_FAILED = -2,
        MP_TIMER_INIT_FAILED = -3,
        MP_MEDIA_LOAD_FAILED = -4,
    };

public:
    QMediaPlayer(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit QMediaPlayer(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~QMediaPlayer();
    PlayerStatus status()
    {
        return stat;
    }
    bool is_playing();
    bool is_paused();
    bool has_media_player();
    bool is_mute();
    int64_t get_duration();
    void set_float_text(const QString& text); // TODO: 后期
    void set_title_text();

public slots:
    void open(const QUrl& path);
    void play();
    void pause();
    void stop();
    void close();
    void seek(double pos);
    void set_position(int64_t pos);
    void set_position(int hour, int minute, int second, int millisecond);
    void set_scale(float scale);
    void pick_frame(QImage& frame, int64_t pos); // 取一帧图片
    void set_size(const QSize& sz);
    void volume(int vol);
    void silence();
private slots:
    void handleTimer();

signals:
    void media_status(QMediaPlayer::PlayerStatus s);
    void position(double pos);

private:
    void init_member();

private:
    QTimer* timer;
    vlchelper* vlc;
    QMediaPlayer::PlayerStatus stat;
    QMediaPlayer::PlayerStatus last;
};

#endif // QMEDIAPLAYER_H
