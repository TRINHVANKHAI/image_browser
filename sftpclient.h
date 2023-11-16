#ifndef SFTPCLIENT_H
#define SFTPCLIENT_H

#include <QObject>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

enum ssh_ClientStatus {
    SSH_CLIENT_STATUS_DISCONNECTED,
    SSH_CLIENT_STATUS_CONNECTED,
    SSH_CLIENT_STATUS_HOST_VERIFIED,
    SSH_CLIENT_STATUS_AUTHENTICATED,
    SSH_CLIENT_STATUS_SFTP_ESTABLISHED
};

class SftpClient : public QObject
{
    Q_OBJECT
public:
    explicit SftpClient(QObject *parent = nullptr);
    ~SftpClient();
    int verifyKnownHost();
    int sftpChangeListDir(const char *dirname);
    void sftpClientSessionInit();
public slots:
    int connectToServer(char *serverName, int port, char *user);
    void disconnectFromServer();
    int loginAuthPassword(char *pass);
    void onFileDownloadRequested(QString filePath, QString localDir);
signals:
    void sftpClientStatus(enum ssh_ClientStatus stat);
    void sftpClientListFileResponse(QString filename, ssize_t size, uint64_t mtime);
    void requestedFileDownloadStatus(QString filePath, int status);
private:
    int fileDownLoadSync(QString filePath, QString localDir);
    QString sftpCurDir;
    enum ssh_ClientStatus sshClientStatus;
    ssh_session ssh_client_session = nullptr;
    sftp_session sftp_client_session = nullptr;
};

#endif // SFTPCLIENT_H
