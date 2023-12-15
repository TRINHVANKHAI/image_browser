#ifndef SFTPCLIENT_H
#define SFTPCLIENT_H

#include <QThread>
#include <QTimer>
#include <QDebug>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

enum ssh_ClientStatus {
    SSH_CLIENT_STATUS_DISCONNECTED,
    SSH_CLIENT_STATUS_CONNECTED,
    SSH_CLIENT_STATUS_ERR_CONNECTION,
    SSH_CLIENT_STATUS_HOST_VERIFIED,
    SSH_CLIENT_STATUS_ERR_KNOWNHOST,
    SSH_CLIENT_STATUS_AUTHENTICATED,
    SSH_CLIENT_STATUS_ERR_AUTHENTICATION,
    SSH_CLIENT_STATUS_SFTP_ESTABLISHED
};

class SftpClient : public QThread
{
    Q_OBJECT

public:
    explicit SftpClient(QObject *parent = nullptr);
    ~SftpClient();
    int verifyKnownHost();
    int sftpChangeListDir(const char *dirname);

    int downloadSelectedFile(QString filePath, QString localDir);
    int deleteSelectedFile(QString file);

public slots:
    int connectToSftpServer(char *serverName, int port, char *user, char *pass);
    void disconnectFromSftpServer();
    void onKeepAliveEvent();
signals:
    void sftpClientStatus(enum ssh_ClientStatus stat);
    void sftpClientListFileResponse(QStringList filename);
private:

    void run() override {
        keepAliveEvent = new QTimer(this);
        connect(keepAliveEvent, SIGNAL(timeout()), this, SLOT(onKeepAliveEvent()));
        keepAliveEvent->start(30000);
        exec();
        keepAliveEvent->stop();
        delete keepAliveEvent;
    }
    int sftpClientSessionInit();
    void sftpClientSessionDestroy();
    int fileDownLoadSync(QString filePath, QString localDir);
    QString sftpCurDir;
    QTimer *keepAliveEvent;
    enum ssh_ClientStatus sshClientStatus;
    ssh_session ssh_client_session = nullptr;
    sftp_session sftp_client_session = nullptr;
};

#endif // SFTPCLIENT_H
