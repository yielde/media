#include "widget.h"
#include "loginform.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    widget w;
    LoginForm login;
    w.hide();
    login.show();
    return a.exec();
}
