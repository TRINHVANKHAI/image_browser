#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::showMessage(QString text) {
    ui->labelMessage->setText(text);
}

void Dialog::on_pushButtonOK_clicked()
{
    this->accept();
}


void Dialog::on_pushButtonCancel_clicked()
{
    this->reject();
}

