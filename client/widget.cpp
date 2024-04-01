#include "widget.h"
#include "ui_widget.h"
#include <QDropEvent>
#include <QMimeData>
#include <QScreen>
#include <QUrl>

const char* PAUSE_STYLESHEET = "QPushButton#playButton{"
                               "border-image: url(:/UI/white/zanting.png);"
                               "background-color: transparent;}"
                               "QPushButton#playButton:hover{"
                               "border-image: url(:/UI/blue/zanting.png);}"
                               "QPushButton#playButton:pressed{"
                               "border-image: url(:/UI/gray/zanting.png);}";
const char* PLAY_STYLESHEET = "QPushButton#playButton{"
                              "border-image: url(:/UI/blue/bofang.png);"
                              "background-color: transparent;}"
                              "QPushButton#playButton:hover{"
                              "border-image: url(:/UI/gray/bofang.png);}"
                              "QPushButton#playButton:pressed{"
                              "border-image: url(:/UI/white/bofang.png);}";
// 列表显示的时候，按钮的样式
const char* LIST_SHOW = "QPushButton{border-image: url(:/UI/images/arrow-right.png);}"
                        "QPushButton:hover{border-image: url(:/UI/images/arrow-right.png);}"
                        "QPushButton:pressed{border-image: url(:/UI/images/arrow-right.png);}";
// 列表隐藏的时候，按钮的样式
const char* LIST_HIDE = "QPushButton{border-image: url(:/UI/images/arrow-left.png);}"
                        "QPushButton:hover{border-image: url(:/UI/images/arrow-left.png);}"
                        "QPushButton:pressed{border-image: url(:/UI/images/arrow-left.png);}";

// 当前播放文件的样式
const char* CURRENT_PLAY_NORMAL = "background-color: rgba(255, 255, 255, 0);\nfont: 10pt \"黑体\";"
                                  "\ncolor: rgb(255, 255, 255);";

const char* CURRENT_PLAY_FULL = "background-image: url(:/UI/images/screentop.png);"
                                "background-color: transparent;"
                                "\nfont: 20pt \"黑体\";\ncolor: rgb(255, 255, 255);";

// 最大化和恢复按钮
const char* SCREEN_RESTORE_STYLE = "QPushButton{border-image: url(:/UI/images/huifu.png);}\n"
                                   "QPushButton:hover{border-image:url(:/UI/images/huifu-hover.png);}\n"
                                   "QPushButton:pressed{border-image: url(:/UI/images/huifu.png);}";

const char* SCREEN_MAX_STYLE = "QPushButton{border-image: url(:/UI/images/fangda.png);}\n"
                               "QPushButton:hover{border-image:url(:/UI/images/fangda-hover.png);}\n"
                               "QPushButton:pressed{border-image: url(:/UI/images/fangda.png);}";
Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , helper(this)
{
    ui->setupUi(this);
    slider_pressed = false;
    rate = 1.0f;
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setAcceptDrops(true);
    current_play = -1;
    ui->preButton->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->horizontalSlider->setRange(0, 1000);
    ui->horizontalSlider->setValue(0);
    ui->volumeButton->installEventFilter(this);
    ui->volumeSlider->installEventFilter(this);
    ui->listWidget->installEventFilter(this);
    ui->scaleButton->installEventFilter(this); // 倍数按钮
    ui->horizontalSlider->installEventFilter(this);
    installEventFilter(this);
    ui->volumeSlider->setVisible(false);
    ui->volumeSlider->setRange(0, 100);
    ui->time0_5->setVisible(false);
    ui->time1->setVisible(false);
    ui->time1_5->setVisible(false);
    ui->time2->setVisible(false);

    volumeSliderTimerID = -1;
    timesID = -1;
    timesCount = 0;
    fullScreenTimerID = -1;
    volumeCount = 0;
    setTime(0, 0, 0);
    setTime2(0, 0, 0);
    QRect rect = QGuiApplication::primaryScreen()->geometry();
    int currentScreenWidth = rect.width();
    int currentScreenHeight = rect.height();
    if (currentScreenWidth > 800 && currentScreenHeight > 600) {
        setMaximumSize(currentScreenWidth, currentScreenHeight);
    } else {
        setMaximumSize(800, 600);
    }
    init_media();
    info.setWindowFlag(Qt::FramelessWindowHint);
    info.setWindowModality(Qt::ApplicationModal);
    connect(&info, &InfoForm::button_clicked, this, &Widget::slot_connect_clicked);
    info.move(this->frameGeometry().x() + (this->width() - info.width()) / 2, // 弹出窗口居中
        this->frameGeometry().y() + (this->height() - info.height()) / 2);
    ui->curPlay->setText("");
    helper.update(width(), height());
    setMinimumSize(800, 600);
    save_default_rect_info();
    net = new QNetworkAccessManager(this);
    connect(net, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(slots_login_request_finished(QNetworkReply*)));
    connect(this, SIGNAL(update_pos(double)), ui->media, SLOT(seek(double)));
}

