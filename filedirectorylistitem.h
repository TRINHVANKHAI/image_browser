#ifndef FILEDIRECTORYLISTITEM_H
#define FILEDIRECTORYLISTITEM_H

#include <QWidget>
#include <QFileInfo>

namespace Ui {
class FileDirectoryListItem;
}

class FileDirectoryListItem : public QWidget
{
    Q_OBJECT

public:
    explicit FileDirectoryListItem(QString filename, QString size, QString mtime, QWidget *parent = nullptr);
    explicit FileDirectoryListItem(QWidget *parent);
    ~FileDirectoryListItem();
    QString getFileName();
    QString getFileSize();
    QString getFileMTime();

private:
    QString itemFileName;
    QString itemFileSize;
    QString itemFileMTime;
    Ui::FileDirectoryListItem *ui;
};

#endif // FILEDIRECTORYLISTITEM_H
