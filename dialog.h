#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void showMessage(QString txt);

private slots:
    void on_pushButtonOK_clicked();

    void on_pushButtonCancel_clicked();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