Widget::~Widget()
{
    delete ui;
    delete net;
}

void Widget::init_media()
{
    connect(this, &Widget::open, ui->media, &QMediaPlayer::open);
    connect(this, &Widget::play, ui->media, &QMediaPlayer::play);
    connect(this, &Widget::pause, ui->media, &QMediaPlayer::pause);
    connect(this, &Widget::stop, ui->media, &QMediaPlayer::stop);
    connect(this, &Widget::close_media, ui->media, &QMediaPlayer::close);
    connect(this, &Widget::seek, ui->media, &QMediaPlayer::seek);
    connect(this, &Widget::set_scale, ui->media, &QMediaPlayer::set_scale);
    connect(this, &Widget::set_size, ui->media, &QMediaPlayer::set_size);
    connect(this, &Widget::volume, ui->media, &QMediaPlayer::volume);
    connect(this, &Widget::silence, ui->media, &QMediaPlayer::silence);
}

void Widget::save_default_rect_info()
{
    QString names[] = // 子ui名称
        {
            // 34
            "backgroundLb", "media", "downbkgndLb", "listWidget", // 4
            "horizontalSlider", "preButton", "nextButton", "volumeButton", // 4
            "timeLb", "volumeSlider", "label", "logoLabel", // 4
            "userInfo", "loginstatus", "settingBtn", "miniButton", // 4
            "fangdaButton", "closeButton", "showhideList", "playInfo", // 4
            "playInfoIco", "curplay", "stopButton", "time2Lb", // 4
            "fullscreenBtn", "settingButton", "scaleButton", "autoButton", // 4
            "screentop", "time1_5", "time2", "time1", "time0_5", "playButton" // 6
        };

    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int currentScreenWidth = rec.size().width();
    int currentScreenHeight = rec.size().height() - 50;

    qDebug() << "currentScreen" << ui->media->geometry().width() << "----" << ui->media->geometry().height()
             << ":" << ui->media->geometry().x() << ui->media->geometry().y();

    QRect max_rect[] = // 最大化时的尺寸
        {
            QRect(0, 41, currentScreenWidth, currentScreenHeight - 110), // backgroundLb 视频播放背景黑底板
            QRect(0, 41, currentScreenWidth - 300, currentScreenHeight - 110), // media 视频播放控件
            QRect(0, 0, 0, 0), // downbkgndLb 全屏的时候，下方的背景板
            QRect(currentScreenWidth - 300, 41, 300, currentScreenHeight - 41), // listWidget 播放列表
            QRect(0, currentScreenHeight - 68, currentScreenWidth - 300, 22), // horizontalSlider 播放进度条
            QRect(110, currentScreenHeight - 45, 32, 32), // preButton 上一条按钮
            QRect(160, currentScreenHeight - 45, 32, 32), // nextButton 下一条按钮
            QRect(currentScreenWidth - 400, currentScreenHeight - 45, 32, 32), // volumeButton
            QRect(215, currentScreenHeight - 45, 90, 25), // timeLb
            QRect(currentScreenWidth - 398, currentScreenHeight - 205, 22, 160), // volumeSlider 音量滑动条 10
            QRect(0, 0, 1, 1), // label
            QRect(5, 5, 140, 30), // logoLabel
            QRect(currentScreenWidth - 270, 7, 28, 28), // userInfo
            QRect(currentScreenWidth - 270, 7, 45, 25), // loginstatus
            QRect(currentScreenWidth - 160, 5, 30, 30), // settingBtn
            QRect(currentScreenWidth - 120, 5, 30, 30), // miniButton
            QRect(currentScreenWidth - 80, 5, 30, 30), // fangdaButton
            QRect(currentScreenWidth - 40, 5, 30, 30), // closeButton
            QRect(currentScreenWidth - 360, (currentScreenHeight - 170) / 2, 60, 60), // showhideList
            QRect(currentScreenWidth - 298, 43, 296, 46), // playInfo
            QRect(currentScreenWidth - 300, 41, 32, 50), // playInfoIco
            QRect(currentScreenWidth - 265, 50, 260, 32), // curplay
            QRect(60, currentScreenHeight - 45, 32, 32), // stopButton
            QRect(300, currentScreenHeight - 45, 120, 25), // time2Lb
            QRect(currentScreenWidth - 350, currentScreenHeight - 45, 32, 32), // fullscreenBtn
            QRect(currentScreenWidth - 450, currentScreenHeight - 45, 32, 32), // settingButton
            QRect(currentScreenWidth - 500, currentScreenHeight - 45, 42, 32), // scaleButton
            QRect(currentScreenWidth - 560, currentScreenHeight - 45, 42, 32), // autoButton
            QRect(0, 0, 1, 1), // 顶部内容提示
            QRect(currentScreenWidth - 200, currentScreenHeight - 157, 42, 48) /*1.5*/,
            QRect(currentScreenWidth - 200, currentScreenHeight - 185, 42, 48) /*2*/,
            QRect(currentScreenWidth - 200, currentScreenHeight - 129, 42, 48) /*1*/,
            QRect(currentScreenWidth - 200, currentScreenHeight - 101, 42, 48) /*0.5*/,
            QRect(5, currentScreenHeight - 45, 32, 32) // playButton 播放按钮
        };

    currentScreenHeight += 20;

    QRect full_rect[] = // 全屏时的尺寸
        {
            QRect(0, 0, currentScreenWidth, currentScreenHeight), // backgroundLb
            QRect(0, 0, currentScreenWidth, currentScreenHeight), // media
            QRect(0, currentScreenHeight - 60, currentScreenWidth, 60), // downbkgndLb 全屏的时候，下方的背景板
            QRect(), // 列表框（全屏的时候不显示列表）listWidget
            QRect(0, currentScreenHeight - 68, currentScreenWidth, 22), // 播放进度条 5 horizontalSlider
            QRect(110, currentScreenHeight - 45, 32, 32), // preButton
            QRect(160, currentScreenHeight - 45, 32, 32), // nextButton
            QRect(currentScreenWidth - 100, currentScreenHeight - 45, 32, 32), // volumeButton 4
            QRect(215, currentScreenHeight - 45, 90, 25), // timeLb
            QRect(currentScreenWidth - 98, currentScreenHeight - 205, 22, 160), // volumeSlider 音量滑动条
            QRect(), // label
            QRect(), // logoLabel 4
            QRect(), // userInfo
            QRect(), // loginstatus
            QRect(), // settingBtn
            QRect(), // miniButton 4
            QRect(), // fangdaButton
            QRect(), // closeButton
            QRect(), // 列表显示按钮
            QRect(), // playInfo
            QRect(), // playInfoIco //4
            QRect(30, 30, 0, 0), // 当前播放的内容 curplay
            QRect(60, currentScreenHeight - 45, 32, 32), // 停止按钮
            QRect(300, currentScreenHeight - 45, 120, 25),
            QRect(currentScreenWidth - 50, currentScreenHeight - 45, 32, 32), // fullscreenBtn
            QRect(currentScreenWidth - 150, currentScreenHeight - 45, 32, 32),
            QRect(currentScreenWidth - 200, currentScreenHeight - 45, 32, 32),
            QRect(currentScreenWidth - 260, currentScreenHeight - 45, 32, 32),
            QRect(0, 0, 1, 1),
            QRect(currentScreenWidth - 200, currentScreenHeight - 157, 42, 48) /*1.5*/,
            QRect(currentScreenWidth - 200, currentScreenHeight - 185, 42, 48) /*2*/,
            QRect(currentScreenWidth - 200, currentScreenHeight - 129, 42, 48) /*1*/,
            QRect(currentScreenWidth - 200, currentScreenHeight - 101, 42, 48) /*0*/,
            QRect(5, currentScreenHeight - 45, 32, 32), // playButton
        };

    bool max_hide[] = // 最大化时的隐藏状态和初始状态
        {
            false,
            false,
            true,
            false,
            false,
            false,
            false,
            false,
            false,
            true,
            false,
            false,
            true,
            true,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            true,
            true,
            true,
            true,
            true,
            false,
        };
    bool full_hide[] = // 全屏时的隐藏状态和初始状态
        {
            false, false, false, true,
            false, false, false, false,
            false, true, true, true,
            true, true, true, true,
            true, true, true, true,
            true, true, false, false,
            false, false, false, false,
            true, true, true, true, true, false
        };
    int auto_hide_status[] = // 全屏时是否自动隐藏0 不隐藏 1 隐藏 2 不参与
        {
            0, 0, 1, 2,
            1, 1, 1, 1,
            1, 1, 2, 2,
            2, 2, 2, 2,
            2, 2, 2, 2,
            2, 2, 1, 1,
            1, 1, 1, 1,
            1, 2, 2, 2, 2, 1
        };
    QObjectList list = children();
    QObjectList::iterator it = list.begin();

    for (int i = 0; it != list.end(); ++it, ++i) {

        QWidget* widget = (QWidget*)(*it);
        QString name = widget->objectName();
        helper.init_size_info(widget);
        helper.set_full_rect(name, full_rect[i]);
        helper.set_max_rect(name, max_rect[i]);
        helper.set_full_hide(name, full_hide[i]);
        helper.set_max_hide(name, max_hide[i]);
        helper.set_auto_hide(name, auto_hide_status[i]);
    }
}

