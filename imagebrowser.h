#ifndef IMAGEBROWSER_H
#define IMAGEBROWSER_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "sftpclient.h"
#include "loginwindow.h"
#include "settingmenu.h"

#define REMOTE_IMAGE_DIR "/opt/exposure_control/Images"
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
    SettingMenu *pSettingMenu;
signals:
    void disconectFromSftpServer();

public slots:
    void on_sftpStatusChanged(enum ssh_ClientStatus stat);
    void on_sftpClientListFileResponse(QStringList filename);
    void on_LocalDownloadDirChanged(QString dir);
    void on_actionDeleteAccepted();
    void on_actionDeleteAllAccepted();

private slots:
    void onFileItemClicked(QListWidgetItem* item);
    void on_pushButtonDownLoad_clicked();

    void on_pushButtonRefresh_clicked();

    void on_actionClose_triggered();

    void on_actionSettings_triggered();

    void on_actionLogin_triggered();

    void on_actionDelete_triggered();

    void on_actionDelete_All_triggered();

private:
    QString localDownloadDir;
    QStringList sftpFileList;
    Ui::ImageBrowser *ui;
};
#endif // IMAGEBROWSER_H
