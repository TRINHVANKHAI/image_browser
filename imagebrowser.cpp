#include "imagebrowser.h"
#include "ui_imagebrowser.h"
#include "filedirectorylistitem.h"
#include "dialog.h"

#include <QDebug>
#include <QDateTime>

ImageBrowser::ImageBrowser(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageBrowser)
{
    ui->setupUi(this);
    pSftp = new SftpClient(this);
    pSftp->start();
    pLoginWindow = new LoginWindow(this);
    pLoginWindow->setWindowTitle("ログイン");
    pSettingMenu = new SettingMenu(this);
    pSettingMenu->setWindowTitle("ユザー設定");

    connect(pLoginWindow, SIGNAL(loginConnectToServer(char*,int,char*,char*)),
            pSftp, SLOT(connectToSftpServer(char*,int,char*,char*)));
    connect(this, SIGNAL(disconectFromSftpServer()),
            pSftp, SLOT(disconnectFromSftpServer()));
    connect(pSftp, SIGNAL(sftpClientStatus(ssh_ClientStatus)),
            pLoginWindow, SLOT(on_sftpClientStatus(ssh_ClientStatus)));

    connect(pSettingMenu, SIGNAL(localDownloadDirChanged(QString)),
            this, SLOT(on_LocalDownloadDirChanged(QString)));

    connect(pSftp, SIGNAL(sftpClientStatus(ssh_ClientStatus)),
            this, SLOT(on_sftpStatusChanged(ssh_ClientStatus)));
    connect(pSftp, SIGNAL(sftpClientListFileResponse(QStringList)),
            this,  SLOT(on_sftpClientListFileResponse(QStringList)));


    connect(ui->listWidget_FileList, SIGNAL(itemClicked(QListWidgetItem*)),
             this, SLOT(onFileItemClicked(QListWidgetItem*)));

    pSettingMenu->loadSettings();
    pLoginWindow->show();
    pLoginWindow->raise();
    pLoginWindow->activateWindow();
    qDebug() << "ImageBrowser init";
}

ImageBrowser::~ImageBrowser()
{
    delete ui;
    delete pLoginWindow;
    pSftp->quit();
    pSftp->wait();
    delete pSftp;
    qDebug() << "~ImageBrowser()";
}

int ImageBrowser::listChangeRemoteDir(const char *dirname)
{
    pSftp->sftpChangeListDir(dirname);
    return 0;
}

void ImageBrowser::on_sftpStatusChanged(enum ssh_ClientStatus stat)
{
    if(stat == SSH_CLIENT_STATUS_AUTHENTICATED) {
        pLoginWindow->close();
        ui->statusbar->showMessage("認証しました");
    }
    if(stat == SSH_CLIENT_STATUS_SFTP_ESTABLISHED) {
        ui->statusbar->showMessage("接続済み");
        pSftp->sftpChangeListDir(REMOTE_IMAGE_DIR);
    }
}

void ImageBrowser::on_sftpClientListFileResponse(QStringList filenames)
{
    QString filename, filesize, filemtime;
    QString filenamepro;
    QStringList filenamesplit;
    int idx;
    ui->listWidget_FileList->clear();
    QListWidgetItem *headItem = new QListWidgetItem(ui->listWidget_FileList);
    FileDirectoryListItem *headFileDirItem = new FileDirectoryListItem(this);
    headItem->setSizeHint(headFileDirItem->size());
    ui->listWidget_FileList->addItem(headItem);
    ui->listWidget_FileList->setItemWidget(headItem, headFileDirItem);
    sftpFileList.clear();
    for(idx=0; idx<filenames.length(); idx++) {
        filenamepro = filenames.at(idx);
        filenamesplit = filenamepro.split(",");
        filename = filenamesplit.at(0);
        filesize = filenamesplit.at(1);
        filemtime = filenamesplit.at(2);
        sftpFileList.push_back(filename);
        QListWidgetItem *newItem = new QListWidgetItem(ui->listWidget_FileList);
        FileDirectoryListItem *newFileDirItem = new FileDirectoryListItem(filename, filesize, filemtime, this);
        newItem->setSizeHint(newFileDirItem->size());
        ui->listWidget_FileList->addItem(newItem);
        ui->listWidget_FileList->setItemWidget(newItem, newFileDirItem);
    }

}