void Widget::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == timesID) {
        timesCount++;
        if (timesCount > 5) {
            timesCount = 0;
            ui->time0_5->setVisible(false);
            ui->time1->setVisible(false);
            ui->time1_5->setVisible(false);
            ui->time2->setVisible(false);
            killTimer(timesID);
            timesID = -1;
        }
    } else if (event->timerId() == volumeSliderTimerID) {

        volumeCount++;
        if (volumeCount > 5) {
            volumeCount = 0;
            ui->volumeSlider->setVisible(false);
            killTimer(volumeSliderTimerID);
            volumeSliderTimerID = -1;
        }
    }
}

void Widget::setTime(int hour, int minute, int seconds)
{
    QString s;
    QTextStream out(&s);
    out.setFieldWidth(2);
    out.setPadChar('0');
    out << hour;
    out.setFieldWidth(1);
    out << ":";
    out.setFieldWidth(2);
    out << minute;
    out.setFieldWidth(1);
    out << ":";
    out.setFieldWidth(2);
    out << seconds;
    ui->timeLb->setText(s);
}

void Widget::setTime(int64_t tm)
{
    tm /= 1000; // 得到秒数
    int seconds = tm % 60;
    int minute = (tm / 60) % 60;
    int hour = tm / 3600;
    setTime(hour, minute, seconds);
}

