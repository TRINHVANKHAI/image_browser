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
    explicit FileDirectoryListItem(QString filename, ssize_t size, uint64_t mtime, QWidget *parent = nullptr);
    explicit FileDirectoryListItem(QWidget *parent);
    ~FileDirectoryListItem();
    QString itemFileName;
    ssize_t itemFileSize;
    uint64_t itemFileMtime;
private:
    Ui::FileDirectoryListItem *ui;
};

#endif // FILEDIRECTORYLISTITEM_H
