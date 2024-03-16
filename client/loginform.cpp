#include "loginform.h"
#include "ui_loginform.h"

#include <QCryptographicHash>
#include <QDesktopServices>
#include <QJsonParseError>
#include <QMouseEvent>
#include <QtNetwork/QNetworkReply>

bool LOGIN_STATUS = false;

LoginForm::LoginForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LoginForm)
{
    record = new RecordFile("sec.dat");
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint); // 隐藏自带窗口头
    ui->nameEdit->setPlaceholderText("用户名/手机号/邮箱");
    ui->nameEdit->setFrame(false);
    ui->nameEdit->installEventFilter(this);
    ui->pwdEdit->setPlaceholderText("请填写密码");
    ui->pwdEdit->setEchoMode(QLineEdit::Password);
    ui->pwdEdit->installEventFilter(this);
    ui->forget->installEventFilter(this);
    // net = new QNetworkAccessManager(this);
    // connect(net, SIGNAL(finished(QNetworkReply*)), this, SLOT(slots_login_request_fineshed(QNetworkReply*)));
    info.setWindowFlag(Qt::FramelessWindowHint);
    info.setWindowModality(Qt::ApplicationModal);
    QSize sz = size();
    info.move((sz.width() - info.width()) / 2, (sz.height() - info.height()) / 2);
    load_config();
}

LoginForm::~LoginForm()
{
    delete ui;
    delete record;
    delete net;
}

bool LoginForm::eventFilter(QObject* watched, QEvent* event)
{
    if (ui->pwdEdit == watched) {
        if (event->type() == QEvent::FocusIn) {
            ui->pwdEdit->setStyleSheet("color: rgb(251, 251, 251);background-color: transparent;");

        } else if (event->type() == QEvent::FocusOut) {
            if (ui->pwdEdit->text().size() == 0) {
                ui->pwdEdit->setStyleSheet("color: rgb(71, 75, 94);background-color: transparent;");
            }
        }
    } else if (ui->nameEdit == watched) {
        if (event->type() == QEvent::FocusIn) {
            ui->nameEdit->setStyleSheet("color: rgb(251, 251, 251);background-color: transparent;");

        } else if (event->type() == QEvent::FocusOut) {
            if (ui->nameEdit->text().size() == 0) {
                ui->nameEdit->setStyleSheet("color: rgb(71, 75, 94);background-color: transparent;");
            }
        }
    }
    if ((ui->forget == watched) && (event->type() == QEvent::MouseButtonPress)) {
        QDesktopServices::openUrl(QUrl("https://www.baidu.com/"));
    }
    return QWidget::eventFilter(watched, event);
}

void LoginForm::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == auto_login_id) {
        killTimer(auto_login_id);
    }
    QJsonObject& root = record->config();
    QString user = root["user"].toString();
    QString pwd = root["password"].toString();
    check_login(user, pwd);
}

void LoginForm::on_logoButton_released()
{
    if (ui->loginButton->text() == "取消自动登录") {
        killTimer(auto_login_id);
        auto_login_id = -1;
        ui->loginButton->setText("登录");
    } else {
        QString user = ui->nameEdit->text();
        if (user.size() == 0 || user == "用户名/手机号/邮箱") {
            info.set_text(
                    "用户名不能为空\n请输入用户名", "确认")
                .show();
            ui->nameEdit->setFocus();
            return;
        }
        QString pwd = ui->pwdEdit->text();
        if (pwd.size() == 0 || pwd == "请填写密码") {
            info.set_text("密码不能为空\n请输入密码", "确认").show();
            ui->pwdEdit->setFocus();
            return;
        }
        check_login(user, pwd);
    }
}

void LoginForm::slots_autoLoginCheck_stateChange(int state)
{
    record->config()["auto"] = state == Qt::Checked;
    if (state == Qt::Checked) {
        record->config()["remember"] = true;
        ui->rememberPwd->setChecked(true);
    } else {
        ui->rememberPwd->setCheckable(true);
    }
}

void LoginForm::load_config()
{
    connect(ui->autoLoginCheck, SIGNAL(stateChanged(int)), this, SLOT(slots_autoLoginCheck_stateChange(int)));
    QJsonObject& root = record->config();
    ui->rememberPwd->setChecked(root["remember"].toBool());
    ui->autoLoginCheck->setChecked(root["auto"].toBool());
    QString user = root["user"].toString();
    QString pwd = root["password"].toString();
    ui->nameEdit->setText(user);
    ui->pwdEdit->setText(pwd);

    qDebug() << "auto: " << root["auto"].toBool();
    qDebug() << "remember:" << root["remember"].toBool();
    if (root["auto"].toBool()) {
        qDebug() << "user=" << user;
        qDebug() << "pwd=" << pwd;
        if (user.size() > 0 && pwd.size() > 0) {
            {
                ui->loginButton->setText("取消自动登录");
                auto_login_id = startTimer(3000); // 三秒钟后触发timerEvent
            }
        }
    }
}
void LoginForm::slots_login_request_fineshed(QNetworkReply* reply)
{
    this->setEnabled(true);
    bool login_success = false;
    // if (reply->error() != QNetworkReply::NoError) {
    //     info.set_text("登录失败\n" + reply->errorString(), "确认").show();
    //     return;
    // }
    QByteArray data = reply->readAll();
    qDebug() << data;
    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(data, &json_error);
    qDebug() << "json error = " << json_error.error;
    if (json_error.error == QJsonParseError::NoError) {
        if (document.isObject()) {
            const QJsonObject obj = document.object();
            if (obj.contains("status") && obj.contains("message")) {
                QJsonValue status = obj.value("status");
                QJsonValue message = obj.value("message");
                if (status.toInt(-1) == 0) {
                    LOGIN_STATUS = status.toInt(-1) == 0;
                    emit login(record->config()["user"].toString(), QByteArray());
                    hide();
                    login_success = true;
                    char tm[64] = "";
                    time_t t;
                    time(&t);
                    strftime(tm, sizeof(tm), "%Y-%m-%d %H:%M:%S", localtime(&t));
                    record->config()["data"] = QString(tm);
                    record->save();
                }
            }
        }
    } else {
        info.set_text("登录失败\n无法解析服务器应答", "确认").show();
    }
    if (!login_success) {
        info.set_text("登录失败\n用户名或密码错误", "确认").show();
    }
    reply->deleteLater();
}

QString getTime()
{
    time_t t = 0;
    time(&t);
    return QString::number(t);
}

bool LoginForm::check_login(const QString& user, const QString& pwd)
{
    // TODO:远程登录
    LOGIN_STATUS = true;
    emit login(record->config()["user"].toString(), QByteArray());
    hide();
    char tm[64] = "";
    time_t t;
    ::time(&t);
    strftime(tm, sizeof(tm), "%Y-%m-%d %H:%M:%S", localtime(&t));
    record->config()["date"] = QString(tm);
    record->save();
    return false;
}

void LoginForm::mouseMoveEvent(QMouseEvent* event)
{
    move(event->globalPos() - position);
}

void LoginForm::mousePressEvent(QMouseEvent* event)
{
    position = event->globalPos() - this->pos();
}

void LoginForm::mouseReleaseEvent(QMouseEvent*) { }