void Widget::setTime2(int hour, int minute, int seconds)
{
    QString s;
    QTextStream out(&s);
    out << "/";
    out.setFieldWidth(2);
    out.setPadChar('0');
    out << hour;
    out.setFieldWidth(1);
    out << ":";
    out.setFieldWidth(2);
    out << minute;
    out.setFieldWidth(1);
    out << ":";
    out.setFieldWidth(2);
    out << seconds;
    ui->time2Lb->setText(s);
}

void Widget::setTime2(int64_t tm)
{
    tm /= 1000; // 得到秒数
    int seconds = tm % 60;
    int minute = (tm / 60) % 60;
    int hour = tm / 3600;
    setTime2(hour, minute, seconds);
}

void Widget::on_show(const QString& nick, const QByteArray& head)
{
    this->setWindowFlag(Qt::FramelessWindowHint);
    info.setWindowFlag(Qt::FramelessWindowHint);
    info.setWindowModality(Qt::ApplicationModal);
    show();
    info.show();
    ui->loginstatus->setText("已登录");
    ui->media->set_float_text(nick);
    ui->media->setGeometry(0, 45, 1200, 690);
    this->nick = nick;
}

void Widget::slot_connect_clicked()
{
    QString strAddress;
    strAddress = QString("https://www.bing.com");
}

void Widget::on_closeButton_released()
{
    emit close();
}

void Widget::on_preButton_clicked()
{
    if (mediaList.size() <= 0) {
        return;
    }
    current_play--;
    if (current_play < 0) {
        current_play = mediaList.size() - 1;
    }
    emit open(mediaList.at(current_play));
    QString filename = mediaList.at(current_play).fileName();
    if (filename.size() > 12) {
        filename.replace(12, filename.size() - 12, "...");
    }
    ui->curPlay->setText(filename);
}

void Widget::on_playButton_clicked()
{
    int count = ui->listWidget->count();
    if (count <= 0) {
        return;
    }
    QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();
    bool isplaying = ui->media->is_playing();
    int index = 0;
    if (selectedItems.size() > 0) {
        index = ui->listWidget->currentRow();
    }
    if (isplaying == false && (ui->media->is_paused() == false)) {
        emit open(mediaList.at(index));

        emit play();

        current_play = index;
        ui->playButton->setStyleSheet(PAUSE_STYLESHEET);

        tick = 0;
        QString filename = mediaList.at(index).fileName();
        if (filename.size() > 12) {
            filename.replace(12, filename.size() - 12, "...");
        }
        ui->curPlay->setText(filename);
        return;
    }

    if (ui->media->is_paused()) {
        emit play();
        return;
    }
    if (index == this->current_play) {
        emit pause();
        ui->playButton->setStyleSheet(PLAY_STYLESHEET);

        return;
    }
    // 切换内容播放
    emit stop();
    emit open(mediaList.at(index));
    emit play();
    current_play = index;
    QString filename = mediaList.at(index).fileName();
    if (filename.size() > 12) {
        filename.replace(12, filename.size() - 12, "...");
    }
    ui->curPlay->setText(filename);
}

