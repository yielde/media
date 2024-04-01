#include "messageform.h"
#include "ui_messageform.h"
#include <QPainter>
#include <QScreen>

MessageForm::MessageForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MessageForm)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    top = new QImage(":/UI/images/screentop.png");
    // screen_width = QApplication::primaryScreen()
    QRect rec = QApplication::primaryScreen()->geometry();
    screen_width = rec.width();
}

MessageForm::~MessageForm()
{
    delete ui;
    delete top;
}

void MessageForm::setText(const QString& text)
{
    this->text = text;
}

void MessageForm::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QRect r = frameGeometry();
}
