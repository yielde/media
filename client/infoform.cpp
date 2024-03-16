#include "infoform.h"
#include "ui_infoform.h"

#include <QMouseEvent>

InfoForm::InfoForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::InfoForm)
{
    ui->setupUi(this);
}

InfoForm::~InfoForm()
{
    delete ui;
}

InfoForm& InfoForm::set_text(const QString& text, const QString& button)
{
    ui->connectButton->setText(button);
    ui->textLB->setText(text);
    return *this;
}

void InfoForm::on_connectButton_released()
{
    hide();
    emit button_clicked();
}

void InfoForm::on_closeButton_released()
{
    emit closed();
    hide();
}

void InfoForm::mouseMoveEvent(QMouseEvent* event)
{
    move(event->globalPos() - position);
}

void InfoForm::mousePressEvent(QMouseEvent* event)
{
    position = event->globalPos() - this->pos();
}

void InfoForm::mouseReleaseEvent(QMouseEvent*) { }