void Widget::on_nextButton_clicked()
{
    if (mediaList.size() <= 0) {
        return;
    }
    current_play++;
    if (current_play >= mediaList.size()) {
        current_play = 0;
    }
    emit open(mediaList.at(current_play));
    emit play();
    QString filename = mediaList.at(current_play).fileName();
    if (filename.size() > 12) {
        filename.replace(12, filename.size() - 12, "...");
    }
    ui->curPlay->setText(filename);
}

void Widget::on_volumeButton_clicked()
{
    emit silence();
}

void Widget::on_scaleButton_clicked()
{
    if (timesID == -1) {
        timesID = startTimer(200);
        ui->time0_5->setVisible(true);
        ui->time1->setVisible(true);
        ui->time1_5->setVisible(true);
        ui->time2->setVisible(true);
        timesCount = -20;
        return;
    } else {
        killTimer(timesID);
        timesID = -1;
        ui->time0_5->setVisible(false);
        ui->time1->setVisible(false);
        ui->time1_5->setVisible(false);
        ui->time2->setVisible(false);
    }
}

void Widget::on_horizontalSlider_sliderPressed()
{
    slider_pressed = true;
}

void Widget::on_horizontalSlider_sliderReleased()
{
    slider_pressed = false;
}

void Widget::on_horizontalSlider_valueChanged(int value)
{
    if (slider_pressed) {
        if (ui->media->has_media_player()) {
            int max = ui->horizontalSlider->maximum();
            int min = ui->horizontalSlider->minimum();
            double cur = (value - min) * 1.0 / (max - min);
            emit update_pos(cur);
            setTime(ui->media->get_duration() * cur);
        }
    }
}

void Widget::on_media_position(double pos)
{
    if (slider_pressed == false) {
        int max = ui->horizontalSlider->maximum();
        int min = ui->horizontalSlider->minimum();
        int value = min + pos * (max - min);
        ui->horizontalSlider->setValue(value);
        setTime(ui->media->get_duration() * pos);
        setTime2(ui->media->get_duration() * pos);
    }
}

void Widget::on_media_media_status(QMediaPlayer::PlayerStatus s)
{
    switch (s) {
    case QMediaPlayer::MP_OPEN:
        ui->playButton->setStyleSheet(PLAY_STYLESHEET);
        break;
    case QMediaPlayer::MP_PLAY:
        ui->playButton->setStyleSheet(PAUSE_STYLESHEET);
        break;
    case QMediaPlayer::MP_PAUSE:
        ui->playButton->setStyleSheet(PLAY_STYLESHEET);
        break;
    case QMediaPlayer::MP_CLOSE:
        ui->playButton->setStyleSheet(PLAY_STYLESHEET);
        break;
    default:
        ui->playButton->setStyleSheet(PLAY_STYLESHEET);
        break;
    }
}

void Widget::slots_login_request_finished(QNetworkReply* reply)
{
    // TODO：探活
}

void Widget::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void Widget::dropEvent(QDropEvent* event) // TODO: ???
{
    auto files = event->mimeData()->urls();
    for (int i = 0; i < files.size(); ++i) {
        QUrl url = files.at(i);
        ui->listWidget->addItem(url.fileName());
        mediaList.append(url);
    }
}

void Widget::mouseMoveEvent(QMouseEvent* event)
{
    if (helper.cur_status() == 0) {
        // move(event->globalPos() - position);
        helper.modify_mouse_cousor(event->globalPos());
    }
}

void Widget::mousePressEvent(QMouseEvent* event)
{
    if (helper.type() == 0 && (helper.cur_status() == 0)) {
        position = event->globalPos() - this->pos();
    } else if (helper.type() && (helper.cur_status() == 0)) {
        helper.press(this->pos());
    }
}

void Widget::mouseReleaseEvent(QMouseEvent* event)
{
    helper.release();
    if (isFullScreen()) {
        if ((helper.cur_status() == 2) && (fullScreenTimerID == -1)) {
            fullScreenTimerID = startTimer(1500);
            helper.auto_hide(false);
            // TODO: 头部绘制
        }
    }
}

