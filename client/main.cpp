#include "loginform.h"
#include "widget.h"

#include <QApplication>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    setenv("VLC_PLUGIN_PATH", "/Applications/VLC.app/Contents/MacOS/plugins", 0);
    QApplication a(argc, argv);
    Widget w;
    LoginForm login;
    w.hide();
    login.show();
    login.connect(&login, SIGNAL(login(const QString&, const QByteArray&)),
        &w, SLOT(on_show(const QString&, const QByteArray&)));
    return a.exec();
}
