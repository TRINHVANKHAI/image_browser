#include "filedirectorylistitem.h"
#include "ui_filedirectorylistitem.h"
#include <QDebug>
#include <QDateTime>
#include <QIcon>

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
    if(filename.endsWith(".csv")) {
        QIcon csvIcon(":/images/images/text.png");
        QPixmap csvPixmap = csvIcon.pixmap(QSize(16, 16));
        ui->labelFileDirIcon->setPixmap(csvPixmap);
    }
    if(filename.endsWith(".yuv")) {
        QIcon yuvIcon(":/images/images/image.png");
        QPixmap yuvPixmap = yuvIcon.pixmap(QSize(16, 16));
        ui->labelFileDirIcon->setPixmap(yuvPixmap);
    }
}

FileDirectoryListItem::FileDirectoryListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileDirectoryListItem)
{
    ui->setupUi(this);
    ui->labelFileDirIcon->setText("");
    ui->labelFileDirName->setText("Name");
    ui->labelFileDirSize->setText("Size");
    ui->labelFileDirTimeStamp->setText("Changed");
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