bool Widget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->horizontalSlider) {
        if (QEvent::MouseButtonPress == event->type()) {
            slider_pressed = true;
        } else if (QEvent::MouseButtonRelease == event->type()) {
            slider_pressed = false;
        }
    }
    if (watched == this) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (isMaximized()) {
                on_fangdaButton_clicked();
            } else if (isFullScreen() == false) {
                on_fangdaButton_clicked();
            }
        }
    }
    if ((event->type() == QEvent::UpdateRequest) || (event->type() == QEvent::Paint) || (event->type() == QEvent::Timer)) {
        helper.modify_mouse_cousor(QCursor::pos());
    }
    if (watched == ui->volumeButton) {
        if (event->type() == QEvent::HoverEnter) {
            ui->volumeSlider->setVisible(true);
            if (volumeSliderTimerID == -1) {
                volumeSliderTimerID = startTimer(200);
            }
        }
    }
    if (watched == ui->volumeSlider) {
        if ((event->type() == QEvent::HoverMove) || (event->type() == QEvent::MouseMove) || (event->type() == QEvent::Wheel)) {
            volumeCount = 0;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Widget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen()) {
            on_fullscreenBtn_clicked();
        }
    }
}

void Widget::on_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
    if (ui->listWidget->currentRow() < mediaList.size()) {
        int index = ui->listWidget->currentRow();
        QUrl url = mediaList.at(index);
        emit stop();
        emit open(url);
        emit play();

        current_play = index;
        QString filename = url.fileName();
        if (filename.size() > 12) {
            filename.replace(12, filename.size() - 12, "...");
        }
        ui->curPlay->setText(filename);
    }
}

void Widget::on_slowButton_clicked()
{
    rate -= 0.25f;
    emit set_scale(rate);
}

void Widget::on_fastButton_clicked()
{
    rate += 0.25f;
    emit set_scale(rate);
}

void Widget::on_volumeSlider_sliderReleased()
{
    emit volume(ui->volumeSlider->value());
}

void Widget::on_fangdaButton_clicked()
{
    if (isMaximized() == false) {
        showMaximized();
        helper.max_size();
        ui->showhideList->setStyleSheet(LIST_SHOW);
        ui->listWidget->setHidden(false);
    } else {
        showNormal();
        helper.org_size();
        ui->listWidget->setHidden(false);
        ui->showhideList->setStyleSheet(LIST_SHOW);
        ui->userInfo->setHidden(false);
    }
}

void Widget::on_showhideList_pressed()
{
    if (ui->listWidget->isHidden() == false) {
        ui->listWidget->hide();
        ui->curPlay->hide(); // 隐藏列表上面的信息栏
        ui->playInfoIco->hide(); // 隐藏列表上面的信息栏图标
        ui->playInfo->hide(); // 隐藏列表上面的信息栏背景
        ui->showhideList->setStyleSheet(LIST_HIDE);
        QPoint pt = ui->showhideList->pos();
        int w = ui->listWidget->width();
        ui->showhideList->move(pt.x() + w, pt.y());
        QRect rect = ui->media->frameGeometry();
        ui->media->move(rect.x() + 150, rect.y());
        rect = ui->horizontalSlider->frameGeometry();
        ui->horizontalSlider->setGeometry(rect.left(), rect.top(), rect.width() + 300, rect.height());
    } else {
        ui->listWidget->show(); // 显示列表
        ui->curPlay->show(); // 显示列表上面的信息栏
        ui->playInfoIco->show(); // 显示列表上面的信息栏图标
        ui->playInfo->show(); // 显示列表上面的信息栏背景
        ui->showhideList->setStyleSheet(LIST_SHOW);
        QPoint pt = ui->showhideList->pos();
        int w = ui->listWidget->width();
        ui->showhideList->move(pt.x() - w, pt.y());
        QRect rect = ui->media->frameGeometry();
        ui->media->move(rect.x() - 150, rect.y());
        rect = ui->horizontalSlider->frameGeometry();
        ui->horizontalSlider->setGeometry(rect.left(), rect.top(), rect.width() - 300, rect.height());
    }
}

