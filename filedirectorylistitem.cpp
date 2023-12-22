#include "filedirectorylistitem.h"
#include "ui_filedirectorylistitem.h"
#include <QDebug>
#include <QDateTime>

FileDirectoryListItem::FileDirectoryListItem(QString filename, QString size, QString mtime, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileDirectoryListItem)
{
    ui->setupUi(this);

    itemFileName = filename;
    itemFileSize = size;
    itemFileMTime = mtime;
    ui->labelFileDirName->setText(filename);
    ui->labelFileDirSize->setText(size);
    ui->labelFileDirTimeStamp->setText(mtime);
}

FileDirectoryListItem::FileDirectoryListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileDirectoryListItem)
{
    ui->setupUi(this);
    ui->labelFileDirIcon->setText("");
}

FileDirectoryListItem::~FileDirectoryListItem()
{
    delete ui;
}

QString FileDirectoryListItem::getFileName()
{
    return itemFileName;
}

QString FileDirectoryListItem::getFileSize()
{
    return itemFileSize;
}

QString FileDirectoryListItem::getFileMTime()
{
    return itemFileMTime;
}
