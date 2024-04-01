#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>

#include "infoform.h"
#include "recordfile.h"

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget {
    Q_OBJECT

public:
    explicit LoginForm(QWidget* parent = nullptr);
    ~LoginForm();
    virtual bool eventFilter(QObject* watched, QEvent* event);
    virtual void timerEvent(QTimerEvent* event);
protected slots:
    void on_loginButton_released();
    // void on_rememberPwd_stateChanged(int stat);
    void slots_autoLoginCheck_stateChange(int stat);
    void slots_login_request_fineshed(QNetworkReply* reply);
signals:
    void login(const QString& nick, const QByteArray& head);

protected:
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    bool check_login(const QString& user, const QString& pwd);
    void load_config();

private:
    Ui::LoginForm* ui;
    QNetworkAccessManager* net;
    InfoForm info;
    RecordFile* record;
    int auto_login_id;
    QPoint position;
};

#endif // LOGINFORM_H