void Widget::on_fullscreenBtn_clicked()
{
    if (isFullScreen() == false) {
        QString style = "QPushButton{border-image: url(:/UI/images/tuichuquanping.png);}\n";
        style += "QPushButton:hover{border-image:url(:/UI/images/tuichuquanping-hover.png);}\n";
        style += "QPushButton:pressed{border-image: url(:/UI/images/tuichuquanping.png);}";
        ui->fullscreenBtn->setStyleSheet(style);
        showFullScreen();
        helper.full_size();
        ui->showhideList->setStyleSheet(LIST_SHOW);
        QString filename = current_play >= 0 ? mediaList.at(current_play).fileName() : "";
        if (filename.size() > 12) {
            filename.replace(12, filename.size() - 12, "...");
        }
    } else {
        QString style = "QPushButton{border-image: url(:/UI/images/quanping.png);}\n";
        style += "QPushButton:hover{border-image:url(:/UI/images/quanping-hover.png);}\n";
        style += "QPushButton:pressed{border-image: url(:/UI/images/quanping.png);}";
        ui->fullscreenBtn->setStyleSheet(style);
        showMaximized();
        helper.max_size();
    }
}

void Widget::on_stopButton_clicked()
{
    emit stop();
    setTime(0, 0, 0);
    ui->horizontalSlider->setValue(0);
}

void Widget::on_time2_clicked()
{
    ui->media->set_scale(2.0);
    if (timesID >= 0) {
        killTimer(timesID);
        timesID = -1;
        // 播放倍数
        ui->time0_5->setVisible(false);
        ui->time1->setVisible(false);
        ui->time1_5->setVisible(false);
        ui->time2->setVisible(false);
    }
}

void Widget::on_time1_5_clicked()
{
    ui->media->set_scale(1.5);
    if (timesID >= 0) {
        killTimer(timesID);
        timesID = -1;
        // 播放倍数
        ui->time0_5->setVisible(false);
        ui->time1->setVisible(false);
        ui->time1_5->setVisible(false);
        ui->time2->setVisible(false);
    }
}

void Widget::on_time1_clicked()
{
    ui->media->set_scale(1.0);
    if (timesID >= 0) {
        killTimer(timesID);
        timesID = -1;
        // 播放倍数
        ui->time0_5->setVisible(false);
        ui->time1->setVisible(false);
        ui->time1_5->setVisible(false);
        ui->time2->setVisible(false);
    }
}

void Widget::on_time0_5_clicked()
{
    ui->media->set_scale(0.5);
    if (timesID >= 0) {
        killTimer(timesID);
        timesID = -1;
        // 播放倍数
        ui->time0_5->setVisible(false);
        ui->time1->setVisible(false);
        ui->time1_5->setVisible(false);
        ui->time2->setVisible(false);
    }
}

void Widget::on_miniButton_clicked()
{
    this->showMinimized();
}

void Widget::on_volumeSlider_valueChanged(int value)
{
    emit volume(value % 101);
}

Widget::SizeHelper::SizeHelper(Widget* ui)
{
    curent_coursor = 0;
    isabled = true;
    Qt::CursorShape cursor_type[9] = {
        Qt::ArrowCursor,
        Qt::SizeFDiagCursor,
        Qt::SizeVerCursor,
        Qt::SizeBDiagCursor,
        Qt::SizeHorCursor,
        Qt::SizeFDiagCursor,
        Qt::SizeVerCursor,
        Qt::SizeBDiagCursor,
        Qt::SizeHorCursor
    };
    for (int i = 0; i < 9; ++i) {
        cursors[i] = new QCursor(cursor_type[i]);
    }
    index = 0;
    pressed = false;
    this->ui = ui;
    status = 0;
}

Widget::SizeHelper::~SizeHelper()
{
    for (int i = 0; i < 9; ++i) {
        delete cursors[i];
        cursors[i] = NULL;
    }
}

int Widget::SizeHelper::cur_status() const
{
    return status;
}

void Widget::SizeHelper::modify_mouse_cousor(const QPoint& point)
{
    if (pressed == false) {
        QPoint pt = point - ui->pos();
        for (int i = 0; i < 8; ++i) {
            if (size_rect[i].contains(pt)) {
                if (i + 1 != index) {
                    ui->setCursor(*cursors[i + 1]);
                    index = i + 1;
                }
                return;
            }
        }
        if (index != 0) {
            ui->setCursor(*cursors[0]);
            index = 0;
        }
    }
}

void Widget::SizeHelper::update(int nWidth, int nHeight)
{
    int width = nWidth * 0.01;
    int height = nHeight * 0.01;
    int x_[] = {
        // 左上，顶，右上，右，右下，底，左下，左
        0 /*左上*/,
        width * 3 /*顶上*/,
        nWidth - 5 /*右上*/,
        nWidth - width /*右*/,
        nWidth - width /*右下*/,
        width * 3 /*底*/,
        0 /*左下*/,
        0 /*左*/
    };
    int y_[] = {
        0, 0, 0, // 左上，顶，右上
        height * 3, nHeight - 10, nHeight - 10, // 右，右下，底
        nHeight - height, height * 3 // 左下，左
    };
    int w[] = {
        width * 2, width * 14, width * 2,
        width * 2, width * 2, width * 14,
        width * 2, width * 2
    };
    int h[] = {
        height, height, height,
        height * 14, height, height,
        height, height * 14
    };
    for (int i = 0; i < 8; ++i) {
        size_rect[i].setX(x_[i]);
        size_rect[i].setY(y_[i]);
        size_rect[i].setWidth(w[i]);
        size_rect[i].setHeight(h[i]);
    }
}

