#include "imagebrowser.h"
#include "ui_imagebrowser.h"
#include "filedirectorylistitem.h"
#include <QDebug>
#include <QDateTime>

ImageBrowser::ImageBrowser(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageBrowser)
{
    ui->setupUi(this);
    pSftp = new SftpClient();
    pLoginWindow = new LoginWindow(this);
    pLoginWindow->setWindowTitle("ログイン");
    connect(pLoginWindow, SIGNAL(loginConnectToServer(char*,int,char*)), pSftp, SLOT(connectToServer(char*,int,char*)));
    connect(pLoginWindow, SIGNAL(loginAuthUserPassword(char*)), pSftp, SLOT(loginAuthPassword(char*)));

    connect(this, SIGNAL(sftpRequestDownloadFile(QString,QString)),
            pSftp, SLOT(onFileDownloadRequested(QString,QString)));
    connect(pSftp, SIGNAL(requestedFileDownloadStatus(QString,int)),
            this, SLOT(on_sftpRequestDownloadFileStatus(QString,int)));
    connect(pSftp, SIGNAL(sftpClientStatus(ssh_ClientStatus)),
            this, SLOT(on_sftpStatusChanged(ssh_ClientStatus)));
    connect(pSftp, SIGNAL(sftpClientListFileResponse(QString,ssize_t,uint64_t)),
            this,  SLOT(on_sftpClientListFileResponse(QString,ssize_t,uint64_t)));


    connect(ui->listWidget_FileList, SIGNAL(itemClicked(QListWidgetItem*)),
             this, SLOT(onFileItemClicked(QListWidgetItem*)));

    QListWidgetItem *headItem = new QListWidgetItem(ui->listWidget_FileList);
    FileDirectoryListItem *headFileDirItem = new FileDirectoryListItem(this);
    headItem->setSizeHint(headFileDirItem->size());
    ui->listWidget_FileList->addItem(headItem);
    ui->listWidget_FileList->setItemWidget(headItem, headFileDirItem);



    pLoginWindow->show();
    pLoginWindow->raise();
    pLoginWindow->activateWindow();
    qDebug() << "ImageBrowser init";
}

ImageBrowser::~ImageBrowser()
{
    delete ui;
    delete pLoginWindow;
    delete pSftp;
    qDebug() << "~ImageBrowser()";
}

int ImageBrowser::listRemoteDir(const char *dirname)
{

}

void ImageBrowser::on_sftpStatusChanged(enum ssh_ClientStatus stat)
{
    if(stat == SSH_CLIENT_STATUS_AUTHENTICATED) {
        pLoginWindow->close();
        ui->statusbar->showMessage("Authenticated");
        pSftp->sftpClientSessionInit();
    }
    if(stat == SSH_CLIENT_STATUS_SFTP_ESTABLISHED) {
        ui->statusbar->showMessage("Sftp established");
        pSftp->sftpChangeListDir("/tmp/Image");
    }
}

void ImageBrowser::on_sftpClientListFileResponse(QString filename, ssize_t size, uint64_t mtime)
{

    QListWidgetItem *newItem = new QListWidgetItem(ui->listWidget_FileList);
    FileDirectoryListItem *newFileDirItem = new FileDirectoryListItem(filename, size, mtime, this);
    newItem->setSizeHint(newFileDirItem->size());
    ui->listWidget_FileList->addItem(newItem);
    ui->listWidget_FileList->setItemWidget(newItem, newFileDirItem);
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
    QString localDir = "C:/Users/khai/Downloads";
    emit this->sftpRequestDownloadFile(filePathToDownload, localDir);
}

void ImageBrowser::on_sftpRequestDownloadFileStatus(QString fileName, int status)
{

    QString msg;
    if(status) {
        msg = "Downloaded file: " + fileName;
    } else {
        msg = "Failed to download file: " + fileName;
    }
    ui->statusbar->showMessage(msg);
    ui->pushButtonDownLoad->setEnabled(true);
}

void ImageBrowser::on_pushButtonRefresh_clicked()
{
    ui->listWidget_FileList->clear();
    QListWidgetItem *headItem = new QListWidgetItem(ui->listWidget_FileList);
    FileDirectoryListItem *headFileDirItem = new FileDirectoryListItem(this);
    headItem->setSizeHint(headFileDirItem->size());
    ui->listWidget_FileList->addItem(headItem);
    ui->listWidget_FileList->setItemWidget(headItem, headFileDirItem);
    pSftp->sftpChangeListDir("/tmp/Image");
}