void ImageBrowser::onFileItemClicked(QListWidgetItem* item)
{
    FileDirectoryListItem *widgetItem = static_cast<FileDirectoryListItem *>(ui->listWidget_FileList->itemWidget(item));
    ui->lineEditFilePath->setText(widgetItem->itemFileName);

}

void ImageBrowser::on_pushButtonDownLoad_clicked()
{
    ui->pushButtonDownLoad->setEnabled(false);
    QString filePathToDownload = ui->lineEditFilePath->text();
    QString msg;
    ui->statusbar->showMessage(msg);

    if(pSftp->downloadSelectedFile(filePathToDownload, localDownloadDir) == 0) {
        msg = filePathToDownload + QString::fromUtf8(" ファイルを保存しました");
        qDebug() << msg;
        ui->statusbar->showMessage(msg);
    } else {
        msg = filePathToDownload + QString::fromUtf8(" ファイルを保存できませんでした");
        qDebug() << msg;
        ui->statusbar->showMessage(msg);
    }
    ui->pushButtonDownLoad->setEnabled(true);
}

void ImageBrowser::on_pushButtonRefresh_clicked()
{
    this->listChangeRemoteDir(REMOTE_IMAGE_DIR);
}

void ImageBrowser::on_LocalDownloadDirChanged(QString dir)
{
    this->localDownloadDir = dir;
    qDebug() << "ImageBrowser::on_LocalDownloadDirChanged = " << this->localDownloadDir;
}

void ImageBrowser::on_actionLogin_triggered()
{

}

void ImageBrowser::on_actionClose_triggered()
{
    this->close();
}

void ImageBrowser::on_actionSettings_triggered()
{

    this->pSettingMenu->show();
    this->pSettingMenu->raise();
    this->pSettingMenu->isActiveWindow();
}


void ImageBrowser::on_actionDelete_triggered()
{
    QString fileToDelete = ui->lineEditFilePath->text();
    QString mesg = fileToDelete + QString("\n") + QString("ファイルを削除しますか");
    Dialog *verifyDialog = new Dialog(this);
    connect(verifyDialog, SIGNAL(accepted()), this, SLOT(on_actionDeleteAccepted()));
    verifyDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    verifyDialog->setWindowTitle("選択したファイルを削除します");
    verifyDialog->showMessage(mesg);

    verifyDialog->show();
    verifyDialog->raise();
    verifyDialog->activateWindow();
}

void ImageBrowser::on_actionDeleteAccepted()
{
    QString fileToDelete = ui->lineEditFilePath->text();
    QString msg;

    if(pSftp->deleteSelectedFile(fileToDelete)==0) {
        sftpFileList.pop_front();
        msg = fileToDelete + QString::fromUtf8(" ファイルを削除しました");
        qDebug() << msg;
        ui->statusbar->showMessage(msg);
        ui->lineEditFilePath->setText("");
    } else {
        msg = fileToDelete + QString::fromUtf8(" ファイルを削除出来ませんでした");
        qDebug() << msg;
        ui->statusbar->showMessage(msg);
    }
    this->listChangeRemoteDir(REMOTE_IMAGE_DIR);
}

void ImageBrowser::on_actionDelete_All_triggered()
{
    QString mesg = QString("全てのファイルを削除しますか");
    Dialog *verifyDialog = new Dialog(this);
    connect(verifyDialog, SIGNAL(accepted()), this, SLOT(on_actionDeleteAllAccepted()));
    verifyDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    verifyDialog->setWindowTitle("全てのファイルを削除します");
    verifyDialog->showMessage(mesg);

    verifyDialog->show();
    verifyDialog->raise();
    verifyDialog->activateWindow();
}

void ImageBrowser::on_actionDeleteAllAccepted()
{
    QString fileToDelete;
    QString msg;

    while(!sftpFileList.isEmpty()) {
        fileToDelete = sftpFileList.first();
        if(pSftp->deleteSelectedFile(fileToDelete)==0) {
            sftpFileList.pop_front();
            msg = fileToDelete + QString::fromUtf8(" ファイルを削除しています");
            qDebug() << msg;
            ui->statusbar->showMessage(msg);
            ui->lineEditFilePath->setText("");
        } else {
            msg = fileToDelete + QString::fromUtf8(" ファイルを削除出来ませんでした");
            qDebug() << msg;
            ui->statusbar->showMessage(msg);
        }
    }
    this->listChangeRemoteDir(REMOTE_IMAGE_DIR);
    ui->statusbar->showMessage("全てのファイルを削除されました");
}