void Widget::SizeHelper::set_enable(bool isable)
{
    this->isabled = isable;
}

void Widget::SizeHelper::press(const QPoint& point)
{
    if (pressed == false) {
        pressed = true;
        this->point = point;
    }
}

void Widget::SizeHelper::release()
{
    if (pressed) {
        pressed = false;
        point.setX(-1);
        point.setY(-1);
    }
}

void Widget::SizeHelper::init_size_info(QWidget* widget)
{
    SizeInfo info;
    info.widget = widget;
    info.org_rect = widget->frameGeometry();
    info.last_rect = widget->frameGeometry();
    sub_item_size.insert(widget->objectName(), info);
}

void Widget::SizeHelper::set_full_rect(const QString& name, const QRect& rect)
{
    auto it = sub_item_size.find(name);
    if (it != sub_item_size.end()) {
        sub_item_size[name].full_rect = rect;
    }
}

void Widget::SizeHelper::set_max_rect(const QString& name, const QRect& rect)
{
    auto it = sub_item_size.find(name);
    if (it != sub_item_size.end()) {
        sub_item_size[name].max_rect = rect;
    }
}

void Widget::SizeHelper::set_full_hide(const QString& name, bool is_hide)
{
    auto it = sub_item_size.find(name);
    if (it != sub_item_size.end()) {
        sub_item_size[name].is_full_hide = is_hide;
    }
}

void Widget::SizeHelper::set_max_hide(const QString& name, bool is_hide)
{
    auto it = sub_item_size.find(name);
    if (it != sub_item_size.end()) {
        sub_item_size[name].is_max_hide = is_hide;
    }
}

void Widget::SizeHelper::set_auto_hide(const QString& name, int hide_status)
{
    auto it = sub_item_size.find(name);
    if (it != sub_item_size.end()) {
        sub_item_size[name].auto_hide_status = hide_status;
    }
}

void Widget::SizeHelper::auto_hide(bool hidden)
{
    if (status == 2) {
        for (auto it = sub_item_size.begin(); it != sub_item_size.end(); ++it) {
            if ((*it).auto_hide_status == 1) {
                (*it).widget->setHidden(hidden);
            }
        }
    }
}

void Widget::SizeHelper::full_size()
{
    status = 2;
    for (auto it = sub_item_size.begin(); it != sub_item_size.end(); it++) {

        if ((*it).full_rect.width() > 0) {
            (*it).widget->setGeometry((*it).full_rect);
        }
        (*it).widget->setHidden((*it).is_full_hide);
    }
    QRect rec = QGuiApplication::primaryScreen()->geometry();

    qDebug() << "###size set###" << sub_item_size["media"].full_rect << ": " << sub_item_size["showhideList"].full_rect << ": " << rec.width() << rec.height();
}

void Widget::SizeHelper::org_size()
{
    status = 0;
    for (auto it = sub_item_size.begin(); it != sub_item_size.end(); it++) {
        (*it).widget->setGeometry((*it).org_rect);
        if ((*it).widget->objectName() == "screentop") {
            (*it).widget->setHidden(true);
        }
        if ((*it).widget->objectName() == "volumeSlider") {
            (*it).widget->setHidden(true);
        }
        if ((*it).widget->objectName() == "fangdaButton") {
            (*it).widget->setStyleSheet(SCREEN_MAX_STYLE);
        }
    }
}

void Widget::SizeHelper::max_size()
{
    status = 1;
    for (auto it = sub_item_size.begin(); it != sub_item_size.end(); it++) {
        if ((*it).max_rect.width() > 0) {
            (*it).widget->setGeometry((*it).max_rect);
        }
        (*it).widget->setHidden((*it).is_max_hide);
        if ((*it).widget->objectName() == "screentop") {
            (*it).widget->setHidden(true);
        } else if ((*it).widget->objectName() == "fangdaButton") {
            (*it).widget->setStyleSheet(SCREEN_RESTORE_STYLE);
        } else if ((*it).widget->objectName() == "volumeSlider") {
            (*it).widget->setHidden(true);
        }
    }
}
