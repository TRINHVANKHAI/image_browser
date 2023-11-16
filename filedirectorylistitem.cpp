#include "filedirectorylistitem.h"
#include "ui_filedirectorylistitem.h"
#include <QDebug>
#include <QDateTime>

FileDirectoryListItem::FileDirectoryListItem(QString filename, ssize_t size, uint64_t mtime, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileDirectoryListItem)
{
    ui->setupUi(this);

    itemFileName = filename;
    itemFileSize = size;
    itemFileMtime = mtime;
    ui->labelFileDirName->setText(filename);
    ui->labelFileDirSize->setText(QString::number(size/1024)+QString(" KB"));
    ui->labelFileDirTimeStamp->setText(QDateTime::fromSecsSinceEpoch(mtime).toString("yyyy/MM/dd hh:mm"));
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

