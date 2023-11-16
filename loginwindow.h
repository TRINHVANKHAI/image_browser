#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void on_pushButtonLogin_clicked();
    void on_pushButtonCancel_clicked();
    void on_spinBoxPort_valueChanged(int arg1);

signals:
    int loginConnectToServer(char *server, int port, char *user);
    int loginAuthUserPassword(char *pass);

private:
    int ssh_client_port;
    QString ssh_client_host;
    QString ssh_client_user;
    QString ssh_client_pass;
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
