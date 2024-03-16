#ifndef INFOFORM_H
#define INFOFORM_H

#include <QWidget>

namespace Ui {
class InfoForm;
}

class InfoForm : public QWidget {
    Q_OBJECT

public:
    explicit InfoForm(QWidget* parent = nullptr);
    ~InfoForm();

public:
    InfoForm& set_text(const QString& text, const QString& button);

protected:
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
signals:
    void button_clicked();
    void closed();
protected slots:
    void on_connectButton_released();
    void on_closeButton_released();

private:
    Ui::InfoForm* ui;
    QPoint position;
};

#endif // INFOFORM_H
