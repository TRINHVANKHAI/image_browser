#ifndef IMAGEBROWSER_H
#define IMAGEBROWSER_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "sftpclient.h"
#include "loginwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ImageBrowser; }
QT_END_NAMESPACE

class ImageBrowser : public QMainWindow
{
    Q_OBJECT

public:
    ImageBrowser(QWidget *parent = nullptr);
    ~ImageBrowser();
    int listChangeRemoteDir(const char *dirname);
    SftpClient *pSftp;
    LoginWindow *pLoginWindow;
signals:
    void sftpRequestDownloadFile(QString filename, QString localDir);
public slots:
    void on_sftpStatusChanged(enum ssh_ClientStatus stat);
    void on_sftpClientListFileResponse(QString filename, ssize_t size, uint64_t mtime);
    void on_sftpRequestDownloadFileStatus(QString filename, int status);
private slots:
    void onFileItemClicked(QListWidgetItem* item);
    void on_pushButtonDownLoad_clicked();

    void on_pushButtonRefresh_clicked();

private:
    Ui::ImageBrowser *ui;
};
#endif // IMAGEBROWSER_H
