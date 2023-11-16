#include "loginwindow.h"
#include "ui_loginwindow.h"

LoginWindow::LoginWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_spinBoxPort_valueChanged(int port)
{
    ssh_client_port = port;
}

void LoginWindow::on_pushButtonLogin_clicked()
{
    ssh_client_host = ui->lineEditHostName->text();
    ssh_client_user = ui->lineEditUserName->text();
    ssh_client_pass = ui->lineEditPassword->text();
    QByteArray hostBA = ssh_client_host.toLocal8Bit();
    QByteArray userBA = ssh_client_user.toLocal8Bit();
    QByteArray passBA = ssh_client_pass.toLocal8Bit();
    emit loginConnectToServer(hostBA.data(), ssh_client_port, userBA.data());
    emit loginAuthUserPassword(passBA.data());
}

void LoginWindow::on_pushButtonCancel_clicked()
{
    this->close();
}
