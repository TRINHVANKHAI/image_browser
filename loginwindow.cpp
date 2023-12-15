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
    ui->labelLoginStatus->setText("Loging in ...  please wait");
    ssh_client_host = ui->lineEditHostName->text();
    ssh_client_user = ui->lineEditUserName->text();
    ssh_client_pass = ui->lineEditPassword->text();
    ssh_client_port = ui->spinBoxPort->value();
    QByteArray hostBA = ssh_client_host.toLocal8Bit();
    QByteArray userBA = ssh_client_user.toLocal8Bit();
    QByteArray passBA = ssh_client_pass.toLocal8Bit();

    ui->pushButtonLogin->setEnabled(false);
    emit loginConnectToServer(hostBA.data(), ssh_client_port, userBA.data(), passBA.data());
    ui->pushButtonLogin->setEnabled(true);
}

void LoginWindow::on_pushButtonCancel_clicked()
{
    this->close();
}

void LoginWindow::on_sftpClientStatus(enum ssh_ClientStatus stat)
{
    if(stat == SSH_CLIENT_STATUS_ERR_KNOWNHOST) {
        ui->labelLoginStatus->setText("SSH server host is not in known host list");
    }
    else if(stat == SSH_CLIENT_STATUS_ERR_CONNECTION) {
        ui->labelLoginStatus->setText("Failed to connect to server");
    }
    else if(stat == SSH_CLIENT_STATUS_ERR_AUTHENTICATION) {
        ui->labelLoginStatus->setText("Failed to login with your ID and password");
    }
}
